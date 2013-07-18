#include <QSettings>
#if QT_VERSION < 050000
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif
#include "config.h"

Config::Config()
{

}

Config::~Config()
{
}

QString Config::read(const QString &key)
{
#if QT_VERSION < 050000
    QString configPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#endif
    QSettings settings(configPath + "/.imchenwenrc", QSettings::IniFormat);
    return settings.value(key).toString();
}

void Config::write(const QString &key, const QString &value)
{
#if QT_VERSION < 050000
    QString configPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#endif
    QSettings settings(configPath + "/.imchenwenrc", QSettings::IniFormat);
    settings.setValue(key, value);
    settings.sync();
}
