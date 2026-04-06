#include <QApplication>
#include <QTranslator>
#include <QCoreApplication>
#include <QFile>
#include <QIcon>

#include "app/WelcomeWindow/welcomeform.h"
#include "locale/LanguageManager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("cremniy");

    LanguageManager::instance().loadUserDefaultLocale();

    QCoreApplication::setApplicationName("Cremniy");
    a.setWindowIcon(QIcon(":/icons/icon.png"));

    // Themes
    QIcon::setThemeSearchPaths({":/icons"});
    QIcon::setThemeName("phoicons");         // маленькими буквами!

    QFile file(":/styles/style.qss");
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open the style file: " << file.errorString();
        return 1;
    }
    QString styleSheet = QLatin1String(file.readAll());
    a.setStyleSheet(styleSheet);

    WelcomeForm wf;
    wf.show();
    return QCoreApplication::exec();
}
