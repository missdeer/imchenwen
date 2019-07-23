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
        
        auto *cWebView = [[CocoaWebView alloc]initWithObjects:rect frameName:nil groupName:nil target:this];
        
        [[cWebView mainFrame] loadRequest: [NSURLRequest requestWithURL:QUrl::fromUserInput("https://minidump.info").toNSURL()
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
        
        auto *cWebView = [[CocoaWebView alloc]initWithObjects:rect frameName:nil groupName:nil target:this];
        
        [[cWebView mainFrame] loadRequest: [NSURLRequest requestWithURL:QUrl::fromUserInput(loadUrl).toNSURL()
          cachePolicy:NSURLRequestUseProtocolCachePolicy timeoutInterval:15]];
        
        setCocoaView(cWebView);
        
        [cWebView release];
    }
    
}

void QtCocoaWebView::loadRequest(const QString& loadUrl)
{
    @autoreleasepool{
        
        auto *cWebView = (CocoaWebView*)this->cocoaView();
        auto u = QUrl::fromUserInput(loadUrl);
        
        [[cWebView mainFrame] loadRequest:
            [NSURLRequest requestWithURL:u.toNSURL()]];
    }
}

bool QtCocoaWebView::canGoBack()
{
    @autoreleasepool{
        auto *cWebView = (CocoaWebView*)this->cocoaView();
        return cWebView.canGoBack;
    }
    return false;
}

bool QtCocoaWebView::canGoForward()
{
    @autoreleasepool{
        auto *cWebView = (CocoaWebView*)this->cocoaView();
        return cWebView.canGoForward;
    }
    return false;
}

void QtCocoaWebView::back()
{
    @autoreleasepool{
        auto *cWebView = (CocoaWebView*)this->cocoaView();
        
        [cWebView goBack];
    }
}

void QtCocoaWebView::forward()
{
    @autoreleasepool{
        auto* cWebView = (CocoaWebView*)this->cocoaView();
        
        [cWebView goForward];
    }
}

void QtCocoaWebView::reload()
{
    @autoreleasepool{
        auto *cWebView = (CocoaWebView*)this->cocoaView();
        
        [cWebView reload];
    }
}

void QtCocoaWebView::stop()
{
    @autoreleasepool{
        auto *cWebView = (CocoaWebView*)this->cocoaView();
        
        [cWebView stopLoading];
    }
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
    return 0;
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
    auto u = QUrl::fromUserInput(strUrl);
    if (u.isValid())
        m_url = u;
    emit loadFinish(strUrl, strHtml);
}

void QtCocoaWebView::onLoadError()
{
    emit loadError();
}

void QtCocoaWebView::onUrlChanged(const QString &strUrl)
{
    auto u = QUrl::fromUserInput(strUrl);
    if (u.isValid())
        m_url = u;
    emit urlChanged(strUrl);
}
