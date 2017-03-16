#include <QSettings>
#include <QStandardPaths>
#include "config.h"

Config::Config()
{

}

Config::~Config()
{
}

QString Config::read(const QString &key)
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QSettings settings(configPath + "/.imchenwenrc", QSettings::IniFormat);
    return settings.value(key).toString();
}

void Config::write(const QString &key, const QString &value)
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QSettings settings(configPath + "/.imchenwenrc", QSettings::IniFormat);
    settings.setValue(key, value);
    settings.sync();
}
