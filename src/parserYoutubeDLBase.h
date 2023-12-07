/* Copyright 2013-2022 Yikun Liu <cos.lyk@gmail.com>
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

#ifndef ParserYoutubeDLBase_H
#define ParserYoutubeDLBase_H

#include "parserBase.h"

class ParserYoutubeDLBase : public ParserBase
{
    Q_OBJECT
public:
    static ParserYoutubeDLBase *instance()
    {
        return &s_instance;
    }
    explicit ParserYoutubeDLBase(QObject *parent = nullptr);
    ~ParserYoutubeDLBase() override;

protected:
    void runParser(const QUrl &url) override;
    QString m_parserFileName;

private slots:
    void parseOutput();

private:
    void convertToStream(const QJsonObject &item, Stream &stream);

    static ParserYoutubeDLBase s_instance;
};

#endif // ParserYoutubeDLBase_H
