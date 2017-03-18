#include <QSettings>
#include <QStandardPaths>
#include "config.h"

Config::Config()
{

}

Config::~Config()
{
}

QString Config::readItem(const QString &key)
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QSettings settings(configPath + "/.imchenwenrc", QSettings::IniFormat);
    return settings.value(key).toString();
}

void Config::writeItem(const QString &key, const QString &value)
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QSettings settings(configPath + "/.imchenwenrc", QSettings::IniFormat);
    settings.setValue(key, value);
    settings.sync();
}

QStringList Config::readArray(const QString& key)
{
    QStringList res;
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QSettings settings(configPath + "/.imchenwenrc", QSettings::IniFormat);
    int size = settings.beginReadArray(key);
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        res.push_back(settings.value("value").toString());;
    }
    settings.endArray();
    return res;
}

void Config::writeArray(const QString& key, const QStringList& array)
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QSettings settings(configPath + "/.imchenwenrc", QSettings::IniFormat);
    settings.beginWriteArray(key);
    for (int i = 0; i < array.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("value", array.at(i));
    }
    settings.endArray();
    settings.sync();
}
