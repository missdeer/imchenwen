#ifndef COCOAWEBVIEW_H
#define COCOAWEBVIEW_H

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import <WebKit/WebView.h>
#include <QPointer>
#include "qtcocoawebview.h"

@interface CocoaWebView: WebView {
    QPointer<QtCocoaWebView> pTarget;
}

-(id) initWithObjects:(NSRect)frameRect frameName:(NSString *)frameName
                groupName:(NSString *)groupName target:(QPointer<QtCocoaWebView>) target;

@end

#endif
