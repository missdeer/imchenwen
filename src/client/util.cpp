#include "util.h"
#include "config.h"
#include <QNetworkInterface>

namespace Util {

    QString secToTime(int second, bool useFormat)
    {
        static QString format = "<span style=\" font-size:14pt; font-weight:600;color:#00ff00;\">%1:%2:%3</span>";
        QString  hour = QString::number(second / 3600);
        QString min = QString::number((second % 3600) / 60);
        QString sec = QString::number(second % 60);
        if (min.length() == 1)
            min.prepend('0');
        if (sec.length() == 1)
            sec.prepend('0');
        if (useFormat)
            return format.arg(hour, min, sec);
        else
            return QString("%1:%2:%3").arg(hour, min, sec);
    }


    QHostAddress getLocalAddress()
    {
        Config cfg;
        QString ip = cfg.read<QString>("dlnaUseIP");
        if (!ip.isEmpty())
        {
            for(auto && address : QNetworkInterface::allAddresses())
            {
                if (address == QHostAddress(ip))
                    return QHostAddress(ip);
            }
        }

        // see http://stackoverflow.com/questions/13835989/get-local-ip-address-in-qt
        for(auto && address : QNetworkInterface::allAddresses())
        {
            if (address.protocol() == QAbstractSocket::IPv4Protocol
                    && address != QHostAddress(QHostAddress::LocalHost) // Check if it is local adress
                    && address.toString().section( ".",-1,-1 ) != "1") // Check if it is virtual machine
                 return address;
        }
        return QHostAddress();
    }

}
