#ifndef DEPENDENCIESUPGRADE_H
#define DEPENDENCIESUPGRADE_H

#include <QRunnable>
#include <QNetworkAccessManager>

class DependenciesUpgrade : public QRunnable
{
public:
    DependenciesUpgrade();

    virtual void run();
private:
    QNetworkAccessManager nam;
    void upgradeForWin();
    void upgradeForMac();
    void getFile(const QString& u, const QString& saveToFile);
    void getData(const QString& u, QByteArray &data);
    void extractZIP(const QString& zip);
};

#endif // DEPENDENCIESUPGRADE_H
