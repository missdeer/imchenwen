#ifndef BROWSER_H
#define BROWSER_H

#include <QObject>
#include <QVector>
#include <QProcess>
#include <QNetworkAccessManager>
#include <qhttpengine/server.h>
#include "linkresolver.h"
#include "vipresolver.h"
#include "websites.h"
#include "Kast.h"
#include "inmemoryhandler.h"
#include "player.h"
#include "subscriptionhelper.h"
#include "mediarelay.h"

class BrowserWindow;
class WaitingSpinnerWidget;
class PlayerView;
class DLNAPlayerView;
class PlayDialog;

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
    friend class MediaRelay;
    Q_OBJECT
public:
    static Browser &instance();
    ~Browser();

    QVector<BrowserWindow*> windows();
    void addWindow(BrowserWindow *window);
    BrowserWindow *mainWindow();
    BrowserWindow *newMainWindow();
    Kast &kast();
    Websites &shortcuts();
    QNetworkAccessManager &networkAccessManager();

    void loadSettings();
    void resolveAndPlayByMediaPlayer(const QString &u);
    void resolveVIPAndPlayByMediaPlayer(const QString &u);
    void play(const QStringList &url, const QString &title);
    void init();
signals:

private slots:
    void onClipboardChanged();
    void onNormalLinkResolved(MediaInfoPtr mi);
    void onNormalLinkResolvingError(const QString&);
    void onVIPLinkResolved(const QStringList &urls);
    void onVIPLinkResolvingError();
    void onProcessError(QProcess::ProcessError error);
    void onPlayerFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onNewM3U8Ready();
private:
    QNetworkAccessManager m_nam;
    Kast m_kast;
    QVector<BrowserWindow*> m_windows;
    QMap<BrowserWindow*, WindowState> m_windowsState;
    WaitingSpinnerWidget *m_waitingSpinner;
    QProcess m_playerProcess;
    LinkResolver m_linkResolver;
    VIPResolver m_vipResolver;
    Websites m_websites;
    PlayerView *m_builtinPlayer;
    DLNAPlayerView *m_dlnaPlayer;
    QHttpEngine::Server m_httpServer;
    InMemoryHandler m_httpHandler;
    SubscriptionHelper m_liveTVHelper;
    MediaRelay m_mediaRelay;
    PlayDialog *m_playDialog;

    explicit Browser(QObject *parent = nullptr);
    void resolveLink(const QString &u);
    void resolveVIPLink(const QString &u);
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
    void playerStopped();
};
#endif // BROWSER_H
