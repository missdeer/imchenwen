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

void Config::read(const QString &key, Tuple3List &res)
{
    int size = settings().beginReadArray(key);
    for (int i = 0; i < size; ++i)
    {
        settings().setArrayIndex(i);
        res.push_back(std::make_tuple(settings().value("e1").toString(),
                                      settings().value("e2").toString(),
                                      settings().value("e3").toString()));
    }
    settings().endArray();
}

void Config::read(const QString &key, Tuple4List &res)
{
    int size = settings().beginReadArray(key);
    for (int i = 0; i < size; ++i)
    {
        settings().setArrayIndex(i);
        res.push_back(std::make_tuple(settings().value("e1").toString(),
                                      settings().value("e2").toString(),
                                      settings().value("e3").toString(),
                                      settings().value("e4").toString()));
    }
    settings().endArray();
}

void Config::read(const QString &key, Tuple5List &res)
{
    int size = settings().beginReadArray(key);
    for (int i = 0; i < size; ++i)
    {
        settings().setArrayIndex(i);
        res.push_back(std::make_tuple(settings().value("e1").toString(),
                                      settings().value("e2").toString(),
                                      settings().value("e3").toString(),
                                      settings().value("e4").toString(),
                                      settings().value("e5").toString()));
    }
    settings().endArray();
}

void Config::read(const QString &key, PlayerList &players)
{
    int size = settings().beginReadArray(key);
    for (int i = 0; i < size; ++i)
    {
        settings().setArrayIndex(i);
        PlayerPtr p(new Player(Player::PT_EXTERNAL, settings().value("e1").toString()));
        p->setArguments(settings().value("e2").toString());
        players.append(p);
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

void Config::write(const QString &key, const Tuple3List &array)
{
    settings().remove(key);
    settings().beginWriteArray(key);
    for (int i = 0; i < array.size(); ++i)
    {
        settings().setArrayIndex(i);
        settings().setValue("e1", std::get<0>(array.at(i)));
        settings().setValue("e2", std::get<1>(array.at(i)));
        settings().setValue("e3", std::get<2>(array.at(i)));
    }
    settings().endArray();
    settings().sync();
}

void Config::write(const QString &key, const Tuple4List &array)
{
    settings().remove(key);
    settings().beginWriteArray(key);
    for (int i = 0; i < array.size(); ++i)
    {
        settings().setArrayIndex(i);
        settings().setValue("e1", std::get<0>(array.at(i)));
        settings().setValue("e2", std::get<1>(array.at(i)));
        settings().setValue("e3", std::get<2>(array.at(i)));
        settings().setValue("e4", std::get<3>(array.at(i)));
    }
    settings().endArray();
    settings().sync();
}

void Config::write(const QString &key, const Tuple5List &array)
{
    settings().remove(key);
    settings().beginWriteArray(key);
    for (int i = 0; i < array.size(); ++i)
    {
        settings().setArrayIndex(i);
        settings().setValue("e1", std::get<0>(array.at(i)));
        settings().setValue("e2", std::get<1>(array.at(i)));
        settings().setValue("e3", std::get<2>(array.at(i)));
        settings().setValue("e4", std::get<3>(array.at(i)));
        settings().setValue("e5", std::get<4>(array.at(i)));
    }
    settings().endArray();
    settings().sync();
}

void Config::write(const QString &key, const PlayerList &players)
{
    settings().remove(key);
    settings().beginWriteArray(key);
    for (int i = 0; i < players.size(); ++i)
    {
        settings().setArrayIndex(i);
        settings().setValue("e1", players[i]->name());
        settings().setValue("e2", players[i]->arguments());
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
quint16 Config::read<quint16>(const QString& key)
{
    return static_cast<quint16>( settings().value(key, QVariant(0)).toUInt());
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

template<>
quint16 Config::read<quint16>(const QString& key, quint16 defaultValue)
{
    return static_cast<quint16>(settings().value(key, QVariant(defaultValue)).toUInt());
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
