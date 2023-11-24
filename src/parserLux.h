#ifndef PARSERLUX_H
#define PARSERLUX_H

#include <QProcess>

#include "parserBase.h"

class ParserLux : public ParserBase
{
    Q_OBJECT
public:
    explicit ParserLux(QObject *parent = nullptr);
    ~ParserLux() override;
    static ParserLux *instance()
    {
        return &s_instance;
    }

protected:
    void runParser(const QUrl &url) override;

private slots:
    void parseOutput();

private:
    void parseEpisode(QJsonObject episode);

    QProcess         m_process;
    static ParserLux s_instance;
};

#endif // PARSERLUX_H
