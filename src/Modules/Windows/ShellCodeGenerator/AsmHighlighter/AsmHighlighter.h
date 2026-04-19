#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

class AsmHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit AsmHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat    format;
    };

    QVector<Rule> m_rules;

    void addRule(const QString& pattern, const QTextCharFormat& fmt);

    QTextCharFormat m_mnemonicFmt;    // mov, push, call …
    QTextCharFormat m_registerFmt;    // rax, rbx, eax …
    QTextCharFormat m_numberFmt;      // 0x…, decimal, binary
    QTextCharFormat m_commentFmt;     // ; comment
    QTextCharFormat m_stringFmt;      // "string" / 'string'
    QTextCharFormat m_directiveFmt;   // BITS, section, db, dw …
    QTextCharFormat m_labelFmt;       // label:
    QTextCharFormat m_sizePtrFmt;     // BYTE PTR, QWORD PTR …
    QTextCharFormat m_bracketFmt;     // [ ]
};