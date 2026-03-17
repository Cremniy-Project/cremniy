#include "disassemblerworker.h"

#include <QProcess>
#include <QRegularExpression>

DisassemblerWorker::DisassemblerWorker(QObject *parent)
    : QObject{parent}
{}

void DisassemblerWorker::cancel()
{
    m_cancelled = true;
}

QString DisassemblerWorker::detectArch(const QString &filePath)
{
    QProcess fileProc;
    fileProc.start("file", {"-b", filePath});
    if (!fileProc.waitForFinished(3000))
        return {};

    QString desc = QString::fromUtf8(fileProc.readAllStandardOutput()).toLower();

    if (desc.contains("x86-64") || desc.contains("x86_64") || desc.contains("amd64"))
        return "i386:x86-64";
    if (desc.contains("80386") || desc.contains("ia-32"))
        return "i386";
    if (desc.contains("aarch64") || desc.contains("arm64"))
        return "aarch64";
    if (desc.contains("arm"))
        return "arm";
    if (desc.contains("mips") && desc.contains("64"))
        return "mips:isa64r2";
    if (desc.contains("mips"))
        return "mips:3000";
    if (desc.contains("riscv") && desc.contains("64"))
        return "riscv:rv64";
    if (desc.contains("riscv"))
        return "riscv:rv32";

    return {};
}

void DisassemblerWorker::disassemble(const QString &filePath, const QString &arch)
{
    m_cancelled = false;

    // ── arch detection ────────────────────────────────────────────────────
    QString effectiveArch = arch.isEmpty() ? detectArch(filePath) : arch;

    emit logLine("[disasm] file      : " + filePath);
    emit logLine("[disasm] arch-hint : " + (effectiveArch.isEmpty() ? "(auto)" : effectiveArch));

    // ── build objdump command ─────────────────────────────────────────────
    QStringList args;
    args << "-d";
    if (!effectiveArch.isEmpty()) {
        args << "-m" << effectiveArch;
        if (effectiveArch.contains("i386") || effectiveArch.contains("x86"))
            args << "-M" << "intel";
    }
    args << filePath;

    emit logLine("[disasm] command   : objdump " + args.join(' '));

    // ── launch process ────────────────────────────────────────────────────
    // Force C locale: ensures English section headers ("Disassembly of section")
    // and standard tab-separated format regardless of system locale.
    QProcess proc;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG",   "C");
    env.insert("LC_ALL", "C");
    proc.setProcessEnvironment(env);
    proc.setProcessChannelMode(QProcess::SeparateChannels);
    proc.start("objdump", args);

    if (!proc.waitForStarted(5000)) {
        emit logLine("[disasm] ERROR: failed to start objdump");
        emit errorOccurred(tr("Failed to start objdump. Make sure it is installed and in PATH."));
        emit finished();
        return;
    }

    emit logLine("[disasm] objdump started (pid " + QString::number(proc.processId()) + ")");

    QByteArray output;
    while (!proc.waitForFinished(200)) {
        if (m_cancelled) {
            proc.kill();
            emit logLine("[disasm] cancelled by user");
            emit finished();
            return;
        }
        output += proc.readAllStandardOutput();
    }
    output += proc.readAllStandardOutput();

    QByteArray stderrData = proc.readAllStandardError();
    int exitCode = proc.exitCode();

    emit logLine(QString("[disasm] exit code : %1").arg(exitCode));
    emit logLine(QString("[disasm] stdout    : %1 bytes").arg(output.size()));

    if (!stderrData.isEmpty())
        emit logLine("[disasm] stderr    : " + QString::fromUtf8(stderrData).trimmed());

    if (exitCode != 0) {
        QString errMsg = QString::fromUtf8(stderrData).trimmed();
        if (errMsg.isEmpty())
            errMsg = tr("objdump exited with code %1").arg(exitCode);
        emit errorOccurred(errMsg);
        emit finished();
        return;
    }

    if (m_cancelled) { emit finished(); return; }

    // ── dump first 30 lines of raw output to log ──────────────────────────
    emit logLine("[disasm] --- raw output (first 30 lines) ---");
    const QStringList rawLines = QString::fromUtf8(output).split('\n');
    int preview = qMin(30, rawLines.size());
    for (int i = 0; i < preview; ++i)
        emit logLine(rawLines[i]);
    if (rawLines.size() > 30)
        emit logLine(QString("[disasm] ... (%1 more lines)").arg(rawLines.size() - 30));
    emit logLine("[disasm] --- end raw output ---");

    // ── parse ─────────────────────────────────────────────────────────────
    QVector<DisasmSection> sections = parseSections(output);

    emit logLine(QString("[disasm] sections parsed: %1").arg(sections.size()));
    for (const auto &s : sections)
        emit logLine(QString("[disasm]   section '%1': %2 instructions")
                         .arg(s.name).arg(s.instructions.size()));

    int total = sections.size();
    for (int i = 0; i < total; ++i) {
        if (m_cancelled) break;
        emit sectionFound(sections[i]);
        emit progressUpdated((i + 1) * 100 / total);
    }

    emit finished();
}

// ---------------------------------------------------------------------------
// Parser
// ---------------------------------------------------------------------------
// Verified objdump -d format (cat -A shows ^I for TAB):
//
// Disassembly of section .text:
//
// 0000000000001040 <_start>:
//     1040:^If3 0f 1e fa          ^Iendbr64
//     1004:^I48 83 ec 08          ^Isub    $0x8,%rsp
//     1008:^I48 8b 05 d9 2f 00 00 ^Imov    0x2fd9(%rip),%rax   # 3fe8 <sym>

QVector<DisasmSection> DisassemblerWorker::parseSections(const QByteArray &raw)
{
    QVector<DisasmSection> sections;

    static const QRegularExpression reSectionHdr(
        R"(^Disassembly of section\s+(\S+):$)",
        QRegularExpression::MultilineOption);

    static const QRegularExpression reFuncLabel(
        R"(^([0-9a-fA-F]+)\s+<([^>]+)>:$)",
        QRegularExpression::MultilineOption);

    // bytes field: one or more "XX " groups padded with spaces, then TAB before mnemonic
    static const QRegularExpression reInsn(
        R"(^\s+([0-9a-fA-F]+):\t((?:[0-9a-fA-F]{2} )+)\s+\t(\S+)(?:[ \t]+([^#\n]*))?)",
        QRegularExpression::MultilineOption);

    const QString text = QString::fromUtf8(raw);
    const QStringList lines = text.split('\n');
    DisasmSection *curSection = nullptr;

    for (const QString &line : lines) {
        if (m_cancelled) break;

        QRegularExpressionMatch m = reSectionHdr.match(line);
        if (m.hasMatch()) {
            sections.append(DisasmSection{ m.captured(1), {} });
            curSection = &sections.last();
            continue;
        }

        if (!curSection) continue;

        m = reFuncLabel.match(line);
        if (m.hasMatch()) {
            DisasmInstruction lbl;
            lbl.address  = m.captured(1);
            lbl.mnemonic = "<" + m.captured(2) + ">";
            curSection->instructions.append(lbl);
            continue;
        }

        m = reInsn.match(line);
        if (m.hasMatch()) {
            DisasmInstruction insn;
            insn.address  = m.captured(1);
            insn.bytes    = m.captured(2).trimmed();
            insn.mnemonic = m.captured(3);
            insn.operands = m.captured(4).trimmed();
            curSection->instructions.append(insn);
        }
    }

    return sections;
}
