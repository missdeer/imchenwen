#ifndef KAST_H
#define KAST_H

#include "DLNARenderer.h"
#include "DLNAPlaybackInfo.h"


class Kast : public QObject
{
    Q_OBJECT
public:
    Kast(QObject *parent = nullptr);
    void addItemToQueue(const QString & item_url);
    QStringList getRenderers();
    DLNARenderer *renderer(const QString& name);
signals:

private slots:
    void onFoundRenderer(DLNARenderer*);
    void onHttpResponse(const QString &, const QString &);
private:
    QMap<QString, DLNARenderer *> m_renderers;
};
#endif // KAST_H
