#include "disassemblertab.h"
#include "disassemblerworker.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QThread>
#include <QFont>
#include <QColor>
#include <QDateTime>

static constexpr int COL_ADDR     = 0;
static constexpr int COL_BYTES    = 1;
static constexpr int COL_MNEMONIC = 2;
static constexpr int COL_OPERANDS = 3;
static constexpr int NUM_COLS     = 4;

// ─────────────────────────────────────────────────────────────────────────────
DisassemblerTab::DisassemblerTab(QWidget *parent, QString path)
    : QWidget{parent}
    , m_filePath(path)
{
    setupUi();

    m_thread = new QThread(this);
    m_worker = new DisassemblerWorker();
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::finished,       m_worker, &QObject::deleteLater);
    connect(this,  &DisassemblerTab::requestDisassembly,
            m_worker, &DisassemblerWorker::disassemble, Qt::QueuedConnection);
    connect(m_worker, &DisassemblerWorker::sectionFound,
            this,     &DisassemblerTab::onSectionFound,    Qt::QueuedConnection);
    connect(m_worker, &DisassemblerWorker::finished,
            this,     &DisassemblerTab::onWorkerFinished,  Qt::QueuedConnection);
    connect(m_worker, &DisassemblerWorker::errorOccurred,
            this,     &DisassemblerTab::onWorkerError,     Qt::QueuedConnection);
    connect(m_worker, &DisassemblerWorker::progressUpdated,
            this,     &DisassemblerTab::onProgressUpdated, Qt::QueuedConnection);
    connect(m_worker, &DisassemblerWorker::logLine,
            this,     &DisassemblerTab::onLogLine,         Qt::QueuedConnection);

    m_thread->start();
}

DisassemblerTab::~DisassemblerTab()
{
    if (m_worker) m_worker->cancel();
    m_thread->quit();
    m_thread->wait(2000);
}

void DisassemblerTab::setTabData(QByteArray &)
{
    startDisassembly();
}

