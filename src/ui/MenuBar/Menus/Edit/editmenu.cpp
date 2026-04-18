#include "editmenu.h"
#include "ui/MenuBar/menufactory.h"
#include <QKeySequence>

static bool registered = []() {
  MenuFactory::instance().registerMenu("2", []() { return new EditMenu(); });
  return true;
}();

EditMenu::EditMenu() : BaseMenu("Edit") {

  m_find = new QAction("Find", this);
  m_find->setShortcut(QKeySequence::Find);

  m_replace = new QAction("Replace", this);
  m_replace->setShortcut(QKeySequence::Replace);

  m_settings = new QAction("Settings", this);
  
    m_settings->setShortcuts({
        QKeySequence(Qt::CTRL | Qt::Key_Comma),
        QKeySequence("Ctrl+б"),
    });
    
  this->addAction(m_find);
  this->addAction(m_replace);
  this->addSeparator();
  this->addAction(m_settings);
}

void EditMenu::setupConnections(IDEWindow *ideWind) {
  connect(m_find, &QAction::triggered, ideWind,
          &IDEWindow::on_Find);
  connect(m_replace, &QAction::triggered, ideWind,
          &IDEWindow::on_Replace);
  connect(m_settings, &QAction::triggered, ideWind,
          &IDEWindow::on_openSettings);
}
