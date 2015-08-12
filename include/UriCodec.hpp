/**
 * Uri encode and decode (RFC1630, RFC1738, RFC2396)
 * http://www.codeguru.com/cpp/cpp/algorithms/strings/article.php/c12759/URI-Encoding-and-Decoding.htm
 *
 * Copyright 2006 (C) by Jin Qing
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OSHIYA_URI_CODEC__H
#define OSHIYA_URI_CODEC__H

#include <string>

namespace Oshiya
{
    namespace Util
    {
        extern const char HEX2DEC[256];
        std::string UriDecode(const std::string& sSrc);
        
        extern const char SAFE[256];
        std::string UriEncode(const std::string& sSrc);
    }
}

#endif
