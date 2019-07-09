#include "browser.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include "urllineedit.h"
#include "settings.h"
#include "websites.h"
#include "subscriptionhelper.h"
#include "config.h"
#include "popupmenutoolbutton.h"
#include "inputurldialog.h"
#include "donatedialog.h"
#include "dependenciesupgrade.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QTimer>
#include <QInputDialog>
#include <QDesktopServices>

BrowserWindow::BrowserWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , m_tabWidget(new TabWidget(this))
    , m_progressBar(new QProgressBar(this))
    , m_historyBackAction(nullptr)
    , m_historyForwardAction(nullptr)
    , m_stopAction(nullptr)
    , m_reloadAction(nullptr)
    , m_stopReloadAction(nullptr)
    , m_urlLineEdit(new UrlLineEdit(this))
    , m_liveTVAction(nullptr)
    , m_shortcutMenu(nullptr)
    , m_chinaMenu(nullptr)
    , m_abroadMenu(nullptr)
    , m_onlineFilmMenu(nullptr)
{
    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setAttribute(Qt::WA_DeleteOnClose, true);

    m_toolbar = createToolBar();
    addToolBar(m_toolbar);
    menuBar()->addMenu(createFileMenu(m_tabWidget));
    menuBar()->addMenu(createViewMenu(m_toolbar));
    menuBar()->addMenu(createShortcutMenu());
    menuBar()->addMenu(createWindowMenu(m_tabWidget));
    menuBar()->addMenu(createHelpMenu());

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    addToolBarBreak();

    m_progressBar->setMaximumHeight(1);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(QStringLiteral("QProgressBar {border: 0px } QProgressBar::chunk { background-color: red; }"));

    layout->addWidget(m_progressBar);
    layout->addWidget(m_tabWidget);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    connect(m_tabWidget, &TabWidget::titleChanged, this, &BrowserWindow::onWebViewTitleChanged);
    connect(m_tabWidget, &TabWidget::linkHovered, [this](const QString& url) {
        statusBar()->showMessage(url);
    });
    connect(m_tabWidget, &TabWidget::loadProgress, this, &BrowserWindow::onWebViewLoadProgress);
    connect(m_tabWidget, &TabWidget::urlChanged, this, &BrowserWindow::onWebViewUrlChanged);
    connect(m_tabWidget, &TabWidget::iconChanged, this, &BrowserWindow::onWebViewIconChanged);
#if defined(Q_OS_MAC)
#else
    connect(m_tabWidget, &TabWidget::webActionEnabledChanged, this, &BrowserWindow::onWebActionEnabledChanged);
#endif
    connect(m_urlLineEdit, &QLineEdit::returnPressed, [this]() {
        m_urlLineEdit->setFavIcon(QIcon(QStringLiteral(":defaulticon.png")));
        loadPage(m_urlLineEdit->url());
    });
    connect(&Browser::instance().m_liveTVHelper, &SubscriptionHelper::ready, [this](){
        createLiveTVToolButton(m_toolbar);
    });
    connect(&Browser::instance().m_websites, &Websites::ready, [this](){
        createShortcutMenu();
    });

    m_urlLineEdit->setFavIcon(QIcon(QStringLiteral(":defaulticon.png")));

    onWebViewTitleChanged(tr("imchenwen"));
    m_tabWidget->onCreateTab();
}

QSize BrowserWindow::sizeHint() const
{
    QRect desktopRect = QApplication::desktop()->screenGeometry();
    QSize size = desktopRect.size() * qreal(0.9);
    return size;
}

