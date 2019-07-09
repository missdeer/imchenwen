#include "qtcocoawebview.h"
#include <WebKit/WebKit.h>
#include <Webkit/WebView.h>
#include <QDebug>
#include "cocoawebview.h"


QtCocoaWebView::QtCocoaWebView(QWidget *parent) :
    QMacCocoaViewContainer(nullptr, parent)
{
    @autoreleasepool{

    NSRect rect = {{0, 0}, {(CGFloat)parent->width(), (CGFloat)parent->height()}};

    CocoaWebView *cWebView = [[CocoaWebView alloc]initWithObjects:rect frameName:nil groupName:nil target:this];
    
    [[cWebView mainFrame] loadRequest: [NSURLRequest requestWithURL:QUrl::fromUserInput("https://www.google.com").toNSURL()
                                        cachePolicy:NSURLRequestUseProtocolCachePolicy timeoutInterval:15]];
    
    setCocoaView(cWebView);

    [cWebView release];

    }
}

QtCocoaWebView::QtCocoaWebView(const QString& loadUrl, QWidget *parent) :
    QMacCocoaViewContainer(nullptr, parent)
{
    @autoreleasepool{

    NSRect rect = {{0, 0}, {(CGFloat)parent->width(), (CGFloat)parent->height()}};

    CocoaWebView *cWebView = [[CocoaWebView alloc]initWithObjects:rect frameName:nil groupName:nil target:this];

    [[cWebView mainFrame] loadRequest: [NSURLRequest requestWithURL:QUrl::fromUserInput(loadUrl).toNSURL()
                                        cachePolicy:NSURLRequestUseProtocolCachePolicy timeoutInterval:15]];

    setCocoaView(cWebView);

    [cWebView release];

    }

}

void QtCocoaWebView::loadRequest(const QString& loadUrl)
{
   @autoreleasepool{

    CocoaWebView *cWebView = (CocoaWebView*)this->cocoaView();
    NSString *strUrl = loadUrl.toNSString();

    [[cWebView mainFrame] loadRequest:
        [NSURLRequest requestWithURL:[NSURL URLWithString:[NSString stringWithString:strUrl]]]];

    }
}

bool QtCocoaWebView::canGoBack()
{
    return false;
}

bool QtCocoaWebView::canGoForward()
{
    return false;
}

void QtCocoaWebView::back()
{

}

void QtCocoaWebView::forward()
{
    
}

void QtCocoaWebView::reload()
{
    
}

void QtCocoaWebView::onTitleChanged(const QString &strTitle)
{
    m_title = strTitle;
    emit titleChanged(strTitle);
}

void QtCocoaWebView::onIconChanged(const QIcon &icon)
{
    m_icon = icon;
    emit iconChanged(icon);
}

void QtCocoaWebView::onNewWindowsRequest(const QString &strUrl)
{
    emit newWindowsRequest(strUrl);
}

void QtCocoaWebView::onStatusTextChanged(const QString &text)
{
    emit statusTextChanged(text);
}

QUrl QtCocoaWebView::url() const
{
    return m_url;
}

void QtCocoaWebView::setUrl(const QUrl &u)
{
    m_url = u;
    loadRequest(u.toString());
}

QString QtCocoaWebView::title() const
{
    return m_title;
}

int QtCocoaWebView::loadProgress()
{
    
}

qreal QtCocoaWebView::zoomFactor() const
{
    return 1.0;
}

void QtCocoaWebView::setZoomFactor(qreal factor)
{
    
}

void QtCocoaWebView::setHtml(const QString &html, const QUrl &baseUrl)
{
    
}

QIcon QtCocoaWebView::icon()
{
    return m_icon;
}

void QtCocoaWebView::onLoadFinish(const QString &strUrl, const QString &strHtml)
{
    emit loadFinish(strUrl, strHtml);
}

void QtCocoaWebView::onLoadError()
{
    emit loadError();
}

void QtCocoaWebView::onUrlChanged(const QString &strUrl)
{
    m_url = QUrl::fromUserInput(strUrl);
    emit urlChanged(strUrl);
}
