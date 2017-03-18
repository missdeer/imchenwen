#include "config.h"

Config::Config()
{

}

Config::~Config()
{
}

void Config::read(const QString& key, QString& value)
{
    value = settings().value(key).toString();
}

void Config::read(const QString& key, QStringList& res)
{
    int size = settings().beginReadArray(key);
    for (int i = 0; i < size; ++i)
    {
        settings().setArrayIndex(i);
        res.push_back(settings().value("value").toString());;
    }
    settings().endArray();
}

void Config::write(const QString& key, const QStringList& array)
{
    settings().beginWriteArray(key);
    for (int i = 0; i < array.size(); ++i)
    {
        settings().setArrayIndex(i);
        settings().setValue("value", array.at(i));
    }
    settings().endArray();
    settings().sync();
}

QSettings&Config::settings()
{
    static QSettings s(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.imchenwenrc",
                              QSettings::IniFormat);
    return s;
}
