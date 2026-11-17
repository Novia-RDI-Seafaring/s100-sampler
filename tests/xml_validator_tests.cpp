#include "XMLValidator.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <iostream>

static bool writeFile(const QString &path, const QByteArray &content)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(content) == content.size();
}

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    QTemporaryDir temporary;
    if (!temporary.isValid())
        return 1;
    const QString schema = QDir(temporary.path()).filePath("schema with spaces.xsd");
    const QString valid = QDir(temporary.path()).filePath("valid message.xml");
    const QString invalid = QDir(temporary.path()).filePath("invalid message.xml");
    if (!writeFile(schema, "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
                           "<xs:element name=\"message\" type=\"xs:string\"/></xs:schema>") ||
        !writeFile(valid, "<message>synthetic test</message>") ||
        !writeFile(invalid, "<unexpected/>"))
        return 1;
    // The bundled executable must work even when no xmllint is on PATH.
    qputenv("PATH", QByteArray());
    QString error;
    if (!XMLValidator::validate(valid, schema, &error))
    {
        std::cerr << "Bundled xmllint rejected valid XML: " << error.toStdString() << '\n';
        return 1;
    }
    if (XMLValidator::validate(invalid, schema, &error) || error.isEmpty())
        return 1;
    if (XMLValidator::validate(valid, schema + ".missing", &error))
        return 1;
    return 0;
}
