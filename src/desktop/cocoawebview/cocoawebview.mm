#import "cocoawebview.h"
#include <QPixmap>
#include <QIcon>
#include <QtMac>

@interface CocoaWebView()<WebFrameLoadDelegate, WebResourceLoadDelegate, WebUIDelegate, WebPolicyDelegate>
@end

@implementation CocoaWebView

-(id) initWithObjects:(NSRect)frameRect frameName:(NSString *)frameName groupName:(NSString *)groupName target:(QPointer<QtCocoaWebView>) target
{
    self = [super initWithFrame:frameRect frameName:frameName groupName:groupName];
    if (self)
    {
        [self setUIDelegate:self];
        [self setResourceLoadDelegate:self];
        [self setFrameLoadDelegate:self];
        [self setPolicyDelegate:self];
        
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
       pTarget->onLoadError();
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
            pTarget->onLoadFinish(strUrl, strHtml);
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
            pTarget->onUrlChanged(str);
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
            pTarget->onTitleChanged(str);
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
            pTarget->onIconChanged(icon);
        }
    }
}

- (void)webView:(WebView *)sender setStatusText:(NSString *)text
{
    Q_UNUSED(sender);

    @autoreleasepool {
        if (pTarget)
        {
            QString statusText = QString::fromNSString(text);
            pTarget->onStatusTextChanged(statusText);
        }
    }
}

- (void)webView:(WebView *)sender decidePolicyForNewWindowAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request newFrameName:(NSString *)frameName decisionListener:(id<WebPolicyDecisionListener>)listener
{
    Q_UNUSED(sender);
    @autoreleasepool {
        if (pTarget)
        {
            QString strUrl = QString::fromNSString(request.URL.absoluteString);
            pTarget->onNewWindowsRequest(strUrl);
        }
    }
}
@end

