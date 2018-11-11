#ifndef MEDIARELAY_H
#define MEDIARELAY_H

#include <QObject>
#include <QProcess>
#include <QNetworkReply>
#include <QSslError>
#include "playdialog.h"

class MediaRelay : public QObject
{
    Q_OBJECT
public:
    explicit MediaRelay(QObject *parent = nullptr);

    QString makeM3U8(PlayerPtr player, QStringList& urls);
    QString transcoding(const QString& ext, const QString &media);
    void processM3U8(const QString& media, const QByteArray &userAgent, const QByteArray &referrer);
    void stop();
    PlayerPtr player() const;
    void setPlayer(const PlayerPtr &player);

    const QString &ext() const;
    void setExt(const QString &ext);

    const QString &title() const;
    void setTitle(const QString &title);

signals:
    void inputEnd();
    void newM3U8Ready();
private slots:
    void onNetworkError(QNetworkReply::NetworkError code);
    void onNetworkSSLErrors(const QList<QSslError> &errors);
    void onReadyRead();
    void onMediaReadFinished();
private:
    PlayerPtr m_player;
    QString m_ext;
    QString m_title;
    QProcess m_ffmpegProcess;
    QByteArray m_data;
};

#endif // MEDIARELAY_H
