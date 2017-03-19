#include "externalplay.h"
#include "browser.h"
#include "externalplaydialog.h"
#include <QFileInfo>

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

        QProcess& p = Browser::instance().process();
        if (p.state() != QProcess::NotRunning)
        {
            p.terminate();
        }

        QStringList args;
        p.setProgram(std::get<0>(player));
#if defined(Q_OS_MAC)
        QFileInfo fi(std::get<0>(player));
        if (fi.suffix() == "app")
        {
            p.setProgram("/usr/bin/open");
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
            p.start("/usr/bin/open", args);
            return;
        }
#endif
        p.start(std::get<0>(player), args);
    }
}
