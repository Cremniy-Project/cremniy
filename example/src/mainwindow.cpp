#include "mainwindow.h"
#include "CustomCodeEditor.h"
#include "FileDataBuffer.h"
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
{
    auto* central = new QWidget(this);

    auto* mainLayout = new QHBoxLayout(central);
    auto* cce = new CustomCodeEditor(central);

    mainLayout->addWidget(cce);

    central->setLayout(mainLayout);
    setCentralWidget(central);

    FileDataBuffer* fdb = new FileDataBuffer();

    cce->setBuffer(fdb);
}
