#include "viewmenu.h"
#include "ui/MenuBar/menufactory.h"

#include <QActionGroup>

static bool registered = [](){
    MenuFactory::instance().registerMenu("3", [](){
        return new ViewMenu();
    });
    return true;
}();

ViewMenu::ViewMenu() : BaseMenu("View") {
    m_wordWrap = new QAction("Word Wrap", this);
    m_wordWrap->setCheckable(true);
    m_wordWrap->setChecked(true);

    m_tabReplace = new QAction("Insert spaces instead of tab", this);
    m_tabReplace->setCheckable(true);
    m_tabReplace->setChecked(false);

    m_perfMonitor = new QAction("Performance Monitor", this);
    m_perfMonitor->setCheckable(true);
    m_perfMonitor->setChecked(true);

    auto* tabWidthMenu = new QMenu("Tab Width", this);
    QActionGroup* tabWidthGroup = new QActionGroup(this);
    tabWidthGroup->setExclusive(true);
    m_tabWidth2 = tabWidthMenu->addAction("2");
    m_tabWidth4 = tabWidthMenu->addAction("4");
    m_tabWidth8 = tabWidthMenu->addAction("8");
    for (QAction* action : {m_tabWidth2, m_tabWidth4, m_tabWidth8}) {
        action->setCheckable(true);
        tabWidthGroup->addAction(action);
    }
    m_tabWidth4->setChecked(true);

    m_terminal = new QAction("Show terminal", this);
    m_terminal->setCheckable(true);
    m_terminal->setChecked(true);

    this->addAction(m_wordWrap);
    this->addAction(m_tabReplace);
    this->addMenu(tabWidthMenu);
    this->addAction(m_perfMonitor);
    this->addSeparator();
    this->addAction(m_terminal);
}

void ViewMenu::setupConnections(IDEWindow* ideWind){
    connect(m_wordWrap, &QAction::triggered, ideWind, &IDEWindow::on_SetWordWrap);
    connect(m_tabReplace, &QAction::triggered, ideWind, &IDEWindow::on_SetTabReplace);
    connect(m_tabWidth2, &QAction::triggered, ideWind, [ideWind]() { ideWind->on_SetTabWidth(2); });
    connect(m_tabWidth4, &QAction::triggered, ideWind, [ideWind]() { ideWind->on_SetTabWidth(4); });
    connect(m_tabWidth8, &QAction::triggered, ideWind, [ideWind]() { ideWind->on_SetTabWidth(8); });
    connect(m_perfMonitor, &QAction::triggered, ideWind, &IDEWindow::on_Toggle_PerfMonitor);
    connect(m_terminal, &QAction::triggered, ideWind, &IDEWindow::on_Toggle_Terminal);
}
