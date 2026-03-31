#ifndef IDEWINDOW_H
#define IDEWINDOW_H

#include "filestabwidget.h"
#include "filetreeview.h"
#include <QMainWindow>
#include <qboxlayout.h>
#include <qmenubar.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <QLabel>
#include <QTimer>
#include "widgets/terminal/terminalwidget.h"

class IDEWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit IDEWindow(QString ProjectPath, QWidget *parent = nullptr);
    ~IDEWindow() override;

private slots:

    /**
     * @brief Двойной клик
     *
     * Обрабатывает открытие файла или разворачивание директории
    */
    void on_treeView_doubleClicked(const QModelIndex &index);

    /**
     * @brief Открытие контекстного меню
     *
     * Нужен при клике на ПКМ для открытия контекстного меню
    */
    void on_Tree_ContextMenu(const QPoint &pos);


private:

    /**
     * @brief Сохранить текущий путь проекта в истории
    */
    void SaveProjectInCache(const QString project_path);
    void setupPerfMonitor();
    void updatePerfMonitor();

    // - - Main Widgets - -
    QMenuBar* m_menuBar;
    QStatusBar* m_statusBar;
    QWidget* m_mainWidget;
    QHBoxLayout* m_mainLayout;
    QSplitter* m_verticalSplitter;  // splitter (вверх вниз)
    QSplitter* m_mainSplitter; 

    // - - General Widgets - -
    FilesTabWidget* m_filesTabWidget;
    FileTreeView* m_filesTreeView;

    // - - Terminal Widget - -
    TerminalWidget* m_terminal;
    QLabel* m_perfLabel = nullptr;
    QTimer* m_perfTimer = nullptr;


public slots:

    /**
     * @brief Создать новый проект (QMenuBar->File->NewProject)
    */
    void on_NewProject();

    /**
     * @brief Открыть другой проект (QMenuBar->File->OpenProject)
    */
    void on_OpenProject();

    /**
     * @brief Сохранить файл (QMenuBar->File->SaveFile)
    */
    void on_SaveFile();

    /**
     * @brief Закрыть проект (QMenuBar->File->CloseProject)
    */
    void on_ClosingProject();

    /**
     * @brief Нажатие на Settings (QMenuBar->Edit->Settings)
     *
     * Открывает окно Settings
    */
    void on_openSettings();

    /**
     * @brief Отображение терминала
     */
    void on_Toggle_Terminal(bool checked);

    /**
     * @brief Переключение переноса строк в редакторах кода
     */
    void on_SetWordWrap(bool checked);

    /**
     * @brief Переключение вставки пробелов вместо tab в редакторах кода
     */
    void on_SetTabReplace(bool checked);

    /**
     * @brief Изменение визуальной ширины tab в редакторах кода
     */
    void on_SetTabWidth(int width);

    /**
     * @brief Включение/отключение панели метрик производительности
     */
    void on_Toggle_PerfMonitor(bool checked);


signals:
    void saveFileSignal();

};
#endif // IDEWINDOW_H
