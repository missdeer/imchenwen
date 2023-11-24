/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#ifndef ACCESSMANAGER_H
#define ACCESSMANAGER_H

// Manage network access of imchenwen

#include <QHash>
#include <QNetworkAccessManager>
#include <QUrl>

#define DEFAULT_UA "Mozilla/5.0 (X11; Linux x86_64; rv:60.1) Gecko/20100101 Firefox/60.1"

class ProxyFactory;

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    enum ProxyType
    {
        NO_PROXY,
        HTTP_PROXY,
        SOCKS5_PROXY
    };
    Q_ENUM(ProxyType)

    explicit NetworkAccessManager(QObject *parent = nullptr);

    static NetworkAccessManager *instance(); // Singleton

    QNetworkReply *get(const QNetworkRequest &request);
    QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data);

    Q_INVOKABLE void setupProxy(ProxyType proxyType, const QString &proxy = QString(), bool proxyOnlyForParsing = false);

    void addUnseekableHost(const QString &host)
    {
        m_unseekableHosts << host;
    }
    bool urlIsUnseekable(const QUrl &url)
    {
        return m_unseekableHosts.contains(url.host());
    }

    void addReferer(const QUrl &url, const QByteArray &referer)
    {
        m_refererTable[url.host()] = referer;
    }
    QByteArray refererOf(const QUrl &url)
    {
        return m_refererTable[url.host()];
    }

    void addUserAgent(const QUrl &url, const QByteArray &ua)
    {
        m_ua_table[url.host()] = ua;
    }
    QByteArray userAgentOf(const QUrl &url)
    {
        return m_ua_table[url.host()].isEmpty() ? s_defaultUA : m_ua_table[url.host()];
    }

private:
    ProxyFactory              *m_proxyFactory;
    QStringList                m_unseekableHosts;
    QHash<QString, QByteArray> m_refererTable;
    QHash<QString, QByteArray> m_ua_table;

    static QByteArray s_defaultUA;
};

#endif // NETWORKMANAGER_H
