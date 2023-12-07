#ifndef PARSERLUX_H
#define PARSERLUX_H

#include "parserBase.h"

class ParserLux : public ParserBase
{
    Q_OBJECT
public:
    static ParserLux *instance()
    {
        return &s_instance;
    }

private slots:
    void parseOutput();

private:
    explicit ParserLux(QObject *parent = nullptr);
    ~ParserLux() override;
    void runParser(const QUrl &url) override;
    void parseEpisode(QJsonObject episode);

    static ParserLux s_instance;
};

#endif // PARSERLUX_H
