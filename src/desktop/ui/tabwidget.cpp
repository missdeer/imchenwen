#include "tabwidget.h"
#include <QMenu>
#include <QTabBar>
#include <QtCore>

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    QTabBar *tabBar = this->tabBar();
    tabBar->setTabsClosable(true);
    tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    tabBar->setMovable(true);
    tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar, &QTabBar::customContextMenuRequested, this, &TabWidget::onContextMenuRequested);
    connect(tabBar, &QTabBar::tabCloseRequested, this, &TabWidget::onCloseTab);
    connect(tabBar, &QTabBar::tabBarDoubleClicked, [this](int index) {
        if (index != -1)
            return;
        onCreateTab();
    });

    setDocumentMode(true);
    setElideMode(Qt::ElideRight);

    connect(this, &QTabWidget::currentChanged, this, &TabWidget::onCurrentChanged);
}

void TabWidget::onCurrentChanged(int index)
{
    if (index != -1) {
        ImWebView *view = webView(index);
        if (!view->url().isEmpty())
            view->setFocus();
        emit titleChanged(view->title());
        emit loadProgress(view->loadProgress());
        emit urlChanged(view->url());
#if defined (Q_OS_MAC)
#else
        QIcon pageIcon = view->page()->icon();
        if (!pageIcon.isNull())
            emit iconChanged(pageIcon);
        else
            emit iconChanged(QIcon(QStringLiteral(":defaulticon.png")));
        emit webActionEnabledChanged(QWebEnginePage::Back, view->isWebActionEnabled(QWebEnginePage::Back));
        emit webActionEnabledChanged(QWebEnginePage::Forward, view->isWebActionEnabled(QWebEnginePage::Forward));
        emit webActionEnabledChanged(QWebEnginePage::Stop, view->isWebActionEnabled(QWebEnginePage::Stop));
        emit webActionEnabledChanged(QWebEnginePage::Reload,view->isWebActionEnabled(QWebEnginePage::Reload));
#endif
    } else {
        emit titleChanged(QString());
        emit loadProgress(0);
        emit urlChanged(QUrl());
        emit iconChanged(QIcon(QStringLiteral(":defaulticon.png")));
#if defined (Q_OS_MAC)
#else
        emit webActionEnabledChanged(QWebEnginePage::Back, false);
        emit webActionEnabledChanged(QWebEnginePage::Forward, false);
        emit webActionEnabledChanged(QWebEnginePage::Stop, false);
        emit webActionEnabledChanged(QWebEnginePage::Reload, true);
#endif
    }
}

void TabWidget::onContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    menu.addAction(tr("New &Tab"), this, &TabWidget::onCreateTab, QKeySequence::AddTab);
    int index = tabBar()->tabAt(pos);
    if (index != -1) {
        QAction *action = menu.addAction(tr("Clone Tab"));
        connect(action, &QAction::triggered, this, [this,index]() {
            onCloneTab(index);
        });
        menu.addSeparator();
        action = menu.addAction(tr("&Close Tab"));
        action->setShortcut(QKeySequence::Close);
        connect(action, &QAction::triggered, this, [this,index]() {
            onCloseTab(index);
        });
        action = menu.addAction(tr("Close &Other Tabs"));
        connect(action, &QAction::triggered, this, [this,index]() {
            onCloseOtherTabs(index);
        });
        menu.addSeparator();
        action = menu.addAction(tr("Reload Tab"));
        action->setShortcut(QKeySequence::Refresh);
        connect(action, &QAction::triggered, this, [this,index]() {
            onReloadTab(index);
        });
    } else {
        menu.addSeparator();
    }
    menu.addAction(tr("Reload All Tabs"), this, &TabWidget::onReloadAllTabs);
    menu.exec(QCursor::pos());
}

ImWebView *TabWidget::currentWebView()
{
    return webView(currentIndex());
}

ImWebView *TabWidget::navigateInNewTab(const QUrl &url, bool makeCurrent)
{
    auto v = onCreateTab(makeCurrent);
    v->setUrl(url);
    if (makeCurrent)
    {
        v->setFocus();
    }
    return v;
}

void TabWidget::closeAllTabs()
{
    while (count() > 0)
    {
        auto *view = webView(0);
        removeTab(0);
        delete view;
    }
    if (count() == 0)
        onCreateTab();
}

ImWebView *TabWidget::webView(int index)
{
    if (index < 0 || index >= count())
        return onCreateTab(true);
    return qobject_cast<ImWebView*>(widget(index));
}

