#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QIcon>
#include <QImageReader>
#include <QDirIterator>
#include <QDebug>

#include "app/WelcomeWindow/welcomeform.h"

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Munirov");
    QCoreApplication::setApplicationName("Cremniy");
    a.setWindowIcon(QIcon(":/icons/icon.svg"));
    // - - Themes - -

    // Icons
    QIcon::setThemeSearchPaths({":/icons"});
    QIcon::setThemeName("phoicons");         // маленькими буквами!
    qDebug() << "=== SYSTEM DEBUG ===";
    qDebug() << "Supported formats:" << QImageReader::supportedImageFormats();
    qDebug() << "=== THEME DEBUG ===";
    qDebug() << "Theme Search Paths:" << QIcon::themeSearchPaths();
    qDebug() << "Current Theme Name:" << QIcon::themeName();
    QFile themeFile(":/icons/phoicons/index.theme");
    qDebug() << "index.theme exists in resources:" << themeFile.exists();
    qDebug() << "=== RESOURCE TREE ===";
    QDirIterator it(":/icons", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        qDebug() << "Found resource:" << it.next();
    }
    qDebug() << "====================";

    // Style
    QFile baseStyleFile(":/styles/base.qss");
    if (!baseStyleFile.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open the baseStyle file: " << baseStyleFile.errorString();
        return 1;
    }

    QFile themeFile(":/styles/dark.qss");
    if (!themeFile.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open the theme file: " << themeFile.errorString();
        return 1;
    }

    QString baseStyle   = QLatin1String(baseStyleFile.readAll());
    QString theme  = QLatin1String(themeFile.readAll());

    baseStyleFile.close();
    baseStyleFile.close();

    a.setStyleSheet(baseStyle + "\n" + theme);

    WelcomeForm wf;
    wf.show();
    return QCoreApplication::exec();
}