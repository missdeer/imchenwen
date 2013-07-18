#ifndef CONFIG_H
#define CONFIG_H

class Config
{
public:
    Config();

    void load();
    void save();
    QString read(const QString& key);
    void write(const QString& key, const QString& value);
};

#endif // CONFIG_H
