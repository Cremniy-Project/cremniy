#ifndef DISASSEMBLERTAB_H
#define DISASSEMBLERTAB_H

#include "tooltab.h"
#include "disassemblerworker.h"
#include <QWidget>
#include <QThread>

class QTableWidget;
class QPlainTextEdit;
class QLabel;
class QProgressBar;
class QPushButton;
class QComboBox;
class QLineEdit;
class QStackedWidget;
class QSplitter;

class DisassemblerTab : public QWidget, public ToolTab
{
    Q_OBJECT

public:
    explicit DisassemblerTab(QWidget *parent, QString path);
    ~DisassemblerTab();

    void saveToFile(QString path) override {}
    void setTabData(QByteArray &data) override;

signals:
    void modifyData(bool modified);
    void requestDisassembly(const QString &filePath, const QString &arch);

private slots:
    void onSectionFound(const DisasmSection &section);
    void onWorkerFinished();
    void onWorkerError(const QString &msg);
    void onProgressUpdated(int percent);
    void onLogLine(const QString &line);
    void onSearchTextChanged(const QString &text);
    void onSectionComboChanged(int index);
    void startDisassembly();
    void cancelDisassembly();

private:
    void setupUi();
    void setRunningState(bool running);
    void showPlaceholder(const QString &msg);
    void populateSectionCombo();
    void applyFilter();
    void appendLog(const QString &line);

    QString m_filePath;

    QThread            *m_thread  = nullptr;
    DisassemblerWorker *m_worker  = nullptr;
    bool                m_running = false;

    QVector<DisasmSection> m_sections;

    // UI
    QWidget        *m_toolbar        = nullptr;
    QComboBox      *m_sectionCombo   = nullptr;
    QLineEdit      *m_searchEdit     = nullptr;
    QPushButton    *m_runBtn         = nullptr;
    QPushButton    *m_cancelBtn      = nullptr;
    QPushButton    *m_logToggleBtn   = nullptr;
    QProgressBar   *m_progressBar    = nullptr;
    QLabel         *m_statusLabel    = nullptr;
    QSplitter      *m_splitter       = nullptr;
    QStackedWidget *m_stack          = nullptr;
    QTableWidget   *m_table          = nullptr;
    QLabel         *m_placeholderLbl = nullptr;
    QWidget        *m_logPanel       = nullptr;
    QPlainTextEdit *m_logView        = nullptr;

    int m_currentSectionIndex = -1;
};

#endif // DISASSEMBLERTAB_H
