#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QIcon>
#include <QWebEngineView>

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

class WebPage;
class WaitingSpinnerWidget;

class WebView : public QWebEngineView
{
    Q_OBJECT

public:
    WebView(QWidget *parent = nullptr);
    ~WebView() override;
    void setPage(WebPage *page);

    int loadProgress() const;
    bool isWebActionEnabled(QWebEnginePage::WebAction webAction) const;
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;

signals:
    void webActionEnabledChanged(QWebEnginePage::WebAction webAction, bool enabled);

private slots:
private:
    void createWebActionTrigger(QWebEnginePage *page, QWebEnginePage::WebAction);
private:
    int m_loadProgress;
    QString m_rightClickedUrl;
};

#endif
