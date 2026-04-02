#include "asciicharsref.h"
#include "ui/MenuBar/Menus/References/referencewindowfactory.h"
#include <qboxlayout.h>
#include <qclipboard.h>
#include <qguiapplication.h>
#include <qheaderview.h>
#include <qlineedit.h>
#include <qmenu.h>
#include <qtabbar.h>
#include <qtablewidget.h>
#include <memory>
#include <functional>
#include <QRegularExpressionValidator>

static bool registered = []() {
    ReferenceWindowFactory::instance().registerRefWin("1", []() { return new AsciiCharsRef(); });
    return true;
}();

AsciiCharsRef::AsciiCharsRef()
{
    initWindow();
    initWidgets();
}

void AsciiCharsRef::initWindow(){
    this->setWindowTitle("ASCII Characters");
    this->setMinimumSize(840, 580);
}

struct RowData {
    QString displayChar;
    int decimal = 0;
    QString hex;
};

static QVector<RowData> buildAsciiRows() {
    static const char* controlNames[33] = {
        "NUL","SOH","STX","ETX","EOT","ENQ","ACK","BEL",
        "BS","TAB","LF","VT","FF","CR","SO","SI",
        "DLE","DC1","DC2","DC3","DC4","NAK","SYN","ETB",
        "CAN","EM","SUB","ESC","FS","GS","RS","US","SPACE"
    };

    static const uint16_t cp1251Table[128] = {
        0x0402,0x0403,0x201A,0x0453,0x201E,0x2026,0x2020,0x2021,
        0x20AC,0x2030,0x0409,0x2039,0x040A,0x040C,0x040B,0x040F,
        0x0452,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
        0xFFFD,0x2122,0x0459,0x203A,0x045A,0x045C,0x045B,0x045F,
        0x00A0,0x040E,0x045E,0x0408,0x00A4,0x0490,0x00A6,0x00A7,
        0x0401,0x00A9,0x0404,0x00AB,0x00AC,0x00AD,0x00AE,0x0407,
        0x00B0,0x00B1,0x0406,0x0456,0x0491,0x00B5,0x00B6,0x00B7,
        0x0451,0x2116,0x0454,0x00BB,0x0458,0x0405,0x0455,0x0457,
        0x0410,0x0411,0x0412,0x0413,0x0414,0x0415,0x0416,0x0417,
        0x0418,0x0419,0x041A,0x041B,0x041C,0x041D,0x041E,0x041F,
        0x0420,0x0421,0x0422,0x0423,0x0424,0x0425,0x0426,0x0427,
        0x0428,0x0429,0x042A,0x042B,0x042C,0x042D,0x042E,0x042F,
        0x0430,0x0431,0x0432,0x0433,0x0434,0x0435,0x0436,0x0437,
        0x0438,0x0439,0x043A,0x043B,0x043C,0x043D,0x043E,0x043F,
        0x0440,0x0441,0x0442,0x0443,0x0444,0x0445,0x0446,0x0447,
        0x0448,0x0449,0x044A,0x044B,0x044C,0x044D,0x044E,0x044F
    };

    QVector<RowData> rows;
    rows.reserve(256);
    for(int i = 0; i < 256; ++i){
        QString displayChar;
        if(i < 32){
            displayChar = controlNames[i];
        } else if(i == 32){
            displayChar = "SPACE";
        } else if(i == 127){
            displayChar = "DEL";
        } else if(i >= 128){
            uint16_t codepoint = cp1251Table[i - 128];
            if(codepoint == 0xFFFD){
                displayChar = "UNDEF";
            } else {
                displayChar = QString(QChar(codepoint));
            }
        } else {
            displayChar = QString(QChar(i));
        }

        RowData row;
        row.displayChar = displayChar;
        row.decimal = i;
        row.hex = QString::number(i,16).toUpper();
        rows.push_back(row);
    }
    return rows;
}

static QVector<RowData> buildCyrillicRows() {
    QVector<RowData> rows;
    rows.reserve(68);

    // Upper case: A-ya
    for(int cp = 0x0410; cp <= 0x042F; ++cp){
        RowData row;
        row.displayChar = QString(QChar(cp));
        row.decimal = cp;
        row.hex = QString::number(cp, 16).toUpper();
        rows.push_back(row);
    }
    // Upper: Yo
    {
        RowData row;
        row.displayChar = QString(QChar(0x0401));
        row.decimal = 0x0401;
        row.hex = QString::number(0x0401, 16).toUpper();
        rows.push_back(row);
    }
    // Lower case: a-ya
    for(int cp = 0x0430; cp <= 0x044F; ++cp){
        RowData row;
        row.displayChar = QString(QChar(cp));
        row.decimal = cp;
        row.hex = QString::number(cp, 16).toUpper();
        rows.push_back(row);
    }
    // Lower: yo
    {
        RowData row;
        row.displayChar = QString(QChar(0x0451));
        row.decimal = 0x0451;
        row.hex = QString::number(0x0451, 16).toUpper();
        rows.push_back(row);
    }

    return rows;
}

