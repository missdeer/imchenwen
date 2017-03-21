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

void Config::beginGroup(const QString& group)
{
    settings().beginGroup(group);
}

void Config::endGroup()
{
    settings().endGroup();
}

QSettings&Config::settings()
{
    static QSettings s(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.imchenwenrc",
                              QSettings::IniFormat);
    return s;
}

template<>
QString Config::read<QString>(const QString& key)
{
    return settings().value(key).toString();
}

template<>
bool Config::read<bool>(const QString &key)
{
    return settings().value(key, QVariant(false)).toBool();
}

template<>
int Config::read<int>(const QString& key)
{
    return settings().value(key, QVariant(0)).toInt();
}

template<>
bool Config::read<bool>(const QString& key, bool defaultValue)
{
    return settings().value(key, QVariant(defaultValue)).toBool();
}

template<>
int Config::read<int>(const QString& key, int defaultValue)
{
    return settings().value(key, QVariant(defaultValue)).toInt();
}

QString Config::read(const QString& key, const QString& defaultValue)
{
    return settings().value(key, defaultValue).toString();
}

template<>
QVariant Config::read<QVariant>(const QString& key)
{
    return settings().value(key);
}

QVariant Config::read(const QString& key, const QVariant& defaultValue)
{
    return settings().value(key, defaultValue);
}
