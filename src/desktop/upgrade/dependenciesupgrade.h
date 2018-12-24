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
    QByteArray getData(const QString& u);
    void extractZIP(const QString& zip);
    bool checkDate();
    void markDate();
};

#endif // DEPENDENCIESUPGRADE_H
