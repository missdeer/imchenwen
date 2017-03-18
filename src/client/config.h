#ifndef CONFIG_H
#define CONFIG_H

#include <tuple>
#include <QString>
#include <QList>
#include <QSettings>
#include <QStandardPaths>

typedef QList<std::tuple<QString, QString>> Tuple2List;
typedef QList<std::tuple<QString, QString, QString>> Tuple3List;
typedef QList<std::tuple<QString, QString, QString, QString>> Tuple4List;
typedef QList<std::tuple<QString, QString, QString, QString, QString>> Tuple5List;

class Config
{
public:
    Config();
    ~Config();

    template<typename T>
    T read(const QString& key);

    template<>
    QString read(const QString& key)
    {
        return settings().value(key).toString();
    }

    template<typename T>
    void write(const QString& key, const T& value)
    {
        settings().setValue(key, value);
        settings().sync();
    }

    void read(const QString& key, QString& value);
    void read(const QString& key, QStringList& array);
    void write(const QString& key, const QStringList& array);
    void write(const QString& key, const Tuple2List& array);
    void write(const QString& key, const Tuple3List& array);
    void write(const QString& key, const Tuple4List& array);
    void write(const QString& key, const Tuple5List& array);
private:
    QSettings& settings();
};

#endif // CONFIG_H
