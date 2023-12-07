#ifndef ParserYouGet_H
#define ParserYouGet_H

#include "parserBase.h"

class ParserYouGet : public ParserBase
{
    Q_OBJECT
public:
    explicit ParserYouGet(QObject *parent = nullptr);
    ~ParserYouGet() override;
    static ParserYouGet *instance()
    {
        return &s_instance;
    }

protected:
    void runParser(const QUrl &url) override;

private slots:
    void parseOutput();

private:
    void parseEpisode(QJsonObject episode);

    static ParserYouGet s_instance;
};

#endif // ParserYouGet_H