QMenu *BrowserWindow::createFileMenu(TabWidget *tabWidget)
{
    QMenu *fileMenu = new QMenu(tr("&File"));
    fileMenu->addAction(tr("&New Window"), this, &BrowserWindow::onNewWindow, QKeySequence::New);

    QAction *newTabAction = new QAction(QIcon(QLatin1String(":addtab.png")), tr("New &Tab"), this);
    newTabAction->setShortcuts(QKeySequence::AddTab);
    newTabAction->setIconVisibleInMenu(false);
    connect(newTabAction, &QAction::triggered, tabWidget, &TabWidget::onCreateTab);
    fileMenu->addAction(newTabAction);

    QAction *playUrlAction = fileMenu->addAction(tr("Play Url..."));
    playUrlAction->setShortcut(QKeySequence("Ctrl+Shift+P"));
    connect(playUrlAction, &QAction::triggered, this, &BrowserWindow::onPlayURL);

    QAction *resolveUrlAction = fileMenu->addAction(tr("Resolve Url"));
    resolveUrlAction->setShortcut(QKeySequence("Ctrl+P"));
    connect(resolveUrlAction, &QAction::triggered, this, &BrowserWindow::onResolveUrl);

    QAction *resolveUrlAsVIPAction = fileMenu->addAction(tr("Resolve Url As VIP"));
    resolveUrlAsVIPAction->setShortcut(QKeySequence("Ctrl+Shift+V"));
    connect(resolveUrlAsVIPAction, &QAction::triggered, this, &BrowserWindow::onResolveUrlAsVIP);

    QAction *settingsAction = fileMenu->addAction(tr("Preferences..."));
    settingsAction->setMenuRole(QAction::PreferencesRole);
    connect(settingsAction, &QAction::triggered, this, &BrowserWindow::onSettings);

    fileMenu->addSeparator();

    QAction *closeTabAction = new QAction(QIcon(QLatin1String(":closetab.png")), tr("&Close Tab"), this);
    closeTabAction->setShortcuts(QKeySequence::Close);
    closeTabAction->setIconVisibleInMenu(false);
    connect(closeTabAction, &QAction::triggered, this, &BrowserWindow::onCloseCurrentTab);
    fileMenu->addAction(closeTabAction);


    QAction *closeAction = new QAction(tr("&Quit"),this);
    closeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    connect(closeAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(closeAction);

    connect(fileMenu, &QMenu::aboutToShow, [closeAction]() {
        if (Browser::instance().windows().count() == 1)
            closeAction->setText(tr("&Quit"));
        else
            closeAction->setText(tr("&Close Window"));
    });
    return fileMenu;
}

QMenu *BrowserWindow::createViewMenu(QToolBar *toolbar)
{
    QMenu *viewMenu = new QMenu(tr("&View"));
    m_stopAction = viewMenu->addAction(tr("&Stop"));
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Period));
    shortcuts.append(Qt::Key_Escape);
    m_stopAction->setShortcuts(shortcuts);

    m_reloadAction = viewMenu->addAction(tr("Reload Page"));
    m_reloadAction->setShortcuts(QKeySequence::Refresh);
    
#if defined(Q_OS_MAC)
#else
    connect(m_stopAction, &QAction::triggered, [this]() {
        m_tabWidget->onTriggerWebPageAction(QWebEnginePage::Stop);
    });
    connect(m_reloadAction, &QAction::triggered, [this]() {
        m_tabWidget->onTriggerWebPageAction(QWebEnginePage::Reload);
    });
#endif

    QAction *zoomIn = viewMenu->addAction(tr("Zoom &In"));
    zoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    connect(zoomIn, &QAction::triggered, [this]() {
        if (currentTab())
            currentTab()->setZoomFactor(currentTab()->zoomFactor() + 0.1);
    });

    QAction *zoomOut = viewMenu->addAction(tr("Zoom &Out"));
    zoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    connect(zoomOut, &QAction::triggered, [this]() {
        if (currentTab())
            currentTab()->setZoomFactor(currentTab()->zoomFactor() - 0.1);
    });

    QAction *resetZoom = viewMenu->addAction(tr("Reset &Zoom"));
    resetZoom->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(resetZoom, &QAction::triggered, [this]() {
        if (currentTab())
            currentTab()->setZoomFactor(1.0);
    });


    viewMenu->addSeparator();
    QAction *viewToolbarAction = new QAction(tr("Hide Toolbar"),this);
    viewToolbarAction->setShortcut(tr("Ctrl+|"));
    connect(viewToolbarAction, &QAction::triggered, [toolbar,viewToolbarAction]() {
        if (toolbar->isVisible()) {
            viewToolbarAction->setText(tr("Show Toolbar"));
            toolbar->close();
        } else {
            viewToolbarAction->setText(tr("Hide Toolbar"));
            toolbar->show();
        }
    });
    viewMenu->addAction(viewToolbarAction);

    QAction *viewStatusbarAction = new QAction(tr("Hide Status Bar"), this);
    viewStatusbarAction->setShortcut(tr("Ctrl+/"));
    connect(viewStatusbarAction, &QAction::triggered, [this, viewStatusbarAction]() {
        if (statusBar()->isVisible()) {
            viewStatusbarAction->setText(tr("Show Status Bar"));
            statusBar()->close();
        } else {
            viewStatusbarAction->setText(tr("Hide Status Bar"));
            statusBar()->show();
        }
    });
    viewMenu->addAction(viewStatusbarAction);
    return viewMenu;
}

