#include "AsmHighlighter.h"

void AsmHighlighter::addRule(const QString& pattern, const QTextCharFormat& fmt) {
    m_rules.append({ QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption), fmt });
}

AsmHighlighter::AsmHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
    // Pre-populate with style hints and safe fallback colors
    m_formats["comment"].setFontItalic(true);
    m_formats["comment"].setForeground(QColor("#75715e"));
    m_formats["directive"].setFontWeight(QFont::Bold);
    m_formats["directive"].setForeground(QColor("#f92672"));
    m_formats["mnemonic"].setFontWeight(QFont::Bold);
    m_formats["mnemonic"].setForeground(QColor("#66d9ef"));
    m_formats["register"].setForeground(QColor("#fd971f"));
    m_formats["number"].setForeground(QColor("#ae81ff"));
    m_formats["bracket"].setFontWeight(QFont::Bold);
    m_formats["bracket"].setForeground(QColor("#f8f8f2"));
    m_formats["label"].setForeground(QColor("#a6e22e"));
    m_formats["string"].setForeground(QColor("#e6db74"));
    m_formats["sizePtr"].setForeground(QColor("#a6e22e"));

    updateRules();
}

void AsmHighlighter::setColors(const QVariantMap& colors) {
    if (colors.isEmpty()) return; // Don't wipe fallback colors with empty ones

    for (auto it = colors.begin(); it != colors.end(); ++it) {
        if (it.value().canConvert<QColor>()) {
            m_formats[it.key()].setForeground(it.value().value<QColor>());
        }
    }
    updateRules();
    rehighlight();
}

