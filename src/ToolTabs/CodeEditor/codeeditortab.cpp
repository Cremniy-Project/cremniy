#include "codeeditortab.h"
#include "QCodeEditor.hpp"
#include <QStyle>
#include <qboxlayout.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qstackedlayout.h>
#include "filemanager.h"
#include "globalwidgetsmanager.h"
#include "utils.h"
#include <QLineEdit>
#include <QShortcut>
#include <QKeySequence>
#include <QTextCursor>
#include <QHBoxLayout>

#include "core/ToolTabFactory.h"

static bool registered = [](){
    ToolTabFactory::instance().registerTab("1", [](){
        return new CodeEditorTab();
    });
    return true;
}();

CodeEditorTab::CodeEditorTab(QWidget *parent)
    : ToolTab{parent}
{

    // - - Create "Code Editor" Page - -

    m_codeEditorWidget = new QCodeEditor(this);

    QTextOption opt = m_codeEditorWidget->document()->defaultTextOption();
    opt.setTabStopDistance(20);
    m_codeEditorWidget->document()->setDefaultTextOption(opt);

    m_codeEditorWidget->document()->markContentsDirty(0, m_codeEditorWidget->document()->characterCount());
    m_codeEditorWidget->viewport()->update();

    // - - Create "Binary File Detected" Page - -

    m_overlayWidget = new QWidget(this);

    auto overlayLayout = new QVBoxLayout(m_overlayWidget);
    overlayLayout->setAlignment(Qt::AlignCenter);

    QLabel* title = new QLabel("Binary file detected");
    title->setStyleSheet("color: white; font-size: 20px;");
    title->setAlignment(Qt::AlignCenter);
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    overlayLayout->addWidget(title);
    overlayLayout->addSpacing(15);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setAlignment(Qt::AlignCenter);

    QPushButton* anywayOpenBtn = new QPushButton("Open anyway");

    btnLayout->addWidget(anywayOpenBtn);
    overlayLayout->addLayout(btnLayout);

    // - - Create Search Bar Widget - -
    m_searchWidget = new QWidget(this);
    m_searchWidget->setObjectName("searchWidget");

    
    QHBoxLayout* searchLayout = new QHBoxLayout(m_searchWidget);
    searchLayout->setContentsMargins(6, 6, 6, 6);
    searchLayout->setSpacing(5);

    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText("Find...");
    
    m_findPrevBtn = new QPushButton("▲ Prev", this);
    m_findNextBtn = new QPushButton("▼ Next", this);
    m_closeSearchBtn = new QPushButton("✕", this);
    
    m_closeSearchBtn->setFixedWidth(30);

    searchLayout->addWidget(m_searchLineEdit);
    searchLayout->addWidget(m_findPrevBtn);
    searchLayout->addWidget(m_findNextBtn);
    searchLayout->addWidget(m_closeSearchBtn);
    
    m_searchWidget->hide();

    // - - Create and Init Stacked Layout Widget - -
    auto stack = new QStackedLayout;
    stack->setStackingMode(QStackedLayout::StackAll);
    stack->addWidget(m_codeEditorWidget);
    stack->addWidget(m_overlayWidget);

    m_overlayWidget->hide();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addLayout(stack);
    mainLayout->addWidget(m_searchWidget); 

    this->setLayout(mainLayout);

    // - - Search Bar Connects & Shortcuts - -

    // Ctrl+F
    QShortcut* searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    searchShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(searchShortcut, &QShortcut::activated, this, &CodeEditorTab::showSearchBar);

    // Esc
    QShortcut* escapeShortcut = new QShortcut(QKeySequence("Esc"), m_searchWidget);
    escapeShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(escapeShortcut, &QShortcut::activated, this, &CodeEditorTab::hideSearchBar);

    connect(m_closeSearchBtn, &QPushButton::clicked, this, &CodeEditorTab::hideSearchBar);

    connect(m_searchLineEdit, &QLineEdit::returnPressed, this, [this]() {
        performSearch(false);
    });

    connect(m_findNextBtn, &QPushButton::clicked, this, [this]() { performSearch(false); });
    connect(m_findPrevBtn, &QPushButton::clicked, this, [this]() { performSearch(true); });

    connect(m_searchLineEdit, &QLineEdit::textChanged, this, [this]() {
        performSearch(false);
    });


    // Trigger: Menu Bar: View->wordWrap - setWordWrapMode
    connect(GlobalWidgetsManager::instance().get_IDEWindow_menuBar_view_wordWrap(),
            &QAction::changed,
            this, [this]{
                if (GlobalWidgetsManager::instance().get_IDEWindow_menuBar_view_wordWrap()->isChecked())
                    m_codeEditorWidget->setWordWrapMode(QTextOption::WordWrap);
                else
                    m_codeEditorWidget->setWordWrapMode(QTextOption::NoWrap);
            });

    // Clicked: Open File Anyway Button
    connect(anywayOpenBtn, &QPushButton::clicked, this, [this]{
        forceSetData = true;
        this->setTabData();
    });

    // modificationChanged: signal modifyData
    connect(m_codeEditorWidget->document(),
            &QTextDocument::modificationChanged,
            this,
            [this](bool modified){
                if (!m_codeEditorWidget->m_ignoreModification)
                    setModifyIndicator(true);
                    emit modifyData();
            });

    // ContentsChanged: if new hash == old hash: dataEqual, else: signal modifyData
    connect(m_codeEditorWidget->document(),
            &QTextDocument::contentsChanged,
            this,
            [this](){
                QByteArray data = m_codeEditorWidget->getBData();
                uint newDataHash = qHash(data, 0);
                if (m_dataHash == newDataHash) {
                    setModifyIndicator(false);
                    emit dataEqual();
                }
                else{
                    if (!m_codeEditorWidget->m_ignoreModification)
                        setModifyIndicator(true);
                        emit modifyData();
                }
            });

}

