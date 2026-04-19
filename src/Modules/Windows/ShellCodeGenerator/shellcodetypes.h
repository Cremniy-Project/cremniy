#ifndef SHELLCODETYPES_H
#define SHELLCODETYPES_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

enum class ShellcodeOutputStyle {
    C,
    Cpp,
    Raw
};

struct ShellcodeRequest {
    QString assembly;
    int bitness = 64;
};

struct ShellcodeDisasmEntry {
    int offset = 0;
    int size = 0;
    QString mnemonic;
};

struct ShellcodeDependencyStatus {
    QString assemblerPath;
    QString disassemblerPath;
    QStringList missingRequired;
    QStringList missingOptional;

    bool canAssemble() const { return missingRequired.isEmpty(); }
    bool hasWarnings() const { return !missingOptional.isEmpty(); }
};

struct ShellcodeResult {
    bool success = false;
    QByteArray rawBytes;
    QList<ShellcodeDisasmEntry> disassembly;
    QString message;
    bool warning = false;
    ShellcodeDependencyStatus dependencies;
};

#endif // SHELLCODETYPES_H