// ─────────────────────────────────────────────────────────────────────────────
void DisassemblerTab::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Toolbar ──────────────────────────────────────────────────────────────
    m_toolbar = new QWidget(this);
    m_toolbar->setObjectName("dasmToolbar");
    m_toolbar->setFixedHeight(36);
    m_toolbar->setStyleSheet(
        "QWidget#dasmToolbar { background: #262626; border-bottom: 1px solid #1f1f1f; }");
    auto *toolLayout = new QHBoxLayout(m_toolbar);
    toolLayout->setContentsMargins(6, 0, 6, 0);
    toolLayout->setSpacing(6);

    m_runBtn = new QPushButton(tr("Disassemble"), m_toolbar);
    m_runBtn->setFixedHeight(26);
    toolLayout->addWidget(m_runBtn);

    m_cancelBtn = new QPushButton(tr("Cancel"), m_toolbar);
    m_cancelBtn->setFixedHeight(26);
    m_cancelBtn->setVisible(false);
    toolLayout->addWidget(m_cancelBtn);

    toolLayout->addSpacing(8);

    QLabel *secLabel = new QLabel(tr("Section:"), m_toolbar);
    secLabel->setStyleSheet("color: #888888;");
    toolLayout->addWidget(secLabel);

    m_sectionCombo = new QComboBox(m_toolbar);
    m_sectionCombo->setFixedHeight(26);
    m_sectionCombo->setMinimumWidth(140);
    m_sectionCombo->setEnabled(false);
    toolLayout->addWidget(m_sectionCombo);

    toolLayout->addSpacing(8);

    m_searchEdit = new QLineEdit(m_toolbar);
    m_searchEdit->setPlaceholderText(tr("Search address / mnemonic / operands…"));
    m_searchEdit->setFixedHeight(26);
    m_searchEdit->setEnabled(false);
    toolLayout->addWidget(m_searchEdit, 1);

    toolLayout->addSpacing(8);

    m_logToggleBtn = new QPushButton(tr("Log"), m_toolbar);
    m_logToggleBtn->setFixedHeight(26);
    m_logToggleBtn->setFixedWidth(50);
    m_logToggleBtn->setCheckable(true);
    m_logToggleBtn->setChecked(false);
    m_logToggleBtn->setToolTip(tr("Toggle diagnostic log panel"));
    toolLayout->addWidget(m_logToggleBtn);

    mainLayout->addWidget(m_toolbar);

    // ── Progress bar ──────────────────────────────────────────────────────────
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(3);
    m_progressBar->setVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar { background: #1f1f1f; border: none; }"
        "QProgressBar::chunk { background: #2626d5; }");
    mainLayout->addWidget(m_progressBar);

    // ── Splitter: disasm table (top) + log panel (bottom) ────────────────────
    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->setHandleWidth(4);
    m_splitter->setStyleSheet(
        "QSplitter::handle { background: #1f1f1f; }"
        "QSplitter::handle:hover { background: #2626d5; }");
    mainLayout->addWidget(m_splitter, 1);

    // ── Top: stack (table / placeholder) ─────────────────────────────────────
    QWidget *topWidget = new QWidget(m_splitter);
    auto *topLayout = new QVBoxLayout(topWidget);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    m_stack = new QStackedWidget(topWidget);
    topLayout->addWidget(m_stack);

    m_placeholderLbl = new QLabel(topWidget);
    m_placeholderLbl->setAlignment(Qt::AlignCenter);
    m_placeholderLbl->setStyleSheet("color: #555555; font-size: 14px;");
    m_stack->addWidget(m_placeholderLbl);

    m_table = new QTableWidget(topWidget);
    m_table->setColumnCount(NUM_COLS);
    m_table->setHorizontalHeaderLabels({tr("Address"), tr("Bytes"), tr("Mnemonic"), tr("Operands")});
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setShowGrid(false);
    m_table->setAlternatingRowColors(true);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(COL_ADDR,     QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(COL_BYTES,    QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(COL_MNEMONIC, QHeaderView::ResizeToContents);
    m_table->setStyleSheet(
        "QTableWidget { background-color: #1f1f1f; alternate-background-color: #222222;"
        "  gridline-color: #262626; border: none;"
        "  font-family: 'JetBrains Mono', 'Consolas', monospace; font-size: 12px; }"
        "QTableWidget::item { padding: 2px 8px; color: #cccccc; }"
        "QTableWidget::item:selected { background: #2d2d50; color: #ffffff; }"
        "QHeaderView::section { background: #262626; color: #888888; border: none;"
        "  border-bottom: 1px solid #333333; padding: 4px 8px; font-size: 11px; }");
    m_stack->addWidget(m_table);
    m_splitter->addWidget(topWidget);

    // ── Bottom: log panel ─────────────────────────────────────────────────────
    m_logPanel = new QWidget(m_splitter);
    auto *logLayout = new QVBoxLayout(m_logPanel);
    logLayout->setContentsMargins(0, 0, 0, 0);
    logLayout->setSpacing(0);

    // Log header bar
    QWidget *logHeader = new QWidget(m_logPanel);
    logHeader->setFixedHeight(24);
    logHeader->setStyleSheet("background: #1a1a1a; border-top: 1px solid #333333;");
    auto *logHeaderLayout = new QHBoxLayout(logHeader);
    logHeaderLayout->setContentsMargins(8, 0, 8, 0);
    QLabel *logTitle = new QLabel(tr("Diagnostic Log"), logHeader);
    logTitle->setStyleSheet("color: #666666; font-size: 11px;");
    logHeaderLayout->addWidget(logTitle);
    logHeaderLayout->addStretch();
    QPushButton *clearBtn = new QPushButton(tr("Clear"), logHeader);
    clearBtn->setFixedHeight(18);
    clearBtn->setFixedWidth(44);
    clearBtn->setStyleSheet(
        "QPushButton { font-size: 10px; padding: 0; border: 1px solid #333333; color: #888888; }"
        "QPushButton:hover { color: #cccccc; border-color: #555555; }");
    logHeaderLayout->addWidget(clearBtn);
    logLayout->addWidget(logHeader);

    m_logView = new QPlainTextEdit(m_logPanel);
    m_logView->setReadOnly(true);
    m_logView->setMaximumBlockCount(5000);
    m_logView->setStyleSheet(
        "QPlainTextEdit { background: #141414; color: #888888;"
        "  font-family: 'JetBrains Mono', 'Consolas', monospace; font-size: 11px;"
        "  border: none; }");
    logLayout->addWidget(m_logView, 1);
    m_splitter->addWidget(m_logPanel);

    // Hide log panel initially; give table all the space
    m_logPanel->setVisible(false);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 0);

    // ── Status bar ─────────────────────────────────────────────────────────────
    m_statusLabel = new QLabel(this);
    m_statusLabel->setFixedHeight(20);
    m_statusLabel->setStyleSheet(
        "color: #555555; font-size: 11px; padding: 0 6px;"
        "background: #262626; border-top: 1px solid #1f1f1f;");
    mainLayout->addWidget(m_statusLabel);

    showPlaceholder(tr("Press \"Disassemble\" to analyse the file"));

    // ── Connections ───────────────────────────────────────────────────────────
    connect(m_runBtn,       &QPushButton::clicked,
            this,           &DisassemblerTab::startDisassembly);
    connect(m_cancelBtn,    &QPushButton::clicked,
            this,           &DisassemblerTab::cancelDisassembly);
    connect(m_searchEdit,   &QLineEdit::textChanged,
            this,           &DisassemblerTab::onSearchTextChanged);
    connect(m_sectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,           &DisassemblerTab::onSectionComboChanged);
    connect(m_logToggleBtn, &QPushButton::toggled, this, [this](bool checked) {
        m_logPanel->setVisible(checked);
        if (checked) {
            // Give log panel ~30% of space
            int total = m_splitter->height();
            m_splitter->setSizes({total * 70 / 100, total * 30 / 100});
        }
    });
    connect(clearBtn, &QPushButton::clicked, m_logView, &QPlainTextEdit::clear);
}

