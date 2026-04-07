#include "reversecalculatordialog.h"

#include <QAbstractItemView>
#include <QClipboard>
#include <QComboBox>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <cmath>

QString ReverseCalculatorDialog::fmtHex(qulonglong v, int bits) {
    const int nyb = qMax(1, bits / 4);
    return QStringLiteral("0x") + QString::number(v, 16).toUpper().rightJustified(nyb, '0');
}

QString ReverseCalculatorDialog::fmtBin(qulonglong v, int bits) {
    QString s;
    s.reserve(bits + bits / 8);
    for (int i = bits - 1; i >= 0; --i) {
        s += ((v >> i) & 1ULL) ? u'1' : u'0';
        if (i % 8 == 0 && i != 0)
            s += u' ';
    }
    return s;
}

QString ReverseCalculatorDialog::fmtOct(qulonglong v, int bits) {
    const int digits = qMax(1, static_cast<int>(std::ceil(bits / 3.0)));
    return QStringLiteral("0o") + QString::number(v, 8).rightJustified(digits, '0');
}

QString ReverseCalculatorDialog::fmtBytes(qulonglong v, int bits) {
    const int nBytes = qMax(1, bits / 8);
    QStringList parts;
    parts.reserve(nBytes);
    for (int i = nBytes - 1; i >= 0; --i) {
        parts << QString::number((v >> (i * 8)) & 0xFF, 16).toUpper().rightJustified(2, '0');
    }
    return parts.join(' ');
}

int ReverseCalculatorDialog::detectBase(const QString &token) {
    const QString t = token.trimmed();
    if (t.startsWith(QLatin1String("0x"), Qt::CaseInsensitive) ||
        t.startsWith(QLatin1String("+0x"), Qt::CaseInsensitive) ||
        t.startsWith(QLatin1String("-0x"), Qt::CaseInsensitive))
        return 16;

    if (t.startsWith(QLatin1String("0b"), Qt::CaseInsensitive) ||
        t.startsWith(QLatin1String("+0b"), Qt::CaseInsensitive) ||
        t.startsWith(QLatin1String("-0b"), Qt::CaseInsensitive))
        return 2;

    return 10;
}

QString ReverseCalculatorDialog::fmtResult(qulonglong v, int bits, int base) {
    const qulonglong masked = maskToWidth(v, bits);
    switch (base) {
    case 16: return fmtHex(masked, bits);
    case 2: return fmtBin(masked, bits);
    default: {
        if (bits >= 64) {
            return QString::number(static_cast<qlonglong>(masked));
        }
        const qulonglong signBit = 1ULL << (bits - 1);
        if (masked & signBit) {
            const qulonglong mask = (1ULL << bits) - 1ULL;
            return QString::number(-static_cast<qlonglong>((~masked + 1ULL) & mask));
        }
        return QString::number(masked);
    }
    }
}

qulonglong ReverseCalculatorDialog::maskToWidth(qulonglong v, int bits) {
    if (bits >= 64) {
        return v;
    }
    return v & ((1ULL << bits) - 1ULL);
}

qlonglong ReverseCalculatorDialog::toSigned(qulonglong v, int bits) {
    if (bits >= 64) {
        return static_cast<qlonglong>(v);
    }
    const qulonglong signBit = 1ULL << (bits - 1);
    const qulonglong mask = (1ULL << bits) - 1ULL;
    v &= mask;
    if (!(v & signBit)) {
        return static_cast<qlonglong>(v);
    }
    return -static_cast<qlonglong>((~v + 1ULL) & mask);
}

qulonglong ReverseCalculatorDialog::swapEndian(qulonglong v, int bits) {
    const int nBytes = qMax(1, bits / 8);
    qulonglong out = 0;
    for (int i = 0; i < nBytes; ++i) {
        out = (out << 8) | ((v >> (i * 8)) & 0xFFULL);
    }
    return maskToWidth(out, bits);
}

