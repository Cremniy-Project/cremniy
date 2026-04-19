#pragma once

#include <QByteArray>
#include <QList>
#include <QString>

class ShellcodeEngine {
public:
    enum class OutputStyle {
        C = 0,
        Cpp = 1,
        Raw = 2,
    };

    struct DisasmEntry {
        int     offset = 0;
        int     size = 0;
        QString mnemonic;
    };

    struct AssembleResult {
        bool       ok = false;
        QByteArray bytes;
        QString    error;
    };

    struct Dependency {
        QString name;
        bool    found = false;
    };

    [[nodiscard]] static AssembleResult assemble(const QString& asmSource, const QString& bits);
    [[nodiscard]] static QList<DisasmEntry> disassemble(const QByteArray& raw, const QString& bits);
    [[nodiscard]] static QString format(const QByteArray& raw, const QList<DisasmEntry>& entries, OutputStyle style, const QString& bits);

    [[nodiscard]] static QList<Dependency> checkDependencies();
    [[nodiscard]] static QString findTool(const QString& name);

private:
    [[nodiscard]] static QString generateC(const QByteArray& raw, const QList<DisasmEntry>& entries);
    [[nodiscard]] static QString generateCpp(const QByteArray& raw, const QList<DisasmEntry>& entries);
    [[nodiscard]] static QString generateRaw(const QByteArray& raw);
    [[nodiscard]] static QString formatAnnotated(const QByteArray& raw, const QList<DisasmEntry>& entries);
};