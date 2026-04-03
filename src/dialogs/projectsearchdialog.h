#ifndef PROJECTSEARCHDIALOG_H
#define PROJECTSEARCHDIALOG_H

#include <QDialog>
#include <QHash>

class QCloseEvent;
class QResizeEvent;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QLabel;
class QPushButton;
class ProjectSearchWorker;
class ProjectSearchResultDelegate;
class QThread;

class ProjectSearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectSearchDialog(const QString &projectRoot, QWidget *parent = nullptr);
    ~ProjectSearchDialog() override;

signals:
    void openFileRequested(const QString &path, int lineNumber);

private slots:
    void onSearchClicked();
    void onHitsBatch(const QStringList &filePaths, const QList<int> &lineNumbers,
                     const QStringList &linePreviews);
    void onSearchFinished();
    void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void stopActiveSearch();
    QString formatFileGroupLabel(const QString &absPath) const;

    QString m_projectRoot;
    QString m_activeQuery;
    int m_matchCount = 0;

    QLineEdit *m_queryEdit = nullptr;
    QTreeWidget *m_tree = nullptr;
    ProjectSearchResultDelegate *m_previewDelegate = nullptr;
    QHash<QString, QTreeWidgetItem *> m_fileNodes;

    QLabel *m_statusLabel = nullptr;
    QPushButton *m_searchBtn = nullptr;

    QThread *m_thread = nullptr;
    ProjectSearchWorker *m_worker = nullptr;
};

#endif
