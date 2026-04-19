#pragma once

#include "core/modules/WindowBase.h"
#include "ShellcodeEngine/ShellcodeEngine.h"

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

class AsmHighlighter;

class ShellcodeGeneratorDialog : public WindowBase {
    Q_OBJECT

public:
    explicit ShellcodeGeneratorDialog(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onAssemble();
    void onCopyOutput();
    void onClear();

private:
    void buildUi();
    void connectSignals();

    void applyHighlighterStyles();
    void setStatus(const QString& msg, bool error = false);

    [[nodiscard]] QString currentBits() const;
    [[nodiscard]] ShellcodeEngine::OutputStyle currentStyle() const;

    QTextEdit* m_asmInput = nullptr;
    QTextEdit* m_shellcodeOutput = nullptr;
    QComboBox* m_shellcodeStyle = nullptr;
    QComboBox* m_archCombo = nullptr;
    QPushButton* m_copyBtn = nullptr;
    QPushButton* m_clearBtn = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_byteCountLabel = nullptr;

    AsmHighlighter* m_highlighter = nullptr;
};