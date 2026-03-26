#ifndef DISASSEMBLERWORKER_H
#define DISASSEMBLERWORKER_H

#include <QObject>
#include <QString>
#include <QVector>

struct DisasmInstruction {
    QString address;
    QString bytes;
    QString mnemonic;
    QString operands;
    
    // Добавляем поля для совместимости с radare2backend
    quint64 size = 0;
    quint64 fileOffset = 0;
};

struct DisasmSection {
    QString name;
    QVector<DisasmInstruction> instructions;
    
    // Добавляем поля для совместимости с radare2backend
    quint64 vaddr = 0;
    quint64 fileOffset = 0;
    quint64 size = 0;
    bool hasFileMapping = false;
};

struct DisasmFunction {
    QString name;
    QString address; 
};

struct DisasmString {
    QString address; 
    QString value;   
};

class DisassemblerWorker : public QObject
{
    Q_OBJECT
public:
    explicit DisassemblerWorker(QObject *parent = nullptr);
public slots:
    void disassemble(const QString &filePath, const QString &arch);
    void cancel();
signals:
    void sectionFound(const DisasmSection &section);
    void functionsFound(const QVector<DisasmFunction> &funcs);
    void stringsFound(const QVector<DisasmString> &strings);
    void finished();
    void errorOccurred(const QString &errorMsg);
    void progressUpdated(int percent);
    void logLine(const QString &line);
private:
    bool m_cancelled = false;
    QVector<DisasmSection> parseSections(const QByteArray &output);
    static QString detectArch(const QString &filePath);
};

#endif // DISASSEMBLERWORKER_H