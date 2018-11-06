#ifndef INMEMORYHANDLER_H
#define INMEMORYHANDLER_H

#include <qhttpengine/handler.h>

class InMemoryHandler : public QHttpEngine::Handler
{
public:
    explicit InMemoryHandler(QObject *parent = nullptr);

    void setM3U8(const QByteArray& m3u8);
protected:

    /**
     * @brief Reimplementation of [Handler::process()](QHttpEngine::Handler::process)
     */
    virtual void process(QHttpEngine::Socket *socket, const QString &path);

private:
    QByteArray m_m3u8;
};

#endif // INMEMORYHANDLER_H
