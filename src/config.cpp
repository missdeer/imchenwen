#include <QSettings>
#include "config.h"

Config::Config()
{
    QString configPath;
    QSettings settings(configPath, QSettings::IniFormat);
}

void Config::load()
{

}

void Config::save()
{

}

QString Config::read(const QString &key)
{

}

void Config::write(const QString &key, const QString &value)
{

}
