#ifndef BROWSER_H
#define BROWSER_H

#include <QObject>
#include <QVector>
#include <QProcess>
#include <qhttpengine/server.h>
#include "linkresolver.h"
#include "websites.h"
#include "urlrequestinterceptor.h"
#include "Kast.h"
#include "inmemoryhandler.h"
#include "player.h"
#include "subscriptionhelper.h"

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

class BrowserWindow;
class WaitingSpinnerWidget;
class PlayerView;
class DLNAPlayerView;

class Browser : public QObject
{
    enum WindowState
    {
        isMaximized,
        isMinimized,
        isNormal,
        isHidden,
    };
    friend class BrowserWindow;

    Q_OBJECT
public:
    static Browser &instance();
    ~Browser();

    QVector<BrowserWindow*> windows();
    void addWindow(BrowserWindow *window);
    BrowserWindow *mainWindow();
    BrowserWindow *newMainWindow();
    Kast &kast();

    void loadSettings();
    void resolveAndPlayByMediaPlayer(const QString& u);
    void play(const QString& u, const QString& title);
signals:

private slots:
    void onClipboardChanged();
    void onResolved(MediaInfoPtr mi);
    void onResolvingError(const QString&);
    void onProcessError(QProcess::ProcessError error);
    void onPlayerFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onSniffedMediaUrl(const QString& u);
private:
    QVector<BrowserWindow*> m_windows;
    QMap<BrowserWindow*, WindowState> m_windowsState;
    WaitingSpinnerWidget *m_waitingSpinner;
    QProcess m_playerProcess;
    QProcess m_ffmpegProcess;
    LinkResolver m_linkResolver;
    Websites m_websites;
    UrlRequestInterceptor m_urlRequestInterceptor;
    QNetworkAccessManager *m_nam;
    Kast m_kast;
    PlayerView *m_builtinPlayer;
    DLNAPlayerView *m_dlnaPlayer;
    QHttpEngine::Server m_httpServer;
    InMemoryHandler m_httpHandler;
    SubscriptionHelper m_liveTVHelper;
    SubscriptionHelper m_vipVideoHelper;

    explicit Browser(QObject *parent = nullptr);
    void resolveLink(const QString &u);
    void play(MediaInfoPtr mi);
    void doPlay(PlayerPtr player, QStringList& urls, const QString& title, const QString& referrer);
    void playByBuiltinPlayer(const QString &url, const QString& title, const QString &referrer);
    void playByExternalPlayer(PlayerPtr player, const QString &url, const QString& title, const QString &referrer);
    void playByDLNARenderer(PlayerPtr player, const QString &url, const QString& title, const QString &referrer);
    void clean();
    void waiting(bool disableParent = true);
    void clearAtExit();
    void minimizeWindows();
    void stopWaiting();
};
#endif // BROWSER_H
