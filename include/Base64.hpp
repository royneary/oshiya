/*
  Copyright (c) 2005-2014 by Jakob Schroeter <js@camaya.net>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OSHIYA_BASE64__H
#define OSYHIA_BASE64__H

#include <string>

namespace Oshiya
{
    /**
     * @brief An implementation of the Base64 data encoding (RFC 3548)
     */
    namespace Util
    {
    
        /**
         * Base64-encodes the input according to RFC 3548.
         * @param input The data to encode.
         * @return The encoded string.
         */
        std::string base64Encode(const std::string& input);
    
        /**
         * Base64-decodes the input according to RFC 3548.
         * @param input The encoded data.
         * @return The decoded data.
         */
        std::string base64Decode(const std::string& input);
    
    }
}

#endif