// ─────────────────────────────────────────────────────────────────────────────
void DisassemblerTab::startDisassembly()
{
    if (m_running) return;
    if (m_filePath.isEmpty()) { showPlaceholder(tr("No file path set")); return; }

    m_sections.clear();
    m_table->setRowCount(0);
    m_sectionCombo->blockSignals(true);
    m_sectionCombo->clear();
    m_sectionCombo->blockSignals(false);
    m_sectionCombo->setEnabled(false);
    m_searchEdit->setEnabled(false);
    m_currentSectionIndex = -1;

    m_progressBar->setValue(0);
    m_progressBar->setVisible(true);
    m_statusLabel->setText(tr("Running objdump…"));

    // Auto-open log panel when disassembly starts so user sees progress
    if (!m_logToggleBtn->isChecked()) {
        m_logToggleBtn->setChecked(true);
    }

    appendLog("=== Disassembly started: " +
              QDateTime::currentDateTime().toString("hh:mm:ss") + " ===");

    setRunningState(true);
    showPlaceholder(tr("Disassembling…"));

    emit requestDisassembly(m_filePath, {});
}

void DisassemblerTab::cancelDisassembly()
{
    if (m_worker) m_worker->cancel();
}

// ─────────────────────────────────────────────────────────────────────────────
void DisassemblerTab::onLogLine(const QString &line)
{
    appendLog(line);
}

void DisassemblerTab::appendLog(const QString &line)
{
    m_logView->appendPlainText(line);
    // Auto-scroll to bottom
    QTextCursor c = m_logView->textCursor();
    c.movePosition(QTextCursor::End);
    m_logView->setTextCursor(c);
}

// ─────────────────────────────────────────────────────────────────────────────
void DisassemblerTab::onSectionFound(const DisasmSection &section)
{
    if (section.instructions.isEmpty()) return;
    m_sections.append(section);

    if (m_sections.size() == 1) {
        m_stack->setCurrentWidget(m_table);
        populateSectionCombo();
    }

    if (m_currentSectionIndex == -1) {
        m_table->setUpdatesEnabled(false);
        for (const DisasmInstruction &insn : section.instructions) {
            bool isLabel = insn.bytes.isEmpty();
            int row = m_table->rowCount();
            m_table->insertRow(row);
            auto mkItem = [&](const QString &txt) {
                auto *it = new QTableWidgetItem(txt);
                it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                if (isLabel) {
                    it->setForeground(QColor("#5555ff"));
                    QFont f = it->font(); f.setBold(true); it->setFont(f);
                }
                return it;
            };
            m_table->setItem(row, COL_ADDR,     mkItem(insn.address));
            m_table->setItem(row, COL_BYTES,    mkItem(isLabel ? QString() : insn.bytes));
            m_table->setItem(row, COL_MNEMONIC, mkItem(insn.mnemonic));
            m_table->setItem(row, COL_OPERANDS, mkItem(insn.operands));
        }
        m_table->setUpdatesEnabled(true);
    }

    populateSectionCombo();

    int total = 0;
    for (const auto &s : m_sections) total += s.instructions.size();
    m_statusLabel->setText(
        tr("%1 section(s) · %2 instructions loaded…").arg(m_sections.size()).arg(total));
}

