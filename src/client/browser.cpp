#include "browser.h"
#include "browserwindow.h"
#include "webview.h"
#include "linkresolver.h"
#include "waitingspinnerwidget.h"
#include <QAuthenticator>
#include <QMessageBox>

Browser::Browser(QObject* parent)
    : QObject(parent)
    , m_waitingSpinner(nullptr)
    , m_linkResolver(this)
{
    connect(&m_linkResolver, &LinkResolver::resolvingFinished, this, &Browser::resolvingFinished);
    connect(&m_linkResolver, &LinkResolver::resolvingError, this, &Browser::resolvingError);
}

Browser::~Browser()
{
    qDeleteAll(m_windows);
    m_windows.clear();

    if (m_waitingSpinner)
    {
        if (m_waitingSpinner->isSpinning())
            m_waitingSpinner->stop();
        delete m_waitingSpinner;
    }
}

Browser &Browser::instance()
{
    static Browser browser;
    return browser;
}

void Browser::playByExternalPlayer(const QUrl& u)
{
    m_playByBuiltinPlayer = false;
    resolveLink(u);
}

void Browser::playByBuiltinPlayer(const QUrl& u)
{
    m_playByBuiltinPlayer = true;
    resolveLink(u);
}

QVector<BrowserWindow*> Browser::windows()
{
    return m_windows;
}

void Browser::addWindow(BrowserWindow *mainWindow)
{
    if (m_windows.contains(mainWindow))
        return;
    m_windows.prepend(mainWindow);
    QObject::connect(mainWindow, &QObject::destroyed, [this, mainWindow]() {
        m_windows.removeOne(mainWindow);
    });
    mainWindow->show();
}

void Browser::resolveLink(const QUrl &u)
{
    if (!m_waitingSpinner)
    {
        m_waitingSpinner = new WaitingSpinnerWidget((m_windows.isEmpty() ? nullptr : m_windows.at(0)));

        m_waitingSpinner->setRoundness(70.0);
        m_waitingSpinner->setMinimumTrailOpacity(15.0);
        m_waitingSpinner->setTrailFadePercentage(30.0);
        m_waitingSpinner->setNumberOfLines(12);
        m_waitingSpinner->setLineLength(40);
        m_waitingSpinner->setLineWidth(5);
        m_waitingSpinner->setInnerRadius(10);
        m_waitingSpinner->setRevolutionsPerSecond(1);
        //        m_waitingSpinner->setColor(QColor(81, 4, 71));
    }
    if (m_waitingSpinner->isSpinning())
        m_waitingSpinner->stop();

    m_waitingSpinner->start();

    m_linkResolver.resolve(u);
}

void Browser::resolvingFinished(MediaInfoPtr mi)
{
    if (m_waitingSpinner->isSpinning())
        m_waitingSpinner->stop();
    m_waitingSpinner->deleteLater();
    m_waitingSpinner = nullptr;

    if (mi->title.isEmpty() && mi->site.isEmpty())
    {
        QMessageBox::warning((m_windows.isEmpty() ? nullptr : m_windows.at(0)),
                             tr("Error"), tr("Resolving link address failed! Please try again."), QMessageBox::Ok);
        return;
    }

    if (!m_playByBuiltinPlayer)
    {
        if (!mi->ykdl.isEmpty())
        {
            m_externalPlay.Play(mi->ykdl);
            return;
        }
        if (!mi->youtube_dl.isEmpty())
        {
            m_externalPlay.Play(mi->youtube_dl);
            return;
        }
        if (!mi->you_get.isEmpty())
        {
            m_externalPlay.Play(mi->you_get);
            return;
        }
    }
}

void Browser::resolvingError()
{
    if (m_waitingSpinner->isSpinning())
        m_waitingSpinner->stop();
    m_waitingSpinner->deleteLater();
    m_waitingSpinner = nullptr;

    QMessageBox::warning((m_windows.isEmpty() ? nullptr : m_windows.at(0)),
                         tr("Error"), tr("Resolving link address failed!"), QMessageBox::Ok);
}
