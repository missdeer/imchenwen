#ifndef ParserYKDL_H
#define ParserYKDL_H

#include "parserBase.h"

class ParserYKDL : public ParserBase
{
    Q_OBJECT
public:
    explicit ParserYKDL(QObject *parent = nullptr);
    ~ParserYKDL() override;
    static ParserYKDL *instance()
    {
        return &s_instance;
    }

protected:
    void runParser(const QUrl &url) override;

private slots:
    void parseOutput();

private:
    void parseEpisode(QJsonObject episode);

    static ParserYKDL s_instance;
};

#endif // ParserYKDL_H
