#include "shellcodegeneratordialog.h"
#include "AsmHighlighter/AsmHighlighter.h"

#include "core/modules/ModuleManager.h"

#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QVBoxLayout>

static QString displayName() {
    return QCoreApplication::translate("ShellCodeGenerator", "Shell code");
}

static bool registered = []() {
    ModuleManager::instance().registerModule<WindowBase>(
        &displayName, "", []() { return new ShellcodeGeneratorDialog(); });
    return true;
}();

namespace {

    struct ArchEntry {
        const char* label;
        const char* bits;
    };

    constexpr ArchEntry kArchEntries[] = {
        {"x86 (16-bit)", "16"},
        {"x86 (32-bit)", "32"},
        {"x86 (64-bit)", "64"},
    };

    constexpr int kArchCount = static_cast<int>(std::size(kArchEntries));

    struct StyleEntry {
        const char* label;
        ShellcodeEngine::OutputStyle style;
    };

    constexpr StyleEntry kStyles[] = {
        {"C",   ShellcodeEngine::OutputStyle::C},
        {"C++", ShellcodeEngine::OutputStyle::Cpp},
        {"RAW", ShellcodeEngine::OutputStyle::Raw},
    };

    constexpr int kStyleCount = static_cast<int>(std::size(kStyles));

    constexpr auto kEditorStyle =
        "background-color: #1e1e1e;"
        "color: #d4d4d4;"
        "border: 1px solid #3c3c3c;"
        "selection-background-color: #264f78;";

}

ShellcodeGeneratorDialog::ShellcodeGeneratorDialog(QWidget* parent) : WindowBase(parent) {
    setWindowTitle(tr("Shellcode Generator"));
    setModal(false);
    setMinimumSize(1400, 760);

    buildUi();
    connectSignals();

    QTimer::singleShot(0, this, [this]() {
        const auto deps = ShellcodeEngine::checkDependencies();
        QStringList missing;
        for (const auto& d : deps) {
            if (!d.found)
                missing << d.name;
        }

        if (!missing.isEmpty()) {
            QMessageBox::warning(
                this,
                tr("Missing dependencies"),
                tr("The following tools were not found on your system:\n\n"
                    " - %1\n\n"
                    "Please install them or add their location to PATH.\n\n"
                    "Download: https://www.nasm.us/pub/nasm/releasebuilds/").arg(missing.join(QStringLiteral("\n - "))));
            close();
        }
        });
}

void ShellcodeGeneratorDialog::buildUi() {
    const QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);
    root->setContentsMargins(12, 12, 12, 12);

    // toolbar
    auto* toolbar = new QHBoxLayout();
    toolbar->setSpacing(10);

    toolbar->addWidget(new QLabel(tr("Architecture:"), this));

    m_archCombo = new QComboBox(this);
    m_archCombo->setMinimumWidth(120);
    for (int i = 0; i < kArchCount; ++i)
        m_archCombo->addItem(QString::fromUtf8(kArchEntries[i].label));
    m_archCombo->setCurrentIndex(2); // default: x86 64-bit
    toolbar->addWidget(m_archCombo);

    toolbar->addSpacing(10);
    toolbar->addWidget(new QLabel(tr("Output style:"), this));

    m_shellcodeStyle = new QComboBox(this);
    m_shellcodeStyle->setMinimumWidth(80);
    m_shellcodeStyle->setMinimumContentsLength(10);
    for (int i = 0; i < kStyleCount; ++i)
        m_shellcodeStyle->addItem(QString::fromUtf8(kStyles[i].label), static_cast<int>(kStyles[i].style));
    toolbar->addWidget(m_shellcodeStyle);

    toolbar->addStretch(1);

    m_byteCountLabel = new QLabel(tr("0 bytes"), this);
    m_byteCountLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));
    toolbar->addWidget(m_byteCountLabel);

    m_copyBtn = new QPushButton(tr("Copy"), this);
    m_copyBtn->setCursor(Qt::PointingHandCursor);
    toolbar->addWidget(m_copyBtn);

    m_clearBtn = new QPushButton(tr("Clear"), this);
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    toolbar->addWidget(m_clearBtn);

    root->addLayout(toolbar);

    // editors
    auto* editorsLayout = new QHBoxLayout();
    editorsLayout->setSpacing(8);

    m_asmInput = new QTextEdit(this);
    m_asmInput->setPlaceholderText(tr("; Enter x86/x64 assembly here…"));
    m_asmInput->setFont(monoFont);
    m_asmInput->setTabStopDistance(32);
    m_asmInput->setStyleSheet(QString::fromUtf8(kEditorStyle));
    m_asmInput->setLineWrapMode(QTextEdit::NoWrap);

    m_highlighter = new AsmHighlighter(m_asmInput->document());

    m_shellcodeOutput = new QTextEdit(this);
    m_shellcodeOutput->setPlaceholderText(tr("// Shellcode output will appear here…"));
    m_shellcodeOutput->setFont(monoFont);
    m_shellcodeOutput->setReadOnly(true);
    m_shellcodeOutput->setStyleSheet(QString::fromUtf8(kEditorStyle));
    m_shellcodeOutput->setLineWrapMode(QTextEdit::NoWrap);

    editorsLayout->addWidget(m_asmInput, 1);
    editorsLayout->addWidget(m_shellcodeOutput, 1);

    root->addLayout(editorsLayout, 1);

    // status bar
    m_statusLabel = new QLabel(tr("Ready. Make sure nasm and ndisasm are in PATH."), this);
    m_statusLabel->setStyleSheet(QStringLiteral("color: gray; font-size: 11px;"));
    root->addWidget(m_statusLabel);
}