QMenu *BrowserWindow::createWindowMenu(TabWidget *tabWidget)
{
    QMenu *menu = new QMenu(tr("&Window"));

    QAction *nextTabAction = new QAction(tr("Show Next Tab"), this);
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceRight));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Less));
    nextTabAction->setShortcuts(shortcuts);
    connect(nextTabAction, &QAction::triggered, tabWidget, &TabWidget::onNextTab);

    QAction *previousTabAction = new QAction(tr("Show Previous Tab"), this);
    shortcuts.clear();
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceLeft));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Greater));
    previousTabAction->setShortcuts(shortcuts);
    connect(previousTabAction, &QAction::triggered, tabWidget, &TabWidget::onPreviousTab);

    connect(menu, &QMenu::aboutToShow, [this, menu, nextTabAction, previousTabAction]() {
        menu->clear();
        menu->addAction(nextTabAction);
        menu->addAction(previousTabAction);
        menu->addSeparator();

        QVector<BrowserWindow*> windows = Browser::instance().windows();
        int index(-1);
        for (auto window : windows) {
            QAction *action = menu->addAction(window->windowTitle(), this, &BrowserWindow::onShowWindow);
            action->setData(++index);
            action->setCheckable(true);
            if (window == this)
                action->setChecked(true);
        }
    });
    return menu;
}

QMenu *BrowserWindow::createHelpMenu()
{
    QMenu *helpMenu = new QMenu(tr("&Help"));
    connect(helpMenu->addAction(tr("Visit Homepage...")), &QAction::triggered, [](){
        QDesktopServices::openUrl(QUrl("https://minidump.info/imchenwen/"));
    });
    connect(helpMenu->addAction(tr("Donate...")), &QAction::triggered, [&](){
        DonateDialog dlg(this);
        dlg.exec();
    });
    helpMenu->addSeparator();

    connect(helpMenu->addAction(tr("Install Adobe Flash Player")), &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl("https://get.adobe.com/flashplayer/otherversions"));
    });
    connect(helpMenu->addAction(tr("Install mpv")), &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl("https://mpv.io"));
    });
    connect(helpMenu->addAction(tr("Upgrade Resolvers in background")), &QAction::triggered, [](){
        QThreadPool::globalInstance()->start(new DependenciesUpgrade);
    });
    helpMenu->addSeparator();

    QAction *aboutAction = helpMenu->addAction(tr("About..."));
    aboutAction->setMenuRole(QAction::AboutRole);
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this,
                           tr("About imchenwen"),
                           tr("Parse video URLs and invoke external player to play the videos, so that you don't need web browsers such as IE, Chrome, Firefox, Opera, Safari etc any more. It is designed for poor performance machines/OSs.\n\nBuild at ") + QLatin1String( __DATE__ ));
    });

    QAction *aboutQtAction = helpMenu->addAction(tr("About Qt..."));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    connect(aboutQtAction, &QAction::triggered, [this](){QMessageBox::aboutQt(this, tr("imchenwen"));});

    return helpMenu;
}

QMenu *BrowserWindow::createShortcutMenu()
{
    if (!m_shortcutMenu)
        m_shortcutMenu = new QMenu(tr("&Shortcut"));

    struct {
        WebsiteList websites;
        QMenu*& menu;
        QString title;
    } menuTitleMap[] = {
        { Browser::instance().m_websites.inChina(),    m_chinaMenu,      tr("In China")},
        { Browser::instance().m_websites.abroad(),     m_abroadMenu,     tr("Out of China")},
        { Browser::instance().m_websites.onlineFilm(), m_onlineFilmMenu, tr("Online Film")},
    };
    for (auto & m : menuTitleMap)
    {
        if (!m.menu)
        {
            m.menu = new QMenu(m.title);
            m_shortcutMenu->addMenu(m.menu);
        }
        QAction *inChinaAction = m_shortcutMenu->actions()[0];
        auto actions = m.menu->actions();
        for (auto w : m.websites)
        {
            auto it = std::find_if(actions.begin(), actions.end(), [&](QAction* a){
                return a->text() == w->title;
            });
            if (actions.end() == it)
            {
                QAction *action = m.menu->addAction(w->title);
                action->setData(w->url);
                connect(action, &QAction::triggered, this, &BrowserWindow::onShortcut);
                if (w->favourite)
                    m_shortcutMenu->insertAction(inChinaAction, action);
            }
        }
    }

    QAction *inChinaAction = m_shortcutMenu->actions()[0];
    m_shortcutMenu->insertSeparator(inChinaAction);

    return m_shortcutMenu;
}

