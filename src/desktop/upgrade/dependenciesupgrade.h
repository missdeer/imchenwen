#ifndef DEPENDENCIESUPGRADE_H
#define DEPENDENCIESUPGRADE_H

#include <QRunnable>

class DependenciesUpgrade : public QRunnable
{
public:
    DependenciesUpgrade();

    virtual void run();
private:
    void upgradeForWin();
    void upgradeForMac();
};

#endif // DEPENDENCIESUPGRADE_H