void TabWidget::setupView(ImWebView *webView)
{
#if defined (Q_OS_MAC)
    connect(webView, &ImWebView::titleChanged, [this, webView](const QString &title) {
        int index = indexOf(webView);
        if (index != -1)
            setTabText(index, title);
        if (currentIndex() == index)
            emit titleChanged(title);
    });
    connect(webView, &ImWebView::loadFinish, [this, webView](const QString &strUrl, const QString &) {
        QUrl url = QUrl::fromEncoded(QByteArray::fromPercentEncoding(strUrl.toUtf8()));
        int index = indexOf(webView);
        if (index != -1)
            tabBar()->setTabData(index, url);
        if (currentIndex() == index)
            emit urlChanged(url);
    });
    connect(webView, &ImWebView::iconChanged, [this, webView](const QIcon &icon) {
        int index = indexOf(webView);
        QIcon ico = icon.isNull() ? QIcon(QStringLiteral(":defaulticon.png")) : icon;

        if (index != -1)
            setTabIcon(index, ico);
        if (currentIndex() == index)
            emit iconChanged(ico);
    });
    connect(webView, &ImWebView::newWindowsRequest, [this](const QString& strUrl) {
        navigateInNewTab(strUrl);
    });
#else
    QWebEnginePage *webPage = webView->page();

    connect(webView, &QWebEngineView::titleChanged, [this, webView](const QString &title) {
        int index = indexOf(webView);
        if (index != -1)
            setTabText(index, title);
        if (currentIndex() == index)
            emit titleChanged(title);
    });
    connect(webView, &QWebEngineView::urlChanged, [this, webView](const QUrl &url) {
        int index = indexOf(webView);
        if (index != -1)
            tabBar()->setTabData(index, url);
        if (currentIndex() == index)
            emit urlChanged(url);
    });
    connect(webView, &QWebEngineView::loadProgress, [this, webView](int progress) {
        if (currentIndex() == indexOf(webView))
            emit loadProgress(progress);
    });
    connect(webPage, &QWebEnginePage::linkHovered, [this, webView](const QString &url) {
        if (currentIndex() == indexOf(webView))
            emit linkHovered(url);
    });
    connect(webPage, &WebPage::iconChanged, [this, webView](const QIcon &icon) {
        int index = indexOf(webView);
        QIcon ico = icon.isNull() ? QIcon(QStringLiteral(":defaulticon.png")) : icon;

        if (index != -1)
            setTabIcon(index, ico);
        if (currentIndex() == index)
            emit iconChanged(ico);
    });
    connect(webView, &WebView::webActionEnabledChanged, [this, webView](QWebEnginePage::WebAction action, bool enabled) {
        if (currentIndex() ==  indexOf(webView))
            emit webActionEnabledChanged(action,enabled);
    });
    connect(webView, &QWebEngineView::loadStarted, [this, webView]() {
        int index = indexOf(webView);
        if (index != -1) {
            QIcon icon(QLatin1String(":view-refresh.png"));
            setTabIcon(index, icon);
        }
    });
    connect(webPage, &QWebEnginePage::windowCloseRequested, [this, webView]() {
        int index = indexOf(webView);
        if (index >= 0)
            onCloseTab(index);
    });
#endif
}

ImWebView *TabWidget::onCreateTab(bool makeCurrent)
{
    auto *webView = new ImWebView(this);
#if !defined (Q_OS_MAC)
    auto *webPage = new WebPage(QWebEngineProfile::defaultProfile(), webView);
    webView->setPage(webPage);
#endif
    setupView(webView);
    addTab(webView, tr("(Untitled)"));
    if (makeCurrent)
        setCurrentWidget(webView);
    return webView;
}

void TabWidget::onReloadAllTabs()
{
    for (int i = 0; i < count(); ++i)
        webView(i)->reload();
}

void TabWidget::onCloseOtherTabs(int index)
{
    for (int i = count() - 1; i > index; --i)
        onCloseTab(i);
    for (int i = index - 1; i >= 0; --i)
        onCloseTab(i);
}

void TabWidget::onCloseTab(int index)
{
    if (auto *view = webView(index); view)
    {
        auto hasFocus = view->hasFocus();
        removeTab(index);
        if (hasFocus && count() > 0)
            currentWebView()->setFocus();
        if (count() == 0)
            onCreateTab();
        view->deleteLater();
    }
}

void TabWidget::onCloneTab(int index)
{
    if (auto *view = webView(index); view) 
    {
        auto *tab = onCreateTab(false);
        tab->setUrl(view->url());
    }
}

void TabWidget::onSetUrl(const QUrl &url)
{
    if (auto *view = currentWebView(); view) 
    {
        view->setUrl(url);
        view->setFocus();
    }
}

#if defined (Q_OS_MAC)
#else
void TabWidget::onTriggerWebPageAction(QWebEnginePage::WebAction action)
{
    if (auto *view = currentWebView(); view) 
    {
        view->triggerPageAction(action);
        view->setFocus();
    }
}
#endif

void TabWidget::onNextTab()
{
    int next = currentIndex() + 1;
    if (next == count())
        next = 0;
    setCurrentIndex(next);
}

void TabWidget::onPreviousTab()
{
    int next = currentIndex() - 1;
    if (next < 0)
        next = count() - 1;
    setCurrentIndex(next);
}

void TabWidget::onReloadTab(int index)
{
    if (auto *view = webView(index); view)
        view->reload();
}
