#include "externalplay.h"
#include "browser.h"
#include "externalplaydialog.h"

ExternalPlay::ExternalPlay()
{

}

void ExternalPlay::Play(const Streams& streams)
{
    auto windows = Browser::instance().windows();
    ExternalPlayDialog dlg(windows.isEmpty() ? nullptr : reinterpret_cast<QWidget*>(const_cast<BrowserWindow*>(windows.at(0))) );
    dlg.setStreams(streams);
    if (dlg.exec())
    {
        Tuple2 player = dlg.player();
        StreamInfoPtr stream = dlg.media();

        QProcess& p = process();
        if (p.state() != QProcess::NotRunning)
        {
            p.terminate();
        }
        p.setProgram(std::get<0>(player));
        QStringList args;
        QString arg = std::get<1>(player);
        if (!arg.isEmpty())
            args << arg.split(" ");
        args << stream->urls;
        p.start(std::get<0>(player), args);
    }
}

QProcess& ExternalPlay::process()
{
    static QProcess p;
    return p;
}