void BrowserWindow::createLiveTVToolButton(QToolBar *navigationBar)
{
    auto content = Browser::instance().m_liveTVHelper.content();
    if (content.isEmpty())
        return;
    auto titles = content.keys();
    auto *popupMenu = new QMenu(this);
    for (const auto & title : titles)
    {
        auto subscriptions = content.values(title);
        if (!subscriptions.isEmpty())
        {
            auto *subMenu = new QMenu(title, popupMenu);
            popupMenu->addMenu(subMenu);
            for (const auto & subscription : subscriptions)
            {
                for (const auto& sub : *subscription)
                {
                    const auto& [name, tip] = sub;
                    auto *action  = new QAction(name, subMenu);
                    action->setData(tip);
                    action->setToolTip(tip);
                    action->setStatusTip(tip);
                    connect(action, &QAction::triggered, this, &BrowserWindow::onLiveTV);
                    subMenu->addAction(action);
                }
            }
        }
    }

    if (!m_liveTVAction)
    {
        m_liveTVAction = new PopupMenuToolButton(this);
        m_liveTVAction->setIcon(QIcon(QStringLiteral(":playtv.png")));
        m_liveTVAction->setText(tr("Watch Live TV"));
        navigationBar->addWidget(m_liveTVAction);
    } else {
        QMenu *p = m_liveTVAction->menu();
        p->deleteLater();
    }
    m_liveTVAction->setMenu(popupMenu);
}

QString BrowserWindow::maybeVIPVideoTitle() const
{
    if (!m_maybeVIPVideoTitle.isEmpty())
        return m_maybeVIPVideoTitle;
    return m_tabWidget->currentWebView()->title();
}

void BrowserWindow::center()
{
    QRect available = QApplication::desktop()->availableGeometry(this);
    move((available.width() - width())/2,
         (available.height() - height())/2);
}

QToolBar *BrowserWindow::createToolBar()
{
    QToolBar *navigationBar = new QToolBar(tr("Navigation"));
    navigationBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    navigationBar->toggleViewAction()->setEnabled(false);

    m_historyBackAction = new QAction(this);
    QList<QKeySequence> backShortcuts = QKeySequence::keyBindings(QKeySequence::Back);
    for (auto it = backShortcuts.begin(); it != backShortcuts.end();) {
        // Chromium already handles navigate on backspace when appropriate.
        if ((*it)[0] == Qt::Key_Backspace)
            it = backShortcuts.erase(it);
        else
            ++it;
    }
    // For some reason Qt doesn't bind the dedicated Back key to Back.
    backShortcuts.append(QKeySequence(Qt::Key_Back));
    m_historyBackAction->setShortcuts(backShortcuts);
    m_historyBackAction->setIconVisibleInMenu(false);
    m_historyBackAction->setIcon(QIcon(QStringLiteral(":go-previous.png")));
    connect(m_historyBackAction, &QAction::triggered, [this]() {
#if defined(Q_OS_MAC)
        auto* view = m_tabWidget->currentWebView();
        view->back();
#else
        m_tabWidget->onTriggerWebPageAction(QWebEnginePage::Back);
#endif
    });
    navigationBar->addAction(m_historyBackAction);

    m_historyForwardAction = new QAction(this);
    QList<QKeySequence> fwdShortcuts = QKeySequence::keyBindings(QKeySequence::Forward);
    for (auto it = fwdShortcuts.begin(); it != fwdShortcuts.end();) {
        if (((*it)[0] & Qt::Key_unknown) == Qt::Key_Backspace)
            it = fwdShortcuts.erase(it);
        else
            ++it;
    }
    fwdShortcuts.append(QKeySequence(Qt::Key_Forward));
    m_historyForwardAction->setShortcuts(fwdShortcuts);
    m_historyForwardAction->setIconVisibleInMenu(false);
    m_historyForwardAction->setIcon(QIcon(QStringLiteral(":go-next.png")));
    connect(m_historyForwardAction, &QAction::triggered, [this]() {
#if defined(Q_OS_MAC)
        auto* view = m_tabWidget->currentWebView();
        view->forward();
#else
        m_tabWidget->onTriggerWebPageAction(QWebEnginePage::Forward);
#endif
    });
    navigationBar->addAction(m_historyForwardAction);

    m_stopReloadAction = new QAction(this);
    connect(m_stopReloadAction, &QAction::triggered, [this]() {
#if defined(Q_OS_MAC)
        auto* view = m_tabWidget->currentWebView();
        view->stop();
#else
        m_tabWidget->onTriggerWebPageAction(QWebEnginePage::WebAction(m_stopReloadAction->data().toInt()));
#endif
    });
    navigationBar->addAction(m_stopReloadAction);
    navigationBar->addWidget(m_urlLineEdit);
    navigationBar->setIconSize(QSize(32, 32));

    QAction *resolveUrlAction = new QAction(QIcon(QStringLiteral(":play.png")), tr("Resolve Url"), this);
    resolveUrlAction->setToolTip(resolveUrlAction->text());
    connect(resolveUrlAction, &QAction::triggered, this, &BrowserWindow::onResolveUrl);
    navigationBar->addAction(resolveUrlAction);

    QAction *resolveUrlAsVIPAction = new QAction(QIcon(QStringLiteral(":playvip.png")), tr("Resolve Url As VIP"), this);
    resolveUrlAsVIPAction->setToolTip(resolveUrlAsVIPAction->text());
    connect(resolveUrlAsVIPAction, &QAction::triggered, this, &BrowserWindow::onResolveUrlAsVIP);
    navigationBar->addAction(resolveUrlAsVIPAction);

    createLiveTVToolButton(navigationBar);

    return navigationBar;
}

