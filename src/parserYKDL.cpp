#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSettings>

#include "parserYKDL.h"
#include "accessManager.h"
#include "dialogs.h"
#include "platform/paths.h"

ParserYKDL ParserYKDL::s_instance;

ParserYKDL::ParserYKDL(QObject *parent) : ParserBase(parent)
{
    // Connect
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ParserYKDL::parseOutput);
}

ParserYKDL::~ParserYKDL() = default;

void ParserYKDL::runParser(const QUrl &url)
{
    // Check if another task is running
    if (m_process.state() == QProcess::Running)
    {
        Q_ASSERT(Dialogs::instance() != nullptr);
        Dialogs::instance()->messageDialog(tr("Error"), tr("Another file is being parsed."));
        return;
    }

    // Get and apply proxy settings
    QSettings                       settings;
    NetworkAccessManager::ProxyType proxyType = static_cast<NetworkAccessManager::ProxyType>(settings.value(QStringLiteral("network/proxy_type")).toInt());
    QString                         proxy     = settings.value(QStringLiteral("network/proxy")).toString();

    if (!proxy.isEmpty() && proxyType == NetworkAccessManager::HTTP_PROXY)
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("HTTP_PROXY"), QStringLiteral("http://%1/").arg(proxy));
        m_process.setProcessEnvironment(env);
    }
    else if (!proxy.isEmpty() && proxyType == NetworkAccessManager::SOCKS5_PROXY)
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("HTTP_PROXY"), QStringLiteral("socks5://%1/").arg(proxy));
        m_process.setProcessEnvironment(env);
    }

    // Set user-agent
    QStringList args;
    args << QStringLiteral("-j"); //<< QStringLiteral("-p") << QStringLiteral("-u") << QStringLiteral(DEFAULT_UA);
    if (QFile::exists(userResourcesPath() + QStringLiteral("/cookie.txt")))
    {
        args << QStringLiteral("-c") << userResourcesPath() + QStringLiteral("/cookie.txt");
    }
    else if (QFile::exists(userResourcesPath() + QStringLiteral("/") + url.host() + QStringLiteral("_cookie.txt")))
    {
        args << QStringLiteral("-c") << userResourcesPath() + QStringLiteral("/") + url.host() + QStringLiteral("_cookie.txt");
    }
    args << url.toString();
    m_process.setWorkingDirectory(userResourcesPath());
    m_process.start(userResourcesPath() + QStringLiteral("/ykdl"), args, QProcess::ReadOnly);
}

void ParserYKDL::parseOutput()
{
    QByteArray output = m_process.readAllStandardOutput();

    // Parse JSON
    QJsonParseError json_error;
    QJsonDocument   document = QJsonDocument::fromJson(output, &json_error);

    if (json_error.error != QJsonParseError::NoError)
    {
        showErrorDialog(QString::fromUtf8(m_process.readAllStandardError()));
        return;
    }

    // Get episode list
    QJsonArray episodes = document.array();
    if (episodes.size() == 0)
    {
        Dialogs::instance()->messageDialog(tr("Error"), tr("The given url has no video item."));
    }
    else if (episodes.size() == 1)
    {
        parseEpisode(episodes[0].toObject());
    }
    else
    {
        QStringList titles;
        for (auto episode : episodes)
        {
            titles << episode.toObject()[QStringLiteral("title")].toString();
            Dialogs::instance()->selectionDialog(
                tr("Select episode"), titles, [=](int index, bool) { parseEpisode(episodes[index].toObject()); }, QString());
        }
    }
}

void ParserYKDL::parseEpisode(QJsonObject episode)
{
    // Check error
    if (!episode[QStringLiteral("err")].isNull())
    {
        showErrorDialog(QStringLiteral("YKDL Error: ") + episode[QStringLiteral("err")].toString());
        return;
    }

    // Get title
    m_result.title = episode[QStringLiteral("title")].toString();

    // Get danmaku
    if (!episode[QStringLiteral("caption")].isNull())
    {
        QJsonObject caption = episode[QStringLiteral("caption")].toObject();
        if (!caption[QStringLiteral("danmaku")].isNull())
        {
            m_result.danmaku_url = caption[QStringLiteral("danmaku")].toObject()[QStringLiteral("url")].toString();
        }
        if (!caption[QStringLiteral("subtitle")].isNull())
        {
            m_result.subtitle_url = caption[QStringLiteral("subtitle")].toObject()[QStringLiteral("url")].toString();
        }
    }

    // get all available streams
    QJsonObject streams = episode[QStringLiteral("streams")].toObject();
    for (auto iter = streams.constBegin(); iter != streams.constEnd(); iter++)
    {
        QJsonObject item = iter.value().toObject();

        // Basic stream infos
        Stream stream;
        stream.container = item[QStringLiteral("ext")].toString();
        stream.is_dash   = item[QStringLiteral("NeedMux")].toBool();
        stream.referer   = episode[QStringLiteral("url")].toString();
        stream.seekable  = true;

        // Write urls list
        QJsonArray parts = item[QStringLiteral("parts")].toArray();
        if (parts.count() == 0) // this stream is not available, skip it
        {
            continue;
        }

        for (const auto &part : parts)
        {
            stream.urls << QUrl(part.toObject()[QStringLiteral("url")].toString());
        }

        // Add stream to list
        m_result.stream_types << item[QStringLiteral("quality")].toString();
        m_result.streams << stream;
    }
    finishParsing();
}