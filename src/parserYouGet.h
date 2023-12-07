#ifndef ParserYouGet_H
#define ParserYouGet_H

#include "parserBase.h"

class ParserYouGet : public ParserBase
{
    Q_OBJECT
public:
    static ParserYouGet *instance()
    {
        return &s_instance;
    }

private slots:
    void parseOutput();

private:
    explicit ParserYouGet(QObject *parent = nullptr);
    ~ParserYouGet() override;
    void runParser(const QUrl &url) override;
    void parseEpisode(QJsonObject episode);

    static ParserYouGet s_instance;
};

#endif // ParserYouGet_H