void DisassemblerTab::onWorkerFinished()
{
    setRunningState(false);
    m_progressBar->setVisible(false);

    appendLog("=== Disassembly finished ===");

    if (m_sections.isEmpty()) {
        showPlaceholder(
            tr("No disassemblable sections found.\n"
               "Make sure the file is a supported binary (ELF, PE, Mach-O…)\n"
               "Check the Log panel for details."));
        m_statusLabel->clear();
        return;
    }

    int total = 0;
    for (const auto &s : m_sections) total += s.instructions.size();
    m_statusLabel->setText(
        tr("%1 section(s) · %2 instructions").arg(m_sections.size()).arg(total));

    m_sectionCombo->setEnabled(true);
    m_searchEdit->setEnabled(true);
}

void DisassemblerTab::onWorkerError(const QString &msg)
{
    setRunningState(false);
    m_progressBar->setVisible(false);
    appendLog("[ERROR] " + msg);
    showPlaceholder(tr("objdump error — see Log panel for details"));
    m_statusLabel->setText(tr("Error"));
}

void DisassemblerTab::onProgressUpdated(int percent)
{
    m_progressBar->setValue(percent);
}

// ─────────────────────────────────────────────────────────────────────────────
void DisassemblerTab::setRunningState(bool running)
{
    m_running = running;
    m_runBtn->setVisible(!running);
    m_cancelBtn->setVisible(running);
}

void DisassemblerTab::showPlaceholder(const QString &msg)
{
    m_placeholderLbl->setText(msg);
    m_stack->setCurrentWidget(m_placeholderLbl);
}

void DisassemblerTab::populateSectionCombo()
{
    m_sectionCombo->blockSignals(true);
    int prev = m_sectionCombo->currentIndex();
    m_sectionCombo->clear();
    m_sectionCombo->addItem(tr("All sections"));
    for (const auto &s : m_sections)
        m_sectionCombo->addItem(s.name);
    m_sectionCombo->setCurrentIndex(prev >= 0 && prev < m_sectionCombo->count() ? prev : 0);
    m_sectionCombo->blockSignals(false);
}

void DisassemblerTab::onSectionComboChanged(int index)
{
    m_currentSectionIndex = (index <= 0) ? -1 : (index - 1);
    applyFilter();
}

void DisassemblerTab::onSearchTextChanged(const QString &)
{
    applyFilter();
}

void DisassemblerTab::applyFilter()
{
    if (m_sections.isEmpty()) return;

    const QString query = m_searchEdit->text().trimmed().toLower();

    QVector<const DisasmSection *> visible;
    if (m_currentSectionIndex < 0)
        for (const auto &s : m_sections) visible.append(&s);
    else if (m_currentSectionIndex < m_sections.size())
        visible.append(&m_sections[m_currentSectionIndex]);

    m_table->setUpdatesEnabled(false);
    m_table->setRowCount(0);

    for (const DisasmSection *sec : visible) {
        for (const DisasmInstruction &insn : sec->instructions) {
            bool isLabel = insn.bytes.isEmpty();
            if (!query.isEmpty()) {
                bool match = insn.address.toLower().contains(query)
                          || insn.mnemonic.toLower().contains(query)
                          || insn.operands.toLower().contains(query)
                          || insn.bytes.toLower().contains(query);
                if (!match) continue;
            }
            int row = m_table->rowCount();
            m_table->insertRow(row);
            auto mkItem = [&](const QString &txt) {
                auto *it = new QTableWidgetItem(txt);
                it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                if (isLabel) {
                    it->setForeground(QColor("#5555ff"));
                    QFont f = it->font(); f.setBold(true); it->setFont(f);
                }
                return it;
            };
            m_table->setItem(row, COL_ADDR,     mkItem(insn.address));
            m_table->setItem(row, COL_BYTES,    mkItem(isLabel ? QString() : insn.bytes));
            m_table->setItem(row, COL_MNEMONIC, mkItem(insn.mnemonic));
            m_table->setItem(row, COL_OPERANDS, mkItem(insn.operands));
        }
    }

    m_table->setUpdatesEnabled(true);

    if (m_table->rowCount() == 0 && !m_sections.isEmpty())
        showPlaceholder(tr("No results for \"%1\"").arg(m_searchEdit->text()));
    else
        m_stack->setCurrentWidget(m_table);

    m_statusLabel->setText(tr("%1 rows shown").arg(m_table->rowCount()));
}

#include "moc_disassemblertab.cpp"
