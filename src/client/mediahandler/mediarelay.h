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
    QString transcoding(const QString &media);
    void processM3U8(const QString& media, const QByteArray &userAgent, const QByteArray &referrer);
    void stop();
    PlayerPtr player() const;
    void setPlayer(const PlayerPtr &player);

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
    void onReadStandardOutput();
    void onRedirected(const QUrl &url);
private:
    PlayerPtr m_player;
    QString m_title;
    QProcess m_ffmpegProcess;
    QByteArray m_socketData;
};

#endif // MEDIARELAY_H
