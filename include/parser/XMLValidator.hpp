#pragma once

#include <QString>

namespace XMLValidator
{
    // Uses xmllint next to the executable, then PATH. No build-machine path is embedded.
    bool validate(const QString &xmlFile, const QString &schemaFile, QString *error = nullptr);
}
