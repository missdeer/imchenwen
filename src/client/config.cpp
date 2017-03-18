#include "config.h"

Config::Config()
{

}

Config::~Config()
{
}

template<>
QString Config::read<QString>(const QString& key)
{
    return settings().value(key).toString();
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

void Config::read(const QString& key, Tuple2List& res)
{
    int size = settings().beginReadArray(key);
    for (int i = 0; i < size; ++i)
    {
        settings().setArrayIndex(i);
        res.push_back(std::make_tuple(settings().value("e1").toString(),
                                      settings().value("e2").toString()));
    }
    settings().endArray();
}

void Config::write(const QString& key, const QStringList& array)
{
    settings().remove(key);
    settings().beginWriteArray(key);
    for (int i = 0; i < array.size(); ++i)
    {
        settings().setArrayIndex(i);
        settings().setValue("value", array.at(i));
    }
    settings().endArray();
    settings().sync();
}

void Config::write(const QString& key, const Tuple2List& array)
{
    settings().remove(key);
    settings().beginWriteArray(key);
    for (int i = 0; i < array.size(); ++i)
    {
        settings().setArrayIndex(i);
        settings().setValue("e1", std::get<0>(array.at(i)));
        settings().setValue("e2", std::get<1>(array.at(i)));
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