// - - override functions - -

// - public slots -

void CodeEditorTab::setFile(QString filepath){
    m_fileContext = new FileContext(filepath);
    QFileInfo fileInfo(filepath);
    QString ext = (fileInfo.suffix()).toLower();
    m_codeEditorWidget->setFileExt(ext);
}

void CodeEditorTab::setTabData(){

    qDebug() << "CodeEditorTab: setTabData";

    QByteArray data = FileManager::openFile(m_fileContext);

    if (isBinary(data) && !forceSetData){
        m_codeEditorWidget->hide();
        m_overlayWidget->show();
    }
    else{
        m_dataHash = qHash(data, 0);
        m_codeEditorWidget->show();
        m_overlayWidget->hide();
        m_codeEditorWidget->setBData(data);
        forceSetData = false;
    }

    setModifyIndicator(false);
    emit dataEqual();
}

void CodeEditorTab::saveTabData() {
    qDebug() << "CodeEditorTab: saveTabData";

    QByteArray data = m_codeEditorWidget->getBData();
    uint newDataHash = qHash(data, 0);
    if (newDataHash == m_dataHash) return;
    m_dataHash = newDataHash;

    FileManager::saveFile(m_fileContext, &data);

    m_codeEditorWidget->document()->setModified(false);

    setModifyIndicator(false);
    emit dataEqual();
    emit refreshDataAllTabsSignal();
}

void CodeEditorTab::showSearchBar()
{
    // Если открыта панель "Binary File", поиск не нужен
    if (!m_overlayWidget->isHidden()) return; 

    m_searchWidget->show();
    m_searchLineEdit->setFocus();
    m_searchLineEdit->selectAll(); // Выделяем текст, если там уже что-то было
}

void CodeEditorTab::hideSearchBar()
{
    m_searchWidget->hide();
    m_codeEditorWidget->setFocus(); // Возвращаем фокус в редактор
}

void CodeEditorTab::performSearch(bool backward)
{
    QString query = m_searchLineEdit->text();
    if (query.isEmpty()) {
        m_searchLineEdit->setStyleSheet(""); // Сброс стиля
        return;
    }

    QTextDocument::FindFlags flags;
    if (backward) flags |= QTextDocument::FindBackward;
    
    // Выполняем поиск
    bool found = m_codeEditorWidget->find(query, flags);

    if (!found) {
        QTextCursor cursor = m_codeEditorWidget->textCursor();
        cursor.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
        m_codeEditorWidget->setTextCursor(cursor);
        
        found = m_codeEditorWidget->find(query, flags);
    }

    if (!found) {
        m_searchLineEdit->setProperty("state", "error");
    } else {
        m_searchLineEdit->setProperty("state", "normal");
    }
    // Обновляем виджет, чтобы Qt применил новые стили из QSS
    m_searchLineEdit->style()->unpolish(m_searchLineEdit);
    m_searchLineEdit->style()->polish(m_searchLineEdit);
}
