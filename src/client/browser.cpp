#include "browser.h"
#include "browserwindow.h"
#include "webview.h"
#include "linkresolver.h"
#include "waitingspinnerwidget.h"
#include "playdialog.h"
#include <QAuthenticator>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>
#include <QApplication>
#include <QClipboard>

void Browser::startParsedProcess()
{
    QString parsedPath = QApplication::applicationDirPath();
#if defined(Q_OS_WIN)
    parsedPath.append("/parsed.exe");
#else
    parsedPath.append("/../Resources/parsed");
#endif
    if (!QFile::exists(parsedPath))
    {
        QMessageBox::information(nullptr, tr("Notice"), tr("Can't launch parsed process for link resolving, please launch it by yourself."), QMessageBox::Ok);
    }
    else
    {
        QProcess::startDetached(parsedPath, QStringList() << "-l");
    }
}

Browser::Browser(QObject* parent)
    : QObject(parent)
    , m_waitingSpinner(nullptr)
    , m_linkResolver(this)
{
    connect(&m_linkResolver, &LinkResolver::resolvingFinished, this, &Browser::resolvingFinished);
    connect(&m_linkResolver, &LinkResolver::resolvingError, this, &Browser::resolvingError);
    connect(&m_linkResolver, &LinkResolver::resolvingSilentFinished, this, &Browser::resolvingSilentFinished);
    connect(&m_linkResolver, &LinkResolver::resolvingSilentError, this, &Browser::resolvingSilentError);
    connect(&m_process, &QProcess::errorOccurred, this, &Browser::errorOccurred);
    Config cfg;
    if (cfg.read<bool>("inChinaLocalMode") || cfg.read<bool>("abroadLocalMode"))
    {
        startParsedProcess();
    }

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &Browser::clipboardChanged);
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

void Browser::playByMediaPlayer(const QUrl& u)
{
    resolveLink(u);
}

void Browser::clipboardChanged()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString originalText = clipboard->text();

    if (originalText.startsWith("http://") || originalText.startsWith("https://"))
    {
        m_linkResolver.resolve(QUrl(originalText), true);
    }
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

void Browser::doPlayByMediaPlayer(MediaInfoPtr mi)
{
    PlayDialog dlg(m_windows.isEmpty() ? nullptr : reinterpret_cast<QWidget*>(const_cast<BrowserWindow*>(m_windows.at(0))) );
    dlg.setMediaInfo(mi);
    if (dlg.exec())
    {
        Tuple2 player = dlg.player();
        StreamInfoPtr stream = dlg.media();

        if (m_process.state() != QProcess::NotRunning)
        {
            m_process.terminate();
        }

        QStringList args;
        m_process.setProgram(std::get<0>(player));
#if defined(Q_OS_MAC)
        QFileInfo fi(std::get<0>(player));
        if (fi.suffix() == "app")
        {
            m_process.setProgram("/usr/bin/open");
            args << std::get<0>(player) << "--args";
        }
#endif
        QString arg = std::get<1>(player);
        if (!arg.isEmpty())
            args << arg.split(" ");
        args << stream->urls;

#if defined(Q_OS_MAC)
        if (fi.suffix() == "app")
        {
            m_process.start("/usr/bin/open", args);
            return;
        }
#endif
        m_process.start(std::get<0>(player), args);
    }
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

    doPlayByMediaPlayer(mi);
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

void Browser::resolvingSilentFinished(MediaInfoPtr mi)
{
    if (mi->title.isEmpty() && mi->site.isEmpty())
    {
        return;
    }

    if (!m_windows.isEmpty())
    {
        m_windows.at(0)->activateWindow();
        m_windows.at(0)->raise();
    }
    doPlayByMediaPlayer(mi);
}

void Browser::resolvingSilentError()
{

}

void Browser::errorOccurred(QProcess::ProcessError error)
{
    QString msg;
    switch(error)
    {
    case QProcess::FailedToStart:
        msg = tr("The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program.");
        break;
    case QProcess::Crashed:
        msg = tr("The process crashed some time after starting successfully.");
        break;
    default:
        msg = tr("An unknown error occurred.");
        break;
    }
    QMessageBox::warning((m_windows.isEmpty() ? nullptr : m_windows.at(0)),
                         tr("Error on launching external player"), msg, QMessageBox::Ok);
}
