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

void QtCocoaWebView::titleChanged(const QString &strTitle)
{
    emit signalTitleChanged(strTitle);
}

void QtCocoaWebView::iconChanged(const QIcon &icon)
{
    emit signalIconChanged(icon);
}

void QtCocoaWebView::loadFinish(const QString &strUrl, const QString &strHtml)
{
    emit signalLoadFinish(strUrl, strHtml);
}

void QtCocoaWebView::loadError()
{
    emit signalLoadError();
}

void QtCocoaWebView::urlChanged(const QString &strUrl)
{
    emit signalUrlChanged(strUrl);
}
