#import "cocoawebview.h"
#include <QPixmap>
#include <QIcon>
#include <QtMac>

@interface CocoaWebView()<WebFrameLoadDelegate, WebResourceLoadDelegate>
@end

@implementation CocoaWebView

-(id) initWithObjects:(NSRect)frameRect frameName:(NSString *)frameName groupName:(NSString *)groupName target:(QPointer<QtCocoaWebView>) target
{
    self = [super initWithFrame:frameRect frameName:frameName groupName:groupName];
    if (self)
    {
        [self setResourceLoadDelegate:self];
        [self setFrameLoadDelegate:self];

        pTarget = target;
    }
    return self;
}

- (void)webView:(WebView *)sender didStartProvisionalLoadForFrame:(WebFrame *)frame
{
    Q_UNUSED(sender);
    Q_UNUSED(frame);
}

- (void)webView:(WebView *)sender resource:(id)identifier didFailLoadingWithError:(NSError *)error fromDataSource:(WebDataSource *)dataSource
{
    Q_UNUSED(sender);
    Q_UNUSED(identifier);
    Q_UNUSED(error);
    Q_UNUSED(dataSource);
    if(pTarget)
    {
       pTarget->loadError();
    }
}


- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    Q_UNUSED(sender);
    @autoreleasepool{
        NSString *strCurr = [[[[frame dataSource]request]URL]absoluteString];
        QString strUrl = QString::fromNSString(strCurr);

        WebDataSource *source = [frame dataSource];
        NSData *data = [source data];
        NSString *nsstr = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        QString strHtml = QString::fromNSString(nsstr);


        if(pTarget)
        {
            pTarget->loadFinish(strUrl, strHtml);
        }
    }
}

- (void)webView:(WebView *)sender didCommitLoadForFrame:(WebFrame *)frame
{
    Q_UNUSED(sender);
    @autoreleasepool{

        NSString *strCurr = [[[[frame dataSource]request]URL]absoluteString];
        QString str = QString::fromNSString(strCurr);

        if(pTarget)
        {
            pTarget->urlChanged(str);
        }
    }
}

- (void)webView:(WebView *)sender didReceiveTitle:(NSString *)title forFrame:(WebFrame *)frame
{
    Q_UNUSED(sender);
    Q_UNUSED(frame);
    @autoreleasepool{
        if (pTarget)
        {
            QString str = QString::fromNSString(title);
            pTarget->titleChanged(str);
        }
    }
}

- (void)webView:(WebView *)sender didReceiveIcon:(NSImage *)image forFrame:(WebFrame *)frame
{
    Q_UNUSED(sender);
    Q_UNUSED(frame);
    @autoreleasepool {
        if (pTarget)
        {
            CGImageRef cgImgRef = [image CGImageForProposedRect:nil context:nil hints:nil];
            QPixmap pm = QtMac::fromCGImageRef(cgImgRef);
            QIcon icon(pm);
            pTarget->iconChanged(icon);
        }
    }
}

@end

