#include "XMLValidator.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

bool XMLValidator::validate(const QString &xmlFile, const QString &schemaFile, QString *error)
{
#ifdef Q_OS_WIN
    const QString programName = QStringLiteral("xmllint.exe");
#else
    const QString programName = QStringLiteral("xmllint");
#endif
    QString program = QDir(QCoreApplication::applicationDirPath()).filePath(programName);
    if (!QFileInfo(program).isFile())
        program = QStandardPaths::findExecutable(programName);
    if (program.isEmpty())
    {
        if (error)
            *error = QStringLiteral("xmllint is missing; install it beside the application or on PATH.");
        return false;
    }

    QProcess validator;
    validator.start(program, {QStringLiteral("--nonet"), QStringLiteral("--noout"),
                              QStringLiteral("--schema"), schemaFile, xmlFile});
    if (!validator.waitForStarted(5000))
    {
        if (error)
            *error = validator.errorString();
        return false;
    }
    if (!validator.waitForFinished(30000))
    {
        validator.kill();
        validator.waitForFinished();
        if (error)
            *error = QStringLiteral("xmllint did not finish within 30 seconds.");
        return false;
    }
    const bool valid = validator.exitStatus() == QProcess::NormalExit && validator.exitCode() == 0;
    if (error)
        *error = valid ? QString() : QString::fromLocal8Bit(validator.readAllStandardError());
    return valid;
}