bool ReverseCalculatorDialog::parseValue(const QString &text, qulonglong *outValue) {
    if (!outValue) {
        return false;
    }
    const QString t = text.trimmed();
    if (t.isEmpty()) {
        return false;
    }

    static const QRegularExpression binSpaced(R"(^\s*([+-]?)\s*(0[bB][01 ]+)\s*$)");
    const auto bm = binSpaced.match(t);
    if (bm.hasMatch()) {
        QString digits = bm.captured(2).mid(2);
        digits.remove(' ');
        if (digits.isEmpty()) {
            return false;
        }
        bool ok = false;
        const qulonglong v = digits.toULongLong(&ok, 2);
        if (!ok) {
            return false;
        }
        const QString sign = bm.captured(1);
        *outValue = (sign == '-') ? static_cast<qulonglong>(-static_cast<qlonglong>(v)) : v;
        return true;
    }

    static const QRegularExpression re(R"(^\s*([+-]?)\s*(0x[0-9a-fA-F]+|0b[01]+|\d+)\s*$)");
    const auto m = re.match(t);
    if (!m.hasMatch()) {
        return false;
    }

    const QString sign = m.captured(1);
    const QString num = m.captured(2);

    bool ok = false;
    qulonglong v = 0;
    if (num.startsWith(QLatin1String("0x"), Qt::CaseInsensitive)) {
        v = num.mid(2).toULongLong(&ok, 16);
    } else if (num.startsWith(QLatin1String("0b"), Qt::CaseInsensitive)) {
        v = num.mid(2).toULongLong(&ok, 2);
    } else {
        v = num.toULongLong(&ok, 10);
    }

    if (!ok) {
        return false;
    }
    *outValue = (sign == '-') ? static_cast<qulonglong>(-static_cast<qlonglong>(v)) : v;
    return true;
}

bool ReverseCalculatorDialog::parseExpression(const QString &text, qulonglong *outValue, QString *errorOut,
                                              int *lhsBase) {
    if (!outValue || !errorOut) {
        return false;
    }

    static const QRegularExpression tokenRe(R"(\s*(0[xX][0-9a-fA-F]+|0[bB][01 ]+|\d+|[+\-*/%&|^]|<<|>>)\s*)");

    QStringList tokens;
    QRegularExpressionMatchIterator it = tokenRe.globalMatch(text);
    int lastEnd = 0;

    while (it.hasNext()) {
        const auto match = it.next();
        if (match.capturedStart() > lastEnd &&
            text.mid(lastEnd, match.capturedStart() - lastEnd).trimmed().size() > 0) {
            *errorOut = tr("Invalid syntax");
            return false;
        }
        tokens << match.captured(1).trimmed();
        lastEnd = match.capturedEnd();
    }

    if (tokens.isEmpty()) {
        *errorOut = tr("Empty expression");
        return false;
    }

    static const QStringList allOps = {"+", "-", "*", "/", "%", "&", "|", "^", "<<", ">>"};
    QStringList merged;
    for (int j = 0; j < tokens.size(); ++j) {
        const QString &tok = tokens[j];
        if ((tok == "+" || tok == "-") && (merged.isEmpty() || allOps.contains(merged.last()))) {
            if (j + 1 >= tokens.size()) {
                *errorOut = tr("Invalid syntax at end");
                return false;
            }
            merged << tok + tokens[++j];
        } else {
            merged << tok;
        }
    }

    if (merged.size() % 2 == 0) {
        *errorOut = tr("Incomplete expression");
        return false;
    }

    QList<qulonglong> values;
    QList<QString> ops;
    values.reserve((merged.size() + 1) / 2);
    ops.reserve(merged.size() / 2);

    qulonglong first = 0;
    if (!parseValue(merged[0], &first)) {
        *errorOut = tr("Invalid operand: ") + merged[0];
        return false;
    }
    values.append(first);
    if (lhsBase) {
        *lhsBase = detectBase(merged[0]);
    }

    for (int j = 1; j < merged.size(); j += 2) {
        ops.append(merged[j]);
        qulonglong val = 0;
        if (!parseValue(merged[j + 1], &val)) {
            *errorOut = tr("Invalid operand: ") + merged[j + 1];
            return false;
        }
        values.append(val);
    }

    const QList<QStringList> precedence = {{"*", "/", "%"}, {"+", "-"}, {"<<", ">>"}, {"&"}, {"^"}, {"|"}};

    for (const QStringList &level : precedence) {
        for (int j = 0; j < ops.size();) {
            if (!level.contains(ops[j])) {
                ++j;
                continue;
            }

            const QString op = ops[j];
            const qulonglong lhs = values[j];
            const qulonglong rhs = values[j + 1];
            qulonglong res = 0;


            if (op == "*") {
                res = lhs * rhs;
            } else if (op == "/") {
                if (!rhs) {
                    *errorOut = tr("Division by zero");
                    return false;
                }
                res = lhs / rhs;
            } else if (op == "%") {
                if (!rhs) {
                    *errorOut = tr("Modulo by zero");
                    return false;
                }
                res = lhs % rhs;
            } else if (op == "+") {
                res = lhs + rhs;
            } else if (op == "-") {
                res = lhs - rhs;
            } else if (op == "<<") {
                res = (rhs >= 64) ? 0 : (lhs << rhs);
            } else if (op == ">>") {
                res = (rhs >= 64) ? 0 : (lhs >> rhs);
            } else if (op == "&") {
                res = lhs & rhs;
            } else if (op == "^") {
                res = lhs ^ rhs;
            } else if (op == "|") {
                res = lhs | rhs;
            }

            values[j] = res;
            values.removeAt(j + 1);
            ops.removeAt(j);
        }
    }

    if (values.isEmpty()) {
        return false;
    }

    *outValue = values[0];
    return true;
}

