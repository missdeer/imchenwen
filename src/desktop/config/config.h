#ifndef CONFIG_H
#define CONFIG_H

#include <tuple>
#include <QString>
#include <QList>
#include <QSharedPointer>
#include <QSettings>
#include <QStandardPaths>
#include "player.h"

typedef std::tuple<QString, QString> Tuple2;
typedef QSharedPointer<Tuple2> Tuple2Ptr;
typedef QList<Tuple2> Tuple2List;
typedef std::tuple<QString, QString, QString> Tuple3;
typedef QSharedPointer<Tuple3> Tuple3Ptr;
typedef QList<Tuple3> Tuple3List;
typedef std::tuple<QString, QString, QString, QString> Tuple4;
typedef QSharedPointer<Tuple4> Tuple4Ptr;
typedef QList<Tuple4> Tuple4List;
typedef std::tuple<QString, QString, QString, QString, QString> Tuple5;
typedef QSharedPointer<Tuple5> Tuple5Ptr;
typedef QList<Tuple5> Tuple5List;

class Config
{
public:
    Config();
    ~Config();

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
