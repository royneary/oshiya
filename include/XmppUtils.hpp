/**
 * This file is part of Oshiya, an XEP-0357 compatible XMPP component
 * Copyright (C) 2015 Christian Ulrich <christian@rechenwerk.net>
 * 
 * Oshiya is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Oshiya is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Oshiya.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OSHIYA_XMPP_UTILS__H
#define OSHIYA_XMPP_UTILS__H

#include "Jid.hpp"
#include "XmlElement.hpp"
#include "XData.hpp"

extern "C"
{
    #include "strophe.h"
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
    #include "src/common.h"
    #pragma GCC diagnostic pop
}

#include <memory>
#include <stdexcept>

namespace Oshiya
{
    namespace Util
    {
        Jid makeJid(const std::string& str, xmpp_ctx_t* const ctx);

        /**
         * constructs a string from a char*. If NULL constructs an empty
         * string
         */
        std::string makeString(const char* maybeNull);

        /**
         * adds an error element with the given condition elements
         */
        void addErrorToStanza(XmlElement& stanza,
                              const std::string& errorType,
                              const std::string& condition,
                              const std::string& ns,
                              const std::string& appSpecificCondition = "",
                              const std::string& appSpecificNs = "");

        /**
         * switches a stanza's from and to attributes
         */
        void switchFromTo(XmlElement& stanza);

        /**
         * calls addErrorToStanza and switchFromTo on a copy of the given stanza
         */
        XmlElement makeStanzaError(const XmlElement& stanza,
                                   const std::string& errorType,
                                   const std::string& condition,
                                   const std::string& ns,
                                   const std::string& appSpecificCondition = "",
                                   const std::string& appSpecificNs = "");

        XData parseXData(const XmlElement& el);

        XmlElement makeXDataElement(const XData& xdata, xmpp_ctx_t* ctx);
    }
}

#endif