void BrowserWindow::onWebViewIconChanged(const QIcon &icon)
{
    m_urlLineEdit->setFavIcon(icon);
}

void BrowserWindow::onWebViewUrlChanged(const QUrl &url)
{
    m_urlLineEdit->setUrl(url);
    if (url.isEmpty())
        m_urlLineEdit->setFocus();
}

#if defined (Q_OS_MAC)

#else
void BrowserWindow::onWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled)
{
    switch (action) {
    case QWebEnginePage::Back:
        m_historyBackAction->setEnabled(enabled);
        break;
    case QWebEnginePage::Forward:
        m_historyForwardAction->setEnabled(enabled);
        break;
    case QWebEnginePage::Reload:
        m_reloadAction->setEnabled(enabled);
        break;
    case QWebEnginePage::Stop:
        m_stopAction->setEnabled(enabled);
        break;
    default:
        qWarning("Unhandled webActionChanged singal");
    }
}
#endif

void BrowserWindow::onShortcut()
{
    auto *action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    auto url = action->data().toString();
    if (!url.isEmpty())
    {
        if (m_tabWidget->currentWebView()->url().url().isEmpty())
            loadPage(url);
        else
            m_tabWidget->navigateInNewTab(url);
    }
}

void BrowserWindow::onLiveTV()
{
    auto *action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    auto url = action->data().toString();
    Browser::instance().play(url, QStringList() << url, tr("Live TV - ") + action->text());
}

void BrowserWindow::onSettings()
{
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        Browser::instance().m_liveTVHelper.update();
        Browser::instance().m_websites.update();
        Browser::instance().m_vipResolver.update();
        Browser::instance().m_linkResolver.setupResolvers();
    }
}

void BrowserWindow::onResolveUrl()
{
    if (!m_tabWidget)
        return;
    if (!m_tabWidget->currentWebView())
        return;
    QUrl u = m_tabWidget->currentWebView()->url();
    if (!u.isValid())
        return;
    Browser::instance().resolveUrl(u.toString());
}

void BrowserWindow::onResolveUrlAsVIP()
{
    if (!m_tabWidget)
        return;
    if (!m_tabWidget->currentWebView())
        return;
    QUrl u = m_tabWidget->currentWebView()->url();
    if (!u.isValid())
        return;
    Browser::instance().resolveUrlAsVIP(u.toString());
}

void BrowserWindow::onWebViewTitleChanged(const QString &title)
{
    if (title.isEmpty())
        setWindowTitle(tr("imchenwen"));
    else
        setWindowTitle(tr("%1 - imchenwen").arg(title));
}

void BrowserWindow::onNewWindow()
{
    auto *window = new BrowserWindow();
    Browser::instance().addWindow(window);
    window->loadHomePage();
}

void BrowserWindow::onFileOpen()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open Web Resource"), QString(),
                                                tr("Web Resources (*.html *.htm *.svg *.png *.gif *.svgz);;All files (*.*)"));
    if (file.isEmpty())
        return;
    loadPage(file);
}

void BrowserWindow::onPlayURL()
{
    InputUrlDialog dlg(this);
    if (dlg.exec())
    {
        if (dlg.resolveThenPlay())
        {
            Browser::instance().resolveUrl(dlg.url());
        }
        else if (dlg.resolveAsVIPThenPlay())
        {
            Browser::instance().resolveUrlAsVIP(dlg.url());
        }
        else if (dlg.sniffThenPlay())
        {
            m_tabWidget->navigateInNewTab(dlg.url());
        }
        else if (dlg.playDirectly())
        {
            Browser::instance().play(dlg.url(), QStringList() << dlg.url(), tr("Play URL directly"));
        }
    }
}

void BrowserWindow::closeEvent(QCloseEvent *event)
{
    if (m_tabWidget->count() > 1) {
        int ret = QMessageBox::warning(this, tr("Confirm close"),
                                       tr("Are you sure you want to close the window ?\n"
                                          "There are %1 tabs open.").arg(m_tabWidget->count()),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::No) {
            event->ignore();
            return;
        }
    }
    m_tabWidget->closeAllTabs();
    event->accept();
    deleteLater();
}

void BrowserWindow::runScriptOnOpenViews(const QString& /*source*/)
{

}

void BrowserWindow::onCloseCurrentTab()
{
    m_tabWidget->onCloseTab(m_tabWidget->currentIndex());
}

void BrowserWindow::loadHomePage()
{
    Config cfg;
    loadPage(cfg.read<QString>("defaultHome"));
}

void BrowserWindow::recoverCurrentTabUrl()
{
    QFile f(":/vipplaying.html");
    if (f.open(QIODevice::ReadOnly))
    {
        QByteArray data = f.readAll();
        f.close();
        QString u = currentTab()->url().toString();

        Config cfg;
        Tuple2List vipVideos;
        cfg.read("vipVideo", vipVideos);

        auto it = std::find_if(vipVideos.begin(), vipVideos.end(), [&u](const Tuple2& vv){
            return u.startsWith(std::get<1>(vv));
        });
        if (vipVideos.end() != it)
        {
            u = u.right(u.length() - std::get<1>(*it).length());
        }

        QString c = QString(data).arg(u);
        auto v = m_tabWidget->currentWebView();
        v->setHtml(c, QUrl("qrc:///vipplaying.html"));
    }
}

void BrowserWindow::resetCurrentView()
{
    auto v = m_tabWidget->currentWebView();
    if (v->url().url() == "qrc:///vipplaying.html")
    {
        v->back();
        v->back();
    }
}

void BrowserWindow::loadPage(const QString &page)
{
    loadPage(QUrl::fromUserInput(page));
}

void BrowserWindow::loadPage(const QUrl &url)
{
    if (url.isValid()) {
        m_urlLineEdit->setUrl(url);
        m_tabWidget->onSetUrl(url);
    }
}

TabWidget *BrowserWindow::tabWidget() const
{
    return m_tabWidget;
}

ImWebView *BrowserWindow::currentTab() const
{
    return m_tabWidget->currentWebView();
}

void BrowserWindow::onWebViewLoadProgress(int progress)
{
    static QIcon stopIcon(QStringLiteral(":process-stop.png"));
    static QIcon reloadIcon(QStringLiteral(":view-refresh.png"));

    if (progress < 100 && progress > 0) {
#if defined (Q_OS_MAC)
#else
        m_stopReloadAction->setData(QWebEnginePage::Stop);
#endif
        m_stopReloadAction->setIcon(stopIcon);
        m_stopReloadAction->setToolTip(tr("Stop loading the current page"));
    } else {
#if defined (Q_OS_MAC)
#else
        m_stopReloadAction->setData(QWebEnginePage::Reload);
#endif
        m_stopReloadAction->setIcon(reloadIcon);
        m_stopReloadAction->setToolTip(tr("Reload the current page"));
    }
    m_progressBar->setValue(progress < 100 ? progress : 0);
}

void BrowserWindow::onShowWindow()
{
    if (auto *action = qobject_cast<QAction*>(sender()); action) {
        int offset = action->data().toInt();
        QVector<BrowserWindow*> windows = Browser::instance().windows();
        windows.at(offset)->activateWindow();
        windows.at(offset)->currentTab()->setFocus();
    }
}
