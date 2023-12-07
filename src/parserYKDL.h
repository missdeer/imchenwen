#ifndef ParserYKDL_H
#define ParserYKDL_H

#include "parserBase.h"

class ParserYKDL : public ParserBase
{
    Q_OBJECT
public:
    static ParserYKDL *instance()
    {
        return &s_instance;
    }

private slots:
    void parseOutput();

private:
    explicit ParserYKDL(QObject *parent = nullptr);
    ~ParserYKDL() override;
    void runParser(const QUrl &url) override;
    void parseEpisode(QJsonObject episode);

    static ParserYKDL s_instance;
};

#endif // ParserYKDL_H
