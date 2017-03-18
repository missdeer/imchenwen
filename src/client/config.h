#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

class Config
{
public:
    Config();
    ~Config();

    QString readItem(const QString& key);
    void writeItem(const QString& key, const QString& value);
    QStringList readArray(const QString& key);
    void writeArray(const QString& key, const QStringList& array);
};

#endif // CONFIG_H
