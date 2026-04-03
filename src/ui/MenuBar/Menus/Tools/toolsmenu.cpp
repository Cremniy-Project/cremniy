#include "toolsmenu.h"
#include "Tools/Windows/ReverseCalculator/reversecalculatordialog.h"
#include "Tools/Windows/DataConverter/dataconverterdialog.h"
#include "Tools/Windows/ShellCodeGenerator/shellcodegeneratordialog.h"

#include "ui/MenuBar/menufactory.h"
#include "ui/ToolsTabWidget/ToolTabFactory.h"
#include <QKeySequence>
#include <QAction>

static bool registered = []() {
  MenuFactory::instance().registerMenu("5", []() { return new ToolsMenu(); });
  return true;
}();

ToolsMenu::ToolsMenu() : BaseMenu(tr("Tools")) {
  m_reverseCalculator = new QAction(tr("Reverse Calculator"), this);
  m_reverseCalculator->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));

  m_dataConverter = new QAction("Data Converter", this);
  m_dataConverter->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));

  m_shellcodeGenerator = new QAction("Shellcode Generator", this);
  m_shellcodeGenerator->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));

  const auto toolDescriptors = ToolTabFactory::instance().availableTabs(ToolTabGroup::Other);
  for (const auto& descriptor : toolDescriptors) {
    auto* action = new QAction(descriptor.name, this);
    action->setProperty("toolTabId", descriptor.id);
    m_toolTabActions.append(action);
    addAction(action);
  }

  if (!m_toolTabActions.isEmpty()) {
    addSeparator();
  }

  addAction(m_reverseCalculator);
  addAction(m_dataConverter);
  addAction(m_shellcodeGenerator);
}

void ToolsMenu::setupConnections(IDEWindow *ideWind) {
  m_ideWindow = ideWind;

  for (QAction* action : m_toolTabActions) {
    connect(action, &QAction::triggered, this, [this, action]() {
      if (!m_ideWindow) {
        return;
      }

      m_ideWindow->openToolForCurrentFile(action->property("toolTabId").toString());
    });
  }

  connect(m_reverseCalculator, &QAction::triggered, this,
          &ToolsMenu::on_Open_ReverseCalculator);
  connect(m_dataConverter, &QAction::triggered, this,
          &ToolsMenu::on_Open_DataConverter);
  connect(m_shellcodeGenerator, &QAction::triggered, this,
          &ToolsMenu::on_Open_ShellcodeGenerator);
}

void ToolsMenu::on_Open_ReverseCalculator() {
  auto *dlg = new ReverseCalculatorDialog(m_ideWindow);
  dlg->setAttribute(Qt::WA_DeleteOnClose, true);
  if (m_ideWindow) {
    dlg->adjustSize();
    dlg->move(m_ideWindow->geometry().center() - dlg->rect().center());
  }
  dlg->show();
  dlg->raise();
  dlg->activateWindow();
}

void ToolsMenu::on_Open_DataConverter() {
  auto *dlg = new DataConverterDialog(m_ideWindow);
  dlg->setAttribute(Qt::WA_DeleteOnClose, true);
  if (m_ideWindow) {
    dlg->adjustSize();
    dlg->move(m_ideWindow->geometry().center() - dlg->rect().center());
  }
  dlg->show();
  dlg->raise();
  dlg->activateWindow();
}
void ToolsMenu::on_Open_ShellcodeGenerator() {
  auto *dlg = new ShellcodeGeneratorDialog(m_ideWindow);
  dlg->setAttribute(Qt::WA_DeleteOnClose, true);
  if (m_ideWindow) {
    dlg->adjustSize();
    dlg->move(m_ideWindow->geometry().center() - dlg->rect().center());
  }
  dlg->show();
  dlg->raise();
  dlg->activateWindow();
}
