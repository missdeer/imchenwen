#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QMainWindow>
#include <QWebEnginePage>

QT_BEGIN_NAMESPACE
class QProgressBar;
class QToolBar;
QT_END_NAMESPACE

class TabWidget;
class UrlLineEdit;
class WebView;
class PopupMenuToolButton;

class BrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    BrowserWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
    ~BrowserWindow() override;
    QSize sizeHint() const override;
    TabWidget *tabWidget() const;
    WebView *currentTab() const;

    void loadPage(const QString &url);
    void loadPage(const QUrl &url);
    void loadHomePage();
    void recoverCurrentTabUrl();
    void resetCurrentView();
    QString maybeVIPVideoTitle() const;
    void center();
protected:
    void closeEvent(QCloseEvent *event) override;
public slots:
    void runScriptOnOpenViews(const QString &source);
    void onCloseCurrentTab();
private slots:
    void onNewWindow();
    void onFileOpen();
    void onPlayURL();
    void onShowWindow();
    void onWebViewLoadProgress(int);
    void onWebViewTitleChanged(const QString &title);
    void onWebViewUrlChanged(const QUrl &url);
    void onWebViewIconChanged(const QIcon &icon);
    void onWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);

    void onShortcut();
    void onLiveTV();
    void onSettings();
    void onPlayByExternalMediaPlayer();
    void onPlayVIPByExternalMediaPlayer();
private:
    QMenu *createFileMenu(TabWidget *tabWidget);
    QMenu *createViewMenu(QToolBar *toolBar);
    QMenu *createWindowMenu(TabWidget *tabWidget);
    QMenu *createHelpMenu();
    QMenu *createShortcutMenu();
    QToolBar *createToolBar();
    void createLiveTVToolButton(QToolBar *navigationBar = nullptr);
private:
    TabWidget *m_tabWidget;
    QProgressBar *m_progressBar;
    QAction *m_historyBackAction;
    QAction *m_historyForwardAction;
    QAction *m_stopAction;
    QAction *m_reloadAction;
    QAction *m_stopReloadAction;
    UrlLineEdit *m_urlLineEdit;
    PopupMenuToolButton *m_liveTVAction;
    QToolBar *m_toolbar;
    QMenu *m_shortcutMenu;
    QMenu *m_chinaMenu;
    QMenu *m_abroadMenu;
    QMenu *m_onlineFilmMenu;
    QString m_maybeVIPVideoTitle;
};

#endif // BROWSERWINDOW_H
