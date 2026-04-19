#include "ShellcodeEngine.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextStream>

QString ShellcodeEngine::findTool(const QString& name) {
    const QString fromPath = QStandardPaths::findExecutable(name);
    if (!fromPath.isEmpty())
        return fromPath;

    const QStringList candidates = {
        // win
        QStringLiteral("C:/Program Files/NASM/%1.exe").arg(name),
        QStringLiteral("C:/Program Files (x86)/NASM/%1.exe").arg(name),
        QStringLiteral("C:/nasm/%1.exe").arg(name),
        // macOS (homebrew)
        QStringLiteral("/opt/homebrew/bin/%1").arg(name),
        QStringLiteral("/usr/local/bin/%1").arg(name),
        // linux
        QStringLiteral("/usr/bin/%1").arg(name),
    };

    for (const QString& candidate : candidates) {
        if (QFile::exists(candidate))
            return candidate;
    }

    return name;
}

QList<ShellcodeEngine::Dependency> ShellcodeEngine::checkDependencies() {
    const QStringList tools = { QStringLiteral("nasm"), QStringLiteral("ndisasm") };
    QList<Dependency> result;

    for (const QString& tool : tools) {
        const QString path = findTool(tool);
        const bool found = !QStandardPaths::findExecutable(QFileInfo(path).fileName()).isEmpty() || QFile::exists(path);
        result.append({ tool, found });
    }
    return result;
}

ShellcodeEngine::AssembleResult ShellcodeEngine::assemble(const QString& asmSource,
    const QString& bits) {
    const QString tmpAsm = QDir::tempPath() + QStringLiteral("/shellgen_input.asm");
    const QString tmpBin = QDir::tempPath() + QStringLiteral("/shellgen_output.bin");

    {
        QFile f(tmpAsm);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
            return { false, {}, QStringLiteral("Cannot create temp file: %1").arg(tmpAsm) };

        QTextStream out(&f);
        out << "BITS " << bits << "\n" << asmSource << "\n";
    }

    QProcess proc;
    proc.start(findTool(QStringLiteral("nasm")),
        { QStringLiteral("-f"), QStringLiteral("bin"),
         QStringLiteral("-o"), tmpBin, tmpAsm }
    );

    if (!proc.waitForStarted(3000)) {
        QFile::remove(tmpAsm);
        return { false, {}, QStringLiteral("nasm not found. Install it and make sure it is in PATH.") };
    }
    proc.waitForFinished(5000);

    QFile::remove(tmpAsm);

    if (proc.exitCode() != 0) {
        QFile::remove(tmpBin);
        const QString err = QString::fromUtf8(proc.readAllStandardError()).trimmed().replace(tmpAsm, QStringLiteral("<input>"));
        return { false, {}, err };
    }

    QFile binFile(tmpBin);
    if (!binFile.open(QIODevice::ReadOnly)) {
        return { false, {}, QStringLiteral("Cannot read nasm output file.") };
    }
    const QByteArray bytes = binFile.readAll();
    binFile.close();
    QFile::remove(tmpBin);

    if (bytes.isEmpty())
        return { false, {}, QStringLiteral("Assembled 0 bytes — empty output.") };

    return { true, bytes, {} };
}

QList<ShellcodeEngine::DisasmEntry>
ShellcodeEngine::disassemble(const QByteArray& raw, const QString& bits) {
    QList<DisasmEntry> result;

    const QString tmpBin = QDir::tempPath() + QStringLiteral("/shellgen_disasm.bin");
    {
        QFile f(tmpBin);
        if (!f.open(QIODevice::WriteOnly))
            return result;
        f.write(raw);
    }

    QProcess proc;
    proc.start(findTool(QStringLiteral("ndisasm")), { QStringLiteral("-b"), bits, tmpBin });

    const bool started = proc.waitForStarted(3000);
    const bool finished = started && proc.waitForFinished(5000);
    QFile::remove(tmpBin);

    if (!started || !finished)
        return result;

    const QString     output = QString::fromUtf8(proc.readAllStandardOutput());
    const QStringList lines = output.split(u'\n', Qt::SkipEmptyParts);

    static const QRegularExpression kSpaces(QStringLiteral("\\s+"));

    for (const QString& line : lines) {
        const QStringList parts = line.trimmed().split(kSpaces, Qt::SkipEmptyParts);
        if (parts.size() < 3)
            continue;

        bool ok = false;
        const int offset = parts[0].toInt(&ok, 16);
        if (!ok)
            continue;

        const QByteArray rawBytes = QByteArray::fromHex(parts[1].toLatin1());
        if (rawBytes.isEmpty())
            continue;

        const QString mnemonic = parts.mid(2).join(u' ').toLower();
        result.append({ offset, static_cast<int>(rawBytes.size()), mnemonic });
    }
    return result;
}

