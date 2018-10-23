/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef BROWSER_H
#define BROWSER_H

#include <QObject>
#include <QVector>
#include <QProcess>
#include "linkresolver.h"
#include "websites.h"

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

class BrowserWindow;
class WaitingSpinnerWidget;

class Browser : public QObject
{
    Q_OBJECT
public:
    static Browser &instance();
    ~Browser();

    QVector<BrowserWindow*> windows();
    void addWindow(BrowserWindow* window);
    BrowserWindow *mainWindow();
    BrowserWindow *newMainWindow();

    void loadSettings();
    void playByMediaPlayer(const QString& u);
    void playVIPByMediaPlayer(const QString &u);
    Websites &websites();
signals:

private slots:
#if defined(Q_OS_WIN)
    void finished();
    void ping();
#endif
    void clipboardChanged();
    void resolvingFinished(MediaInfoPtr mi);
    void resolvingError(const QString&);
    void errorOccurred(QProcess::ProcessError error);
    void playerFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void stopWaiting();
private:
    explicit Browser(QObject *parent = nullptr);
    void resolveLink(const QString &u, bool vip);
    void doPlayByMediaPlayer(MediaInfoPtr mi);
    void clean();
    void waiting(bool disableParent = true);
private:
    QVector<BrowserWindow*> m_windows;
    WaitingSpinnerWidget* m_waitingSpinner;
    QProcess m_process;
    QProcess m_parsedProcess;
    LinkResolver m_linkResolver;
    Websites m_websites;
    QNetworkAccessManager* m_nam;
    void clearAtExit();
};
#endif // BROWSER_H
