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
    setCocoaView(cWebView);

    [cWebView release];

    }
}

QtCocoaWebView::QtCocoaWebView(QString loadUrl, QWidget *parent) :
    QMacCocoaViewContainer(nullptr, parent)
{
    @autoreleasepool{

    NSString *strUrl = loadUrl.toNSString();

    NSRect rect = {{0, 0}, {(CGFloat)parent->width(), (CGFloat)parent->height()}};

    CocoaWebView *cWebView = [[CocoaWebView alloc]initWithObjects:rect frameName:nil groupName:nil target:this];

    [[cWebView mainFrame] loadRequest: [NSURLRequest requestWithURL:[NSURL URLWithString:strUrl]
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

void QtCocoaWebView::goBack()
{

}

void QtCocoaWebView::goForward()
{

}

void QtCocoaWebView::onTitleChanged(const QString &strTitle)
{
    emit titleChanged(strTitle);
}

void QtCocoaWebView::onIconChanged(const QIcon &icon)
{
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
    emit urlChanged(strUrl);
}
