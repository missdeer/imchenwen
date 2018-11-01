#ifndef BROWSER_H
#define BROWSER_H

#include <QObject>
#include <QVector>
#include <QProcess>
#include "linkresolver.h"
#include "websites.h"
#include "urlrequestinterceptor.h"
#include "Kast.h"
#include "mpvwindow.h"

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

class BrowserWindow;
class WaitingSpinnerWidget;

class Browser : public QObject
{
    enum WindowState
    {
        isMaximized,
        isMinimized,
        isNormal,
        isHidden,
    };

    Q_OBJECT
public:
    static Browser &instance();
    ~Browser();

    QVector<BrowserWindow*> windows();
    void addWindow(BrowserWindow *window);
    BrowserWindow *mainWindow();
    BrowserWindow *newMainWindow();
    Kast *kast();

    void loadSettings();
    void resolveAndPlayByMediaPlayer(const QString& u);
    void doPlayByMediaPlayer(const QString& u, const QString& title);
    Websites &websites();
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
    LinkResolver m_linkResolver;
    Websites m_websites;
    UrlRequestInterceptor m_urlRequestInterceptor;
    QNetworkAccessManager *m_nam;
    Kast m_kast;
    MPVWindow* m_mpv;

    explicit Browser(QObject *parent = nullptr);
    void resolveLink(const QString &u);
    void doPlayByMediaPlayer(MediaInfoPtr mi);
    void playByBuiltinPlayer(MediaInfoPtr mi);
    void playByBuiltinPlayer(const QString& u, const QString& title);
    void clean();
    void waiting(bool disableParent = true);
    void clearAtExit();
    void minimizeWindows();
    void stopWaiting();
};
#endif // BROWSER_H