void ShellcodeGeneratorDialog::connectSignals() {
    // Debounce: only assemble 250 ms after the user stops typing
    auto* debounce = new QTimer(this);
    debounce->setSingleShot(true);
    debounce->setInterval(250);

    connect(m_asmInput, &QTextEdit::textChanged, debounce,
        qOverload<>(&QTimer::start));
    connect(debounce, &QTimer::timeout,
        this, &ShellcodeGeneratorDialog::onAssemble);

    auto triggerReassemble = [this](int) {
        if (!m_shellcodeOutput->toPlainText().isEmpty())
            onAssemble();
        };
    connect(m_shellcodeStyle, qOverload<int>(&QComboBox::currentIndexChanged), this, triggerReassemble);
    connect(m_archCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, triggerReassemble);

    connect(m_copyBtn, &QPushButton::clicked, this, &ShellcodeGeneratorDialog::onCopyOutput);
    connect(m_clearBtn, &QPushButton::clicked, this, &ShellcodeGeneratorDialog::onClear);
}

QString ShellcodeGeneratorDialog::currentBits() const {
    const int idx = m_archCombo->currentIndex();
    if (idx < 0 || idx >= kArchCount)
        return QStringLiteral("64");
    return QString::fromUtf8(kArchEntries[idx].bits);
}

ShellcodeEngine::OutputStyle ShellcodeGeneratorDialog::currentStyle() const {
    return static_cast<ShellcodeEngine::OutputStyle>(
        m_shellcodeStyle->currentData().toInt());
}

void ShellcodeGeneratorDialog::setStatus(const QString& msg, bool error) {
    m_statusLabel->setText(msg);
    m_statusLabel->setStyleSheet(
        error ? QStringLiteral("color: #dc3545; font-size: 11px;")
        : QStringLiteral("color: gray; font-size: 11px;"));

    if (error)
        QApplication::beep();
}

void ShellcodeGeneratorDialog::onAssemble() {
    const QString asmText = m_asmInput->toPlainText().trimmed();

    if (asmText.isEmpty()) {
        m_shellcodeOutput->clear();
        m_byteCountLabel->setText(tr("0 bytes"));
        setStatus(tr("Ready."));
        return;
    }

    setStatus(tr("Assembling…"));

    // assemble
    const QString bits = currentBits();
    const auto    result = ShellcodeEngine::assemble(asmText, bits);

    if (!result.ok) {
        setStatus(result.error, true);
        return;
    }

    m_byteCountLabel->setText(tr("%1 bytes").arg(result.bytes.size()));

    // disassemble
    const auto entries = ShellcodeEngine::disassemble(result.bytes, bits);

    // format output
    const QString output = ShellcodeEngine::format(result.bytes, entries, currentStyle(), bits);
    m_shellcodeOutput->setPlainText(output);
    setStatus(tr("OK — %1 bytes assembled.").arg(result.bytes.size()));
}

void ShellcodeGeneratorDialog::onCopyOutput() {
    const QString text = m_shellcodeOutput->toPlainText();
    if (!text.isEmpty()) {
        QGuiApplication::clipboard()->setText(text);
        setStatus(tr("Copied to clipboard."));
    }
}

void ShellcodeGeneratorDialog::onClear() {
    m_asmInput->clear();
    m_shellcodeOutput->clear();
    m_byteCountLabel->setText(tr("0 bytes"));
    setStatus(tr("Ready."));
}