void AsciiCharsRef::initWidgets(){

    QVBoxLayout* layout = new QVBoxLayout(this);

    QTabBar* tabBar = new QTabBar(this);
    tabBar->addTab("ASCII (0-255)");
    tabBar->addTab("Cyrillic (Unicode)");
    tabBar->setExpanding(false);
    layout->addWidget(tabBar);

    QTableWidget* table = new QTableWidget(this);
    table->setColumnCount(3);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setContextMenuPolicy(Qt::CustomContextMenu);

    QStringList headers;
    headers << "Char" << "Decimal" << "Hex";
    table->setHorizontalHeaderLabels(headers);

    auto currentRows = std::make_shared<QVector<RowData>>(buildAsciiRows());

    auto renderTable = [table](const QVector<RowData>& rows) {
        table->setRowCount(rows.size());
        for(int i = 0; i < rows.size(); ++i){
            QTableWidgetItem* charItem = new QTableWidgetItem(rows[i].displayChar);
            QTableWidgetItem* decItem  = new QTableWidgetItem(QString::number(rows[i].decimal));
            QTableWidgetItem* hexItem  = new QTableWidgetItem(rows[i].hex);

            charItem->setFlags(charItem->flags() & ~Qt::ItemIsEditable);
            decItem->setFlags(decItem->flags() & ~Qt::ItemIsEditable);
            hexItem->setFlags(hexItem->flags() & ~Qt::ItemIsEditable);

            table->setItem(i,0,charItem);
            table->setItem(i,1,decItem);
            table->setItem(i,2,hexItem);
        }
    };

    renderTable(*currentRows);

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);

    layout->addWidget(table);

    QHBoxLayout* lineEditLayout = new QHBoxLayout();
    layout->addLayout(lineEditLayout);

    QLineEdit* symbolInput = new QLineEdit(this);
    symbolInput->setPlaceholderText("Search by Character");
    lineEditLayout->addWidget(symbolInput);

    QLineEdit* decInput = new QLineEdit(this);
    decInput->setPlaceholderText("Search by Decimal (0-255)");
    decInput->setValidator(new QIntValidator(0,255,this));
    lineEditLayout->addWidget(decInput);

    QLineEdit* hexInput = new QLineEdit(this);
    hexInput->setPlaceholderText("Search by Hex (0-FF)");
    hexInput->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{1,2}"),hexInput));
    lineEditLayout->addWidget(hexInput);

    this->setLayout(layout);

    QObject::connect(symbolInput, &QLineEdit::textChanged, this, [=](const QString &text){
        if(text.length() > 1){
            QString last = text.right(1);
            symbolInput->setText(last);
        }
        table->clearSelection();
        if(symbolInput->text().isEmpty()) return;
        QString target = symbolInput->text().left(1);
        for(int i = 0; i < currentRows->size(); ++i){
            if((*currentRows)[i].displayChar == target){
                QTableWidgetSelectionRange range(i,0,i,2);
                table->setRangeSelected(range,true);
                table->scrollToItem(table->item(i,0));
                break;
            }
        }
    });

    QObject::connect(decInput, &QLineEdit::textChanged, this, [=](const QString &text){
        table->clearSelection();
        if(text.isEmpty()) return;
        bool ok;
        int num = text.toInt(&ok);
        if(ok){
            for(int i = 0; i < currentRows->size(); ++i){
                if((*currentRows)[i].decimal == num){
                    QTableWidgetSelectionRange range(i,0,i,2);
                    table->setRangeSelected(range,true);
                    table->scrollToItem(table->item(i,0));
                    break;
                }
            }
        }
    });

    QObject::connect(hexInput, &QLineEdit::textChanged, this, [=](const QString &text){
        table->clearSelection();
        if(text.isEmpty()) return;
        bool ok;
        int num = text.toInt(&ok,16);
        if(ok){
            for(int i = 0; i < currentRows->size(); ++i){
                if((*currentRows)[i].decimal == num){
                    QTableWidgetSelectionRange range(i,0,i,2);
                    table->setRangeSelected(range,true);
                    table->scrollToItem(table->item(i,0));
                    break;
                }
            }
        }
    });

    QObject::connect(tabBar, &QTabBar::currentChanged, this, [=](int index){
        if(index == 0){
            *currentRows = buildAsciiRows();
            decInput->setPlaceholderText("Search by Decimal (0-255)");
            decInput->setValidator(new QIntValidator(0,255,decInput));
            hexInput->setPlaceholderText("Search by Hex (0-FF)");
            hexInput->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{1,2}"),hexInput));
        } else {
            *currentRows = buildCyrillicRows();
            decInput->setPlaceholderText("Search by Decimal (Unicode)");
            decInput->setValidator(new QIntValidator(0,0x10FFFF,decInput));
            hexInput->setPlaceholderText("Search by Hex (Unicode)");
            hexInput->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{1,6}"),hexInput));
        }

        symbolInput->clear();
        decInput->clear();
        hexInput->clear();
        table->clearSelection();
        renderTable(*currentRows);
    });

    QObject::connect(table, &QTableWidget::customContextMenuRequested, this, [=](const QPoint& pos){
        QTableWidgetItem* item = table->itemAt(pos);
        if(!item) return;
        int row = item->row();

        QMenu menu(table);
        QAction* copyChar = menu.addAction("Copy Char");
        QAction* copyDec = menu.addAction("Copy Decimal");
        QAction* copyHex = menu.addAction("Copy Hex");
        QAction* copyRow = menu.addAction("Copy Row");

        QAction* chosen = menu.exec(table->viewport()->mapToGlobal(pos));
        if(!chosen) return;

        QString textToCopy;
        if(chosen == copyChar){
            textToCopy = table->item(row,0)->text();
        } else if(chosen == copyDec){
            textToCopy = table->item(row,1)->text();
        } else if(chosen == copyHex){
            textToCopy = table->item(row,2)->text();
        } else if(chosen == copyRow){
            textToCopy = "| " + table->item(row,0)->text() + " | " + table->item(row,1)->text() + " | " + table->item(row,2)->text() + " |";
        }

        if(!textToCopy.isEmpty()){
            QGuiApplication::clipboard()->setText(textToCopy);
        }
    });
}