void AsmHighlighter::updateRules() {
    m_rules.clear();

    const QString mnemonics =
        QStringLiteral(
            "\\b(?:"
            "MOV|MOVZX|MOVSX|MOVSXD|XCHG|LEA|PUSH|POP|PUSHA|POPA|PUSHAD|POPAD|"
            "PUSHF|POPF|PUSHFD|POPFD|PUSHFQ|POPFQ|"
            "ADD|ADC|SUB|SBB|MUL|IMUL|DIV|IDIV|INC|DEC|NEG|"
            "XADD|CMPXCHG|CMPXCHG8B|CMPXCHG16B|"
            "AND|OR|XOR|NOT|TEST|CMP|"
            "SHL|SHR|SAL|SAR|ROL|ROR|RCL|RCR|SHLD|SHRD|"
            "JMP|JE|JNE|JZ|JNZ|JG|JGE|JL|JLE|JA|JAE|JB|JBE|JS|JNS|"
            "JO|JNO|JP|JNP|JPE|JPO|JCXZ|JECXZ|JRCXZ|"
            "LOOP|LOOPE|LOOPNE|LOOPZ|LOOPNZ|"
            "CALL|RET|RETN|RETF|IRET|IRETD|IRETQ|"
            "MOVS|MOVSB|MOVSW|MOVSD|MOVSQ|"
            "CMPS|CMPSB|CMPSW|CMPSD|CMPSQ|"
            "SCAS|SCASB|SCASW|SCASD|SCASQ|"
            "LODS|LODSB|LODSW|LODSD|LODSQ|"
            "STOS|STOSB|STOSW|STOSD|STOSQ|"
            "REP|REPE|REPNE|REPZ|REPNZ|"
            "BT|BTS|BTR|BTC|BSF|BSR|POPCNT|LZCNT|TZCNT|"
            "INT|INTO|SYSCALL|SYSENTER|SYSEXIT|SYSRET|"
            "NOP|HLT|CPUID|RDTSC|RDTSCP|PAUSE|UD2|"
            "LGDT|SGDT|LIDT|SIDT|LLDT|SLDT|LTR|STR|"
            "MOV|LMSW|SMSW|CLTS|INVD|WBINVD|INVLPG|"
            "IN|OUT|INS|OUTS|INSB|INSW|INSD|OUTSB|OUTSW|OUTSD|"
            "CLC|STC|CMC|CLD|STD|CLI|STI|CLAC|STAC|"
            "LAHF|SAHF|CBW|CWDE|CDQE|CWD|CDQ|CQO|"
            "SETE|SETNE|SETG|SETGE|SETL|SETLE|SETA|SETAE|SETB|SETBE|"
            "SETS|SETNS|SETO|SETNO|SETP|SETNP|"
            "CMOVE|CMOVNE|CMOVG|CMOVGE|CMOVL|CMOVLE|"
            "CMOVA|CMOVAE|CMOVB|CMOVBE|CMOVS|CMOVNS|"
            "MOVAPS|MOVUPS|MOVSS|MOVSD|ADDPS|ADDSS|SUBPS|SUBSS|"
            "MULPS|MULSS|DIVPS|DIVSS|XORPS|ANDPS|ORPS|"
            "MOVDQU|MOVDQA|PXOR|PADDQ|PSUBQ|PCMPEQB|PMOVMSKB|"
            "VMOVAPS|VMOVUPS|VXORPS|VANDPS|VORPS"
            ")\\b");
    addRule(mnemonics, m_formats["mnemonic"]);

    const QString registers =
        QStringLiteral(
            "\\b(?:"
            "RAX|RBX|RCX|RDX|RSI|RDI|RBP|RSP|"
            "R8|R9|R10|R11|R12|R13|R14|R15|"
            "EAX|EBX|ECX|EDX|ESI|EDI|EBP|ESP|"
            "R8D|R9D|R10D|R11D|R12D|R13D|R14D|R15D|"
            "AX|BX|CX|DX|SI|DI|BP|SP|"
            "R8W|R9W|R10W|R11W|R12W|R13W|R14W|R15W|"
            "AL|AH|BL|BH|CL|CH|DL|DH|SPL|BPL|SIL|DIL|"
            "R8B|R9B|R10B|R11B|R12B|R13B|R14B|R15B|"
            "CS|DS|ES|FS|GS|SS|"
            "CR0|CR2|CR3|CR4|CR8|DR0|DR1|DR2|DR3|DR6|DR7|"
            "RIP|EIP|IP|RFLAGS|EFLAGS|FLAGS|"
            "ST0|ST1|ST2|ST3|ST4|ST5|ST6|ST7|"
            "MM0|MM1|MM2|MM3|MM4|MM5|MM6|MM7|"
            "XMM0|XMM1|XMM2|XMM3|XMM4|XMM5|XMM6|XMM7|"
            "XMM8|XMM9|XMM10|XMM11|XMM12|XMM13|XMM14|XMM15|"
            "YMM0|YMM1|YMM2|YMM3|YMM4|YMM5|YMM6|YMM7|"
            "YMM8|YMM9|YMM10|YMM11|YMM12|YMM13|YMM14|YMM15|"
            "ZMM0|ZMM1|ZMM2|ZMM3|ZMM4|ZMM5|ZMM6|ZMM7"
            ")\\b");
    addRule(registers, m_formats["register"]);

    const QString directives =
        QStringLiteral(
            "\\b(?:BITS|SECTION|SEGMENT|GLOBAL|EXTERN|ORG|USE16|USE32|USE64|"
            "DB|DW|DD|DQ|DT|DO|DY|DZ|RESB|RESW|RESD|RESQ|REST|RESO|"
            "EQU|ALIGN|ALIGNB|STRUC|ENDSTRUC|ISTRUC|IEND|"
            "TIMES|INCBIN|%DEFINE|%MACRO|%ENDMACRO|%IF|%ENDIF|%INCLUDE|"
            "PROC|ENDP|END)\\b");
    addRule(directives, m_formats["directive"]);

    const QString sizeSpecs = QStringLiteral("\\b(?:BYTE|WORD|DWORD|QWORD|TWORD|OWORD|YWORD|ZWORD|PTR|SHORT|NEAR|FAR)\\b");
    addRule(sizeSpecs, m_formats["sizePtr"]);

    addRule(QStringLiteral("\\b(?:0[xX][0-9A-Fa-f]+|[0-9A-Fa-f]+[hH]|0[bB][01]+|\\d+)\\b"), m_formats["number"]);
    addRule(QStringLiteral("[\\[\\]]"), m_formats["bracket"]);
    addRule(QStringLiteral("^\\s*[A-Za-z_\\.][A-Za-z0-9_\\.]*\\s*:"), m_formats["label"]);

    addRule(QStringLiteral("\"[^\"]*\"|'[^']*'"), m_formats["string"]);
    addRule(QStringLiteral(";[^\n]*"), m_formats["comment"]);
}

void AsmHighlighter::highlightBlock(const QString& text) {
    for (const Rule& rule : std::as_const(m_rules)) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            const QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
