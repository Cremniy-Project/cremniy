#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>
#include <QColor>
#include <QMap>
#include <QVariantMap>

class AsmHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit AsmHighlighter(QTextDocument* parent = nullptr);

    void setColors(const QVariantMap& colors);
    void updateRules();

protected:
    void highlightBlock(const QString& text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat    format;
    };

    QVector<Rule> m_rules;

    void addRule(const QString& pattern, const QTextCharFormat& fmt);

    // Categories: mnemonic, register, number, comment, string, directive, label, sizePtr, bracket
    QMap<QString, QTextCharFormat> m_formats;
};