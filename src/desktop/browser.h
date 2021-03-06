#ifndef BROWSER_H
#define BROWSER_H

#include <QObject>
#include <QVector>
#include <QProcess>
#include <QNetworkAccessManager>
#include <qhttpengine/server.h>
#include "sniffer.h"
#include "linkresolver.h"
#include "vipresolver.h"
#include "websites.h"
#include "Kast.h"
#include "inmemoryhandler.h"
#include "player.h"
#include "subscriptionhelper.h"
#include "mediarelay.h"
#include "outofchinamainlandproxyfactory.h"
#include "ingfwlistproxyfactory.h"
#include "storageservice.h"

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
    void resolveUrl(const QString &u);
    void resolveUrlAsVIP(const QString &u);
    void play(const QString& originalUrl, const QStringList &results, const QString &title);
    void                            init(bool withoutUI = false);
    OutOfChinaMainlandProxyFactory *outOfChinaMainlandProxyFactory() const;
    InGFWListProxyFactory *inGFWListProxyFactory() const;

signals:

private slots:
    void onClipboardChanged();
    void onNormalLinkResolved(const QString &originalUrl, MediaInfoPtr results);
    void onNormalLinkResolvingError(const QString&, const QString&);
    void onVIPLinkResolved(const QString &originalUrl, const QStringList &results);
    void onVIPLinkResolvingError();
    void onSnifferDone(const QString &originalUrl, const QString& result);
    void onSnifferError();
    void onProcessError(QProcess::ProcessError error);
    void onPlayerFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onNewM3U8Ready();
    void onTranscodingFailed();
private:
    QNetworkAccessManager m_nam;
    Kast m_kast;
    QVector<BrowserWindow*> m_windows;
    QMap<BrowserWindow*, WindowState> m_windowsState;
    WaitingSpinnerWidget *m_waitingSpinner;
    QProcess m_playerProcess;
    LinkResolver m_linkResolver;
    VIPResolver m_vipResolver;
    Sniffer m_sniffer;
    Websites m_websites;
    PlayerView *m_builtinPlayer;
    DLNAPlayerView *m_dlnaPlayer;
    QHttpEngine::Server m_httpServer;
    InMemoryHandler m_httpHandler;
    SubscriptionHelper m_liveTVHelper;
    MediaRelay m_mediaRelay;
    PlayDialog *m_playDialog;
    OutOfChinaMainlandProxyFactory *m_outOfChinaMainlandProxyFactory;
    InGFWListProxyFactory *m_inGFWListProxyFactory;
    StorageService m_storageService;
    bool                              m_withoutUI {false};

    explicit Browser(QObject *parent = nullptr);
    void resolveLink(const QString &u);
    void resolveVIPLink(const QString &u);
    void play(const QString& originalUrl, MediaInfoPtr mi);
    void doPlay(PlayerPtr player, QStringList &videoUrls, const QString &audioUrl, const QString &subtitleUrl, const QString& title, const QString& referrer);
    void playByBuiltinPlayer(const QString &videoUrl, const QString &audioUrl, const QString &subtitle, const QString& title, const QString &referrer);
    void playByExternalPlayer(PlayerPtr player, const QString &videoUrl, const QString &audioUrl, const QString &subtitle, const QString& title, const QString &referrer);
    void playByDLNARenderer(PlayerPtr player, const QString &url, const QString& title, const QString &referrer);
    void clean();
    void waiting(bool disableParent = true);
    void clearAtExit();
    void minimizeWindows();
    void stopWaiting();
    void playerStopped();
};
#endif // BROWSER_H
