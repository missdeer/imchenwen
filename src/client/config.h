#ifndef CONFIG_H
#define CONFIG_H

#include <tuple>
#include <QString>
#include <QList>
#include <QSharedPointer>
#include <QSettings>
#include <QStandardPaths>

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
    void write(const QString& key, const T& value)
    {
        settings().setValue(key, value);
        settings().sync();
    }

    void read(const QString& key, QString& value);
    void read(const QString& key, QStringList& array);
    void read(const QString& key, Tuple2List& array);
    void write(const QString& key, const QStringList& array);
    void write(const QString& key, const Tuple2List& array);
    void write(const QString& key, const Tuple3List& array);
    void write(const QString& key, const Tuple4List& array);
    void write(const QString& key, const Tuple5List& array);
private:
    QSettings& settings();
};

template<> QString Config::read<QString>(const QString& key);

#endif // CONFIG_H
