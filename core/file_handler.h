#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <QString>

class FileHandler
{
public:
    // Читает .txt и .md как обычный текст
    static QString readTextFile(const QString &path);

    // Определяет по расширению, поддерживается ли файл
    static bool isSupported(const QString &path);

    // Возвращает короткое описание: "txt", "md", иначе ""
    static QString extensionOf(const QString &path);
};

#endif