bool ReverseCalculatorDialog::looksLikeExpression(const QString &text) {
    static const QRegularExpression exprRe(R"((?:^|\s|\d)(?:\+|-|\*|/|%|&|\||\^|<<|>>)(?:\s|\d|$))");
    return exprRe.match(text).hasMatch();
}

namespace {
QFrame *makeSeparator(QWidget *parent) {
    auto *f = new QFrame(parent);
    f->setFrameShape(QFrame::HLine);
    f->setFrameShadow(QFrame::Plain);
    f->setFixedHeight(1);
    f->setStyleSheet("background-color: #101010;");
    return f;
}

QPushButton *makeCopyBtn(const QString &label, QWidget *parent) {
    auto *btn = new QPushButton(label, parent);
    btn->setFixedWidth(60);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QLabel *makeValueLabel(QWidget *parent) {
    auto *lbl = new QLabel("None", parent);
    lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    lbl->setFont(QFont("monospace", -1));
    lbl->setWordWrap(true);
    lbl->setStyleSheet("color: #888888; font-weight: bold;");
    return lbl;
}

QLabel *makeSectionHeader(const QString &text, QWidget *parent) {
    auto *lbl = new QLabel(text, parent);
    QFont f = lbl->font();
    f.setBold(true);
    lbl->setFont(f);
    return lbl;
}

QLabel *makeNameLabel(const QString &text, QWidget *parent) { return new QLabel(text, parent); }
} // namespace

ReverseCalculatorDialog::ReverseCalculatorDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Reverse Calculator"));
    setModal(false);
    setMinimumWidth(600);

    auto *root = new QVBoxLayout(this);
    root->setSpacing(5);
    root->setContentsMargins(5, 5, 5, 5);

    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(5);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(tr("Enter value: 255, -1, 0xDEAD, 0b1010, or expression"));
    m_input->setFont(QFont("monospace", -1));
    topRow->addWidget(m_input, 1);

    auto *widthLabel = new QLabel(tr("Width:"), this);
    topRow->addWidget(widthLabel);

    m_width = new QComboBox(this);
    for (auto [label, val] : {std::pair{"8 bit", 8}, {"16 bit", 16}, {"32 bit", 32}, {"64 bit", 64}}) {
        m_width->addItem(label, val);
    }

    m_width->setCurrentIndex(2);
    m_width->setMinimumWidth(72);
    topRow->addWidget(m_width);

    root->addLayout(topRow);

    m_historyList = new QListWidget(this);
    m_historyList->setMaximumHeight(80);
    m_historyList->hide();

    m_clearHistoryBtn = new QPushButton(tr("Clear"), this);
    m_clearHistoryBtn->hide();
    m_clearHistoryBtn->setFixedWidth(60);

    auto *histRow = new QHBoxLayout();
    histRow->setSpacing(4);
    histRow->addWidget(m_historyList, 1);

    auto *histVBox = new QVBoxLayout();
    histVBox->addWidget(m_clearHistoryBtn);
    histVBox->addStretch();
    histRow->addLayout(histVBox);
    root->addLayout(histRow);

    m_status = new QLabel(this);
    m_status->setWordWrap(true);
    m_status->hide();
    root->addWidget(m_status);

    root->addWidget(makeSeparator(this));

    struct RowDef {
        const char *name;
        QLabel **valuePtr;
        QPushButton **btnPtr;
        const char *copyLabel;
    };

    const RowDef rows[] = {
        {"HEX:", &m_hex, &m_copyHex, "Copy"},
        {"DECIMAL (UNSIGNED):", &m_decU, &m_copyDecU, "Copy"},
        {"DECIMAL (SIGNED):", &m_decS, &m_copyDecS, "Copy"},
        {"OCTAL:", &m_oct, &m_copyOct, "Copy"},
        {"BINARY:", &m_bin, &m_copyBin, "Copy"},
        {"BYTES:", &m_bytes, &m_copyBytes, "Copy"},
    };

    for (const auto &r : rows) {
        *r.valuePtr = makeValueLabel(this);
        *r.btnPtr = makeCopyBtn(r.copyLabel, this);

        auto *rowWidget = new QWidget(this);
        auto *hbox = new QHBoxLayout(rowWidget);
        hbox->setContentsMargins(4, 2, 4, 2);
        hbox->setSpacing(4);
        hbox->addWidget(makeNameLabel(r.name, this));
        hbox->addWidget(*r.valuePtr, 1);
        hbox->addWidget(*r.btnPtr);

        root->addWidget(rowWidget);
        root->addWidget(makeSeparator(this));
    }

    auto *bottomRow = new QHBoxLayout();
    m_swapBtn = new QPushButton(tr("Swap Endian"), this);
    m_swapBtn->setToolTip(tr("Swap byte order within selected bit width"));
    m_swapBtn->setCursor(Qt::PointingHandCursor);
    bottomRow->addWidget(m_swapBtn);

    bottomRow->addStretch();

    m_copyAllBtn = new QPushButton(tr("Copy All"), this);
    m_copyAllBtn->setCursor(Qt::PointingHandCursor);
    bottomRow->addWidget(m_copyAllBtn);

    root->addLayout(bottomRow);

    connect(m_input, &QLineEdit::textChanged, this, &ReverseCalculatorDialog::onInputChanged);
    connect(m_input, &QLineEdit::returnPressed, this, &ReverseCalculatorDialog::onReturnPressed);
    connect(m_width, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &ReverseCalculatorDialog::onInputChanged);
    connect(m_swapBtn, &QPushButton::clicked, this, &ReverseCalculatorDialog::onSwapEndian);

    connect(m_copyHex, &QPushButton::clicked, this, [this] { copyText(m_hex->text()); });
    connect(m_copyDecU, &QPushButton::clicked, this, [this] { copyText(m_decU->text()); });
    connect(m_copyDecS, &QPushButton::clicked, this, [this] { copyText(m_decS->text()); });
    connect(m_copyOct, &QPushButton::clicked, this, [this] { copyText(m_oct->text()); });
    connect(m_copyBin, &QPushButton::clicked, this, [this] {
        QString raw = m_bin->text();
        raw.remove(' ');
        if (raw != "-")
            raw.prepend("0b");
        copyText(raw);
    });
    connect(m_copyBytes, &QPushButton::clicked, this, [this] { copyText(m_bytes->text()); });

    connect(m_copyAllBtn, &QPushButton::clicked, this, [this] {
        if (m_hex->text() == "None")
            return;
        QString text = QString("HEX: %1\n"
                               "DEC (UNSIGNED): %2\n"
                               "DEC (SIGNED): %3\n"
                               "OCT: %4\n"
                               "BIN: 0b%5\n"
                               "BYTES: %6")
                           .arg(m_hex->text(), m_decU->text(), m_decS->text(), m_oct->text(),
                                m_bin->text().simplified().remove(' '), m_bytes->text());
        copyText(text);
    });

    connect(m_historyList, &QListWidget::itemClicked, this, &ReverseCalculatorDialog::onHistoryItemClicked);
    connect(m_clearHistoryBtn, &QPushButton::clicked, this, [this] {
        m_historyList->clear();
        m_historyList->hide();
        m_clearHistoryBtn->hide();
    });

