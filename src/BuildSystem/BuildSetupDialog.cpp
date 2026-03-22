#include "BuildSetupDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>

BuildSetupDialog::BuildSetupDialog(const BuildConfig& initial, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Build Configuration");
    setMinimumWidth(420);

    auto* form = new QFormLayout;
    m_buildEdit = new QLineEdit(initial.build);
    m_runEdit   = new QLineEdit(initial.run);
    m_cleanEdit = new QLineEdit(initial.clean);

    m_buildEdit->setPlaceholderText("e.g. cmake --build build");
    m_runEdit->setPlaceholderText("e.g. ./build/myapp");
    m_runEdit->setToolTip("For CMake projects use the actual binary name from your CMakeLists.txt");
    m_cleanEdit->setPlaceholderText("e.g. cmake --build build --target clean");

    form->addRow("Build command:", m_buildEdit);
    form->addRow("Run command:", m_runEdit);
    auto* runHint = new QLabel("💡 For CMake: replace app.exe with your actual binary name from CMakeLists.txt");
    runHint->setStyleSheet("color: gray; font-size: 11px;");
    runHint->setWordWrap(true);
    form->addRow("", runHint);
    form->addRow("Clean command:", m_cleanEdit);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* label = new QLabel(
        "Configure how to build and run your project.\n"
        "Settings will be saved to <b>cremniy.json</b> in the project folder.");
    label->setTextFormat(Qt::RichText);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

BuildConfig BuildSetupDialog::result() const {
    return { m_buildEdit->text().trimmed(),
             m_runEdit->text().trimmed(),
             m_cleanEdit->text().trimmed() };
}