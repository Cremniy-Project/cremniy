#ifndef REVERSECALCULATORDIALOG_H
#define REVERSECALCULATORDIALOG_H

#include <QDialog>

class QComboBox;
class QLineEdit;
class QLabel;
class QPushButton;
class QListWidget;
class QListWidgetItem;
class QGridLayout;

class ReverseCalculatorDialog final : public QDialog {
    Q_OBJECT
public:
    explicit ReverseCalculatorDialog(QWidget *parent = nullptr);

private slots:
    void onInputChanged();
    void onReturnPressed();
    void onHistoryItemClicked(QListWidgetItem *item);
    void onSwapEndian();

private:
    void copyText(const QString &text);
    void updateOutputs(qulonglong value, bool ok);
    void clearOutputs();

    static bool parseValue(const QString &text, qulonglong *outValue);
    static bool parseExpression(const QString &text, qulonglong *outValue, QString *errorOut, int *lhsBase = nullptr);

    static bool looksLikeExpression(const QString &text);
    static int detectBase(const QString &token);
    static qulonglong maskToWidth(qulonglong v, int bits);
    static qlonglong toSigned(qulonglong v, int bits);
    static qulonglong swapEndian(qulonglong v, int bits);
    static QString fmtHex(qulonglong v, int bits);
    static QString fmtBin(qulonglong v, int bits);
    static QString fmtOct(qulonglong v, int bits);
    static QString fmtBytes(qulonglong v, int bits);
    static QString fmtResult(qulonglong v, int bits, int base);

    QLineEdit *m_input = nullptr;
    QComboBox *m_width = nullptr;

    QListWidget *m_historyList = nullptr;
    QPushButton *m_clearHistoryBtn = nullptr;

    QLabel *m_status = nullptr;

    QLabel *m_hex = nullptr;
    QLabel *m_decU = nullptr;
    QLabel *m_decS = nullptr;
    QLabel *m_oct = nullptr;
    QLabel *m_bin = nullptr;
    QLabel *m_bytes = nullptr;

    QPushButton *m_copyHex = nullptr;
    QPushButton *m_copyDecU = nullptr;
    QPushButton *m_copyDecS = nullptr;
    QPushButton *m_copyOct = nullptr;
    QPushButton *m_copyBin = nullptr;
    QPushButton *m_copyBytes = nullptr;
    QPushButton *m_copyAllBtn = nullptr;

    QPushButton *m_swapBtn = nullptr;
};

#endif // REVERSECALCULATORDIALOG_H