    onInputChanged();
}

void ReverseCalculatorDialog::copyText(const QString &text) {
    if (text != "None")
        QGuiApplication::clipboard()->setText(text);
}

void ReverseCalculatorDialog::clearOutputs() {
    for (QLabel *lbl : {m_hex, m_decU, m_decS, m_oct, m_bin, m_bytes}) {
        lbl->setText("None");
        lbl->setStyleSheet("color: #888888;");
    }
}

void ReverseCalculatorDialog::updateOutputs(qulonglong value, bool ok) {
    if (!ok) {
        clearOutputs();
        return;
    }

    const int bits = m_width->currentData().toInt();
    const qulonglong v = maskToWidth(value, bits);

    const QString style = "color: #0073E5; font-weight: bold;";
    for (QLabel *lbl : {m_hex, m_decU, m_decS, m_oct, m_bin, m_bytes}) {
        lbl->setStyleSheet(style);
    }

    m_hex->setText(fmtHex(v, bits));
    m_decU->setText(QString::number(v));
    m_decS->setText(QString::number(toSigned(v, bits)));
    m_oct->setText(fmtOct(v, bits));
    m_bin->setText(fmtBin(v, bits));
    m_bytes->setText(fmtBytes(v, bits));
}

void ReverseCalculatorDialog::onInputChanged() {
    const QString text = m_input->text().trimmed();

    if (text.isEmpty()) {
        m_status->clear();
        m_status->hide();
        clearOutputs();
        adjustSize();
        return;
    }

    qulonglong v = 0;
    QString error;
    bool ok = false;
    int base = 10;

    if (looksLikeExpression(text)) {
        ok = parseExpression(text, &v, &error, &base);
        if (!ok) {
            m_status->setStyleSheet("color: red;");
            m_status->setText(tr("Error: ") + error);
            m_status->show();
        } else {
            m_status->setStyleSheet("color: #00E500;");
            const int bits = m_width->currentData().toInt();
            m_status->setText(QStringLiteral("%1 = %2").arg(text.simplified(), fmtResult(v, bits, base)));
            m_status->show();
        }
    } else {
        ok = parseValue(text, &v);
        if (!ok) {
            m_status->setStyleSheet("color: red;");
            m_status->setText(tr("Error: Invalid input"));
            m_status->show();
        } else {
            m_status->setStyleSheet("");
            m_status->clear();
            m_status->hide();
        }
        adjustSize();
    }

    updateOutputs(v, ok);
}

void ReverseCalculatorDialog::onReturnPressed() {
    const QString text = m_input->text().trimmed();
    if (text.isEmpty())
        return;

    qulonglong v = 0;
    QString error;
    bool ok = false;
    int base = detectBase(text);

    if (looksLikeExpression(text)) {
        ok = parseExpression(text, &v, &error, &base);
    } else {
        ok = parseValue(text, &v);
    }

    if (!ok) {
        return;
    }

    const int bits = m_width->currentData().toInt();
    const QString item = QStringLiteral("%1 = %2").arg(text.simplified(), fmtResult(v, bits, base));

    if (m_historyList->count() > 0 && m_historyList->item(m_historyList->count() - 1)->text() == item) {
        return;
    }

    m_historyList->addItem(item);
    while (m_historyList->count() > 10) {
        delete m_historyList->takeItem(0);
    }

    m_historyList->show();
    m_clearHistoryBtn->show();
    m_historyList->scrollToBottom();
}

void ReverseCalculatorDialog::onHistoryItemClicked(QListWidgetItem *item) {
    if (!item) {
        return;
    }

    QString text = item->text();
    const int eq = text.indexOf(QLatin1String(" = "));
    if (eq != -1) {
        text = text.left(eq);
    }
    m_input->setText(text);
}

void ReverseCalculatorDialog::onSwapEndian() {
    qulonglong v = 0;
    if (!parseValue(m_input->text(), &v)) {
        return;
    }
    const int bits = m_width->currentData().toInt();
    m_input->setText(fmtHex(swapEndian(maskToWidth(v, bits), bits), bits));
}
