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
    BrowserWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
    ~BrowserWindow();
    QSize sizeHint() const override;
    TabWidget *tabWidget() const;
    WebView *currentTab() const;

    void loadPage(const QString &url);
    void loadPage(const QUrl &url);
    void loadHomePage();
protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void handleNewWindowTriggered();
    void handleFileOpenTriggered();
    void handleShowWindowTriggered();
    void handleWebViewLoadProgress(int);
    void handleWebViewTitleChanged(const QString &title);
    void handleWebViewUrlChanged(const QUrl &url);
    void handleWebViewIconChanged(const QIcon &icon);
    void handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);

    void handleShortcutTriggered();
    void handleVIPVideoTriggered();
    void handleLiveTVTriggered();
    void handleOptionsTriggered();
private:
    QMenu *createFileMenu(TabWidget *tabWidget);
    QMenu *createViewMenu(QToolBar *toolBar);
    QMenu *createWindowMenu(TabWidget *tabWidget);
    QMenu *createHelpMenu();
    QMenu *createShortcutMenu();
    QToolBar *createToolBar();

    QString findURL(const QString& area, const QString& name);
private:
    TabWidget *m_tabWidget;
    QProgressBar *m_progressBar;
    QAction *m_historyBackAction;
    QAction *m_historyForwardAction;
    QAction *m_stopAction;
    QAction *m_reloadAction;
    QAction *m_stopReloadAction;
    UrlLineEdit *m_urlLineEdit;
    PopupMenuToolButton* m_vipVideoAction;
    PopupMenuToolButton* m_liveTVAction;
    QToolBar *m_toolbar;
    void createVIPVideoToolButton(QToolBar *navigationBar = nullptr);
    void createLiveTVToolButton(QToolBar *navigationBar = nullptr);
};

#endif // BROWSERWINDOW_H
