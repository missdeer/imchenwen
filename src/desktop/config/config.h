#ifndef CONFIG_H
#define CONFIG_H

#include <tuple>
#include <QString>
#include <QList>
#include <QSharedPointer>
#include <QSettings>
#include <QStandardPaths>
#include "player.h"

using Tuple2     = std::tuple<QString, QString>;
using Tuple2Ptr  = QSharedPointer<Tuple2>;
using Tuple2List = QList<Tuple2>;
using Tuple3     = std::tuple<QString, QString, QString>;
using Tuple3Ptr  = QSharedPointer<Tuple3>;
using Tuple3List = QList<Tuple3>;
using Tuple4     = std::tuple<QString, QString, QString, QString>;
using Tuple4Ptr  = QSharedPointer<Tuple4>;
using Tuple4List = QList<Tuple4>;
using Tuple5     = std::tuple<QString, QString, QString, QString, QString>;
using Tuple5Ptr  = QSharedPointer<Tuple5>;
using Tuple5List = QList<Tuple5>;

class Config
{
public:
    Config()  = default;
    ~Config() = default;

    template<typename T>
    T read(const QString& key);

    template<typename T>
    T read(const QString& key, T defaultValue);

    QString read(const QString& key, const QString& defaultValue);
    QVariant read(const QString& key, const QVariant& defaultValue);

    template<typename T>
    void write(const QString& key, const T& value)
    {
        settings().setValue(key, value);
        settings().sync();
    }

    void read(const QString& key, QString& value);
    void read(const QString& key, QStringList& array);
    void read(const QString& key, Tuple2List& array);
    void read(const QString& key, Tuple3List& array);
    void read(const QString& key, Tuple4List& array);
    void read(const QString& key, Tuple5List& array);
    void read(const QString& key, PlayerList& players);
    void write(const QString& key, const QStringList& array);
    void write(const QString& key, const Tuple2List& array);
    void write(const QString& key, const Tuple3List& array);
    void write(const QString& key, const Tuple4List& array);
    void write(const QString& key, const Tuple5List& array);
    void write(const QString& key, const PlayerList& players);

    void beginGroup(const QString& group);
    void endGroup();
private:
    QSettings& settings();
};

template<> QString Config::read<QString>(const QString& key);
template<> QStringList Config::read<QStringList>(const QString& key);
template<> QByteArray Config::read<QByteArray>(const QString& key);
template<> QVariant Config::read<QVariant>(const QString& key);
template<> bool Config::read<bool>(const QString& key);
template<> bool Config::read<bool>(const QString& key, bool defaultValue);
template<> int Config::read<int>(const QString& key);
template<> int Config::read<int>(const QString& key, int defaultValue);
template<> quint16 Config::read<quint16>(const QString& key);
template<> quint16 Config::read<quint16>(const QString& key, quint16 defaultValue);

#endif // CONFIG_H
