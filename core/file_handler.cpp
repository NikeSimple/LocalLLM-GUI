#include "file_handler.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

QString FileHandler::readTextFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return QString();
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    QString content = stream.readAll();
    file.close();
    return content;
}

QString FileHandler::extensionOf(const QString &path)
{
    return QFileInfo(path).suffix().toLower();
}

bool FileHandler::isSupported(const QString &path)
{
    QString ext = extensionOf(path);
    return ext == "txt" || ext == "md" || ext == "log" || ext == "json"
        || ext == "cpp" || ext == "h" || ext == "py" || ext == "js"
        || ext == "html" || ext == "css" || ext == "xml" || ext == "csv";
}