#include "browser.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include "webpage.h"
#include "webpopupwindow.h"
#include "webview.h"
#include "waitingspinnerwidget.h"
#include "linkresolver.h"
#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QWebEngineContextMenuData>
#include <QDesktopServices>
#include <QNetworkAccessManager>

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)
    , m_loadProgress(0)
    , m_waitingSpinner(nullptr)
    , m_linkResolver(nullptr)
{
    connect(this, &QWebEngineView::loadProgress, [this](int progress) {
        m_loadProgress = progress;
    });
    connect(this, &QWebEngineView::loadFinished, [this](bool success) {
        if (!success) {
            m_loadProgress = 0;
        }
    });

    connect(this, &QWebEngineView::renderProcessTerminated,
            [this](QWebEnginePage::RenderProcessTerminationStatus termStatus, int statusCode) {
        QString status;
        switch (termStatus) {
        case QWebEnginePage::NormalTerminationStatus:
            status = tr("Render process normal exit");
            break;
        case QWebEnginePage::AbnormalTerminationStatus:
            status = tr("Render process abnormal exit");
            break;
        case QWebEnginePage::CrashedTerminationStatus:
            status = tr("Render process crashed");
            break;
        case QWebEnginePage::KilledTerminationStatus:
            status = tr("Render process killed");
            break;
        }
        QMessageBox::StandardButton btn = QMessageBox::question(window(), status,
                                                   tr("Render process exited with code: %1\n"
                                                      "Do you want to reload the page ?").arg(statusCode));
        if (btn == QMessageBox::Yes)
            QTimer::singleShot(0, [this] { reload(); });
    });
}

WebView::~WebView()
{
    if (m_waitingSpinner)
    {
        if (m_waitingSpinner->isSpinning())
            m_waitingSpinner->stop();
        delete m_waitingSpinner;
    }

    if (m_linkResolver)
    {
        disconnect(m_linkResolver, &LinkResolver::resolvingFinished, this, &WebView::resolvingFinished);
        disconnect(m_linkResolver, &LinkResolver::resolvingError, this, &WebView::resolvingError);
        delete m_linkResolver;
    }
}

void WebView::setPage(WebPage *page)
{
    createWebActionTrigger(page,QWebEnginePage::Forward);
    createWebActionTrigger(page,QWebEnginePage::Back);
    createWebActionTrigger(page,QWebEnginePage::Reload);
    createWebActionTrigger(page,QWebEnginePage::Stop);
    QWebEngineView::setPage(page);
}

int WebView::loadProgress() const
{
    return m_loadProgress;
}

void WebView::createWebActionTrigger(QWebEnginePage *page, QWebEnginePage::WebAction webAction)
{
    QAction *action = page->action(webAction);
    connect(action, &QAction::changed, [this, action, webAction]{
        emit webActionEnabledChanged(webAction, action->isEnabled());
    });
}

void WebView::resolveLink(const QUrl &u)
{
    if (!m_waitingSpinner)
    {
        m_waitingSpinner = new WaitingSpinnerWidget(this, Qt::ApplicationModal, true);

        m_waitingSpinner->setRoundness(70.0);
        m_waitingSpinner->setMinimumTrailOpacity(15.0);
        m_waitingSpinner->setTrailFadePercentage(70.0);
        m_waitingSpinner->setNumberOfLines(12);
        m_waitingSpinner->setLineLength(10);
        m_waitingSpinner->setLineWidth(5);
        m_waitingSpinner->setInnerRadius(10);
        m_waitingSpinner->setRevolutionsPerSecond(1);
        m_waitingSpinner->setColor(QColor(81, 4, 71));
    }
    if (m_waitingSpinner->isSpinning())
        m_waitingSpinner->stop();
    m_waitingSpinner->start();

    if (!m_linkResolver)
    {
        m_linkResolver = new LinkResolver(this);
        connect(m_linkResolver, &LinkResolver::resolvingFinished, this, &WebView::resolvingFinished);
        connect(m_linkResolver, &LinkResolver::resolvingError, this, &WebView::resolvingError);
    }
    m_linkResolver->resolve(u);
}

bool WebView::isWebActionEnabled(QWebEnginePage::WebAction webAction) const
{
    return page()->action(webAction)->isEnabled();
}

void WebView::playByBuiltinPlayer(const QUrl &u)
{
    qDebug() << "play " << u << " by built-in player";
    m_playByBuiltinPlayer = true;
    resolveLink(u);
}

void WebView::playByExternalPlayer(const QUrl &u)
{
    qDebug() << "play " << u << " by external player";
    m_playByBuiltinPlayer = false;
    resolveLink(u);
}

QWebEngineView *WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    switch (type) {
    case QWebEnginePage::WebBrowserTab: {
        BrowserWindow *mainWindow = qobject_cast<BrowserWindow*>(window());
        return mainWindow->tabWidget()->createTab();
    }
    case QWebEnginePage::WebBrowserBackgroundTab: {
        BrowserWindow *mainWindow = qobject_cast<BrowserWindow*>(window());
        return mainWindow->tabWidget()->createTab(false);
    }
    case QWebEnginePage::WebBrowserWindow: {
        BrowserWindow *mainWindow = new BrowserWindow();
        Browser::instance().addWindow(mainWindow);
        return mainWindow->currentTab();
    }
    case QWebEnginePage::WebDialog: {
        WebPopupWindow *popup = new WebPopupWindow(page()->profile());
        return popup->view();
    }
    }
    return nullptr;
}

void WebView::resolvingFinished(const MediaInfo &mediaInfo)
{
    if (m_waitingSpinner->isSpinning())
        m_waitingSpinner->stop();

}

void WebView::resolvingError()
{
    if (m_waitingSpinner->isSpinning())
        m_waitingSpinner->stop();

    QMessageBox::warning(this, tr("Error"), tr("Resolving link address failed!"), QMessageBox::Ok);
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = page()->createStandardContextMenu();
    const QList<QAction*> actions = menu->actions();
    auto it = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::OpenLinkInThisWindow));
    if (it != actions.cend())
    {
        m_rightClickedUrl = page()->contextMenuData().linkUrl();
        (*it)->setText(tr("Open Link in This Tab"));
        ++it;
        QAction *before(it == actions.cend() ? nullptr : *it);
        menu->insertAction(before, page()->action(QWebEnginePage::OpenLinkInNewTab));
        menu->addSeparator();
        QAction* playAction = menu->addAction(tr("Play Link by Built-in Player"));
        connect(playAction, &QAction::triggered, [this]() {
            playByBuiltinPlayer(m_rightClickedUrl);
        });
        playAction = menu->addAction(tr("Play Link by External Player"));
        connect(playAction, &QAction::triggered, [this]() {
            playByExternalPlayer(m_rightClickedUrl);
        });
        menu->addSeparator();
        QAction* openAction = menu->addAction(tr("Open URL in Default Web Browser"));
        connect(openAction, &QAction::triggered, [this](){
            QDesktopServices::openUrl(m_rightClickedUrl);
        });
    }
    else
    {
        m_rightClickedUrl = QUrl();
    }
    if (page()->contextMenuData().selectedText().isEmpty())
        menu->addAction(page()->action(QWebEnginePage::SavePage));
    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
    menu->popup(event->globalPos());
}

