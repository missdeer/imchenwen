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

#ifndef ParserYtdlp_H
#define ParserYtdlp_H

#include "parserYoutubeDLBase.h"

class ParserYtdlp : public ParserYoutubeDLBase
{
    Q_OBJECT
public:
    static ParserYtdlp *instance()
    {
        return &s_instance;
    }
    explicit ParserYtdlp(QObject *parent = nullptr);
    ~ParserYtdlp() override = default;


private:

    static ParserYtdlp s_instance;
};

#endif // ParserYtdlp_H