QString ShellcodeEngine::formatAnnotated(const QByteArray& raw,
    const QList<DisasmEntry>& entries) {
    if (!entries.isEmpty()) {
        // widest byte-string for alignment
        int maxByteLen = 0;
        for (const auto& e : entries) {
            // each byte is 4 chars + 2 chars = 6 chars per byte
            maxByteLen = qMax(maxByteLen, e.size * 6);
        }

        QString result;
        result.reserve(entries.size() * 48);

        for (int i = 0; i < entries.size(); ++i) {
            const auto& e = entries[i];
            const bool   isLast = (i + 1 == entries.size());

            // hex part
            QString bytePart;
            bytePart.reserve(e.size * 6);
            for (int b = 0; b < e.size; ++b) {
                const auto byte = static_cast<uint8_t>(raw[e.offset + b]);
                bytePart += QStringLiteral("0x%1").arg(byte, 2, 16, QChar('0'));
                if (!(isLast && b + 1 == e.size))
                    bytePart += QStringLiteral(", ");
            }

            // comment
            const QString comment =
                (e.offset == 0)
                ? QStringLiteral("// %1").arg(e.mnemonic)
                : QStringLiteral("// %1 (+0x%2)").arg(e.mnemonic).arg(e.offset, 0, 16);

            // align comment column
            const int padding = qMax(2, maxByteLen - bytePart.length() + 2);

            result += QStringLiteral("    ") + bytePart + QString(padding, u' ') + comment + u'\n';
        }
        return result;
    }

    // Fallback: ndisasm unavailable —> dump raw bytes, 12 per line
    QString result;
    constexpr int kCols = 12;
    for (int i = 0; i < raw.size(); ++i) {
        if (i % kCols == 0)
            result += QStringLiteral("    ");

        result += QStringLiteral("0x%1").arg(static_cast<uint8_t>(raw[i]), 2, 16, QChar('0'));

        if (i + 1 < raw.size()) {
            result += QStringLiteral(", ");
            if ((i + 1) % kCols == 0)
                result += u'\n';
        }
    }
    result += u'\n';
    return result;
}

QString ShellcodeEngine::generateC(const QByteArray& raw,
    const QList<DisasmEntry>& entries) {
    return QStringLiteral("unsigned char shellcode[] = {  // %1 bytes\n")
        .arg(raw.size())
        + formatAnnotated(raw, entries)
        + QStringLiteral("};\n");
}

QString ShellcodeEngine::generateCpp(const QByteArray& raw,
    const QList<DisasmEntry>& entries) {
    return QStringLiteral("std::array<std::uint8_t, %1> shellcode = {  // %1 bytes\n")
        .arg(raw.size())
        + formatAnnotated(raw, entries)
        + QStringLiteral("};\n");
}

QString ShellcodeEngine::generateRaw(const QByteArray& raw) {
    QString result;
    result.reserve(raw.size() * 3);

    for (int i = 0; i < raw.size(); ++i) {
        result += QStringLiteral("%1").arg(static_cast<uint8_t>(raw[i]), 2, 16, QChar('0'));

        if ((i + 1) % 16 == 0)
            result += u'\n';
        else if (i + 1 < raw.size())
            result += u' ';
    }
    return result;
}

QString ShellcodeEngine::format(const QByteArray& raw,
    const QList<DisasmEntry>& entries,
    OutputStyle               style,
    const QString& bits) {
    Q_UNUSED(bits);

    switch (style) {
    case OutputStyle::C:   return generateC(raw, entries);
    case OutputStyle::Cpp: return generateCpp(raw, entries);
    case OutputStyle::Raw: return generateRaw(raw);
    }
    return generateC(raw, entries);
}