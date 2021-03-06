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

bool WebView::isWebActionEnabled(QWebEnginePage::WebAction webAction) const
{
    return page()->action(webAction)->isEnabled();
}

QWebEngineView  *WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    switch (type) {
    case QWebEnginePage::WebBrowserTab: {
        auto *mainWindow = qobject_cast<BrowserWindow*>(window());
        return mainWindow->tabWidget()->onCreateTab();
    }
    case QWebEnginePage::WebBrowserBackgroundTab: {
        auto *mainWindow = qobject_cast<BrowserWindow*>(window());
        return mainWindow->tabWidget()->onCreateTab(false);
    }
    case QWebEnginePage::WebBrowserWindow: {
        auto *mainWindow = new BrowserWindow();
        Browser::instance().addWindow(mainWindow);
        return mainWindow->currentTab();
    }
    case QWebEnginePage::WebDialog: {
        auto *popup = new WebPopupWindow(page()->profile());
        return popup->view();
    }
    }
    return nullptr;
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = page()->createStandardContextMenu();
    const QList<QAction*> actions = menu->actions();
    auto it = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::CopyLinkToClipboard));
    if (it != actions.cend())
    {
        m_rightClickedUrl = page()->contextMenuData().linkUrl().toString();
        if (m_rightClickedUrl.startsWith("http://") || m_rightClickedUrl.startsWith("https://")  )
        {
            auto separator = menu->insertSeparator(*actions.cbegin());

            QAction *resolveUrlAsVIPAction = new QAction(QIcon(QStringLiteral(":playvip.png")), tr("Resolve Url As VIP"), this);
            menu->insertAction(separator, resolveUrlAsVIPAction);
            connect(resolveUrlAsVIPAction, &QAction::triggered, [this]() {
                Browser::instance().resolveUrlAsVIP(m_rightClickedUrl);
            });

            QAction *resolveUrlAction = new QAction(QIcon(QStringLiteral(":play.png")), tr("Resolve Url"), this);
            menu->insertAction(resolveUrlAsVIPAction, resolveUrlAction);
            connect(resolveUrlAction, &QAction::triggered, [this]() {
                Browser::instance().resolveUrl(m_rightClickedUrl);
            });

            QAction *openAction = menu->addAction(tr("Open URL in Default Web Browser"));
            connect(openAction, &QAction::triggered, [this](){
                QDesktopServices::openUrl(m_rightClickedUrl);
            });
        }
    }
    else
    {
        m_rightClickedUrl.clear();
    }

    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
    menu->popup(event->globalPos());
}

