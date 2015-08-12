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

#ifndef OSHIYA_STANZA_DISPATCHER__H
#define OSHIYA_STANZA_DISPATCHER__H

#include "InPacket.hpp"
#include "XmppUtils.hpp"
#include "SmartPointerUtil.hpp"

extern "C"
{
    #include "strophe.h"
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
    #include "src/common.h"
    #pragma GCC diagnostic pop
}

#include <memory>
#include <type_traits>
#include <unordered_map>

namespace Oshiya
{
    class StanzaDispatcher
    {
        public:
        ///////

        using StanzaHandler = std::function<void(const InPacket&)>;

        void addHandler(InPacket::Type, StanzaHandler handler);

        void handleMessage(const XmlElement& message);

        void handleIq(const XmlElement& iq);

        private:
        ////////

        struct EnumClassHash
        {
            template <typename T>
            std::size_t operator()(T t) const
            {
                return static_cast<std::size_t>(t);
            }
        };

        template <typename Key>
        using HashType =
        typename std::conditional
        <
            std::is_enum<Key>::value,
            EnumClassHash,
            std::hash<Key>
        >::type;

        using DispatchMap =
        std::unordered_map<InPacket::Type, StanzaHandler, HashType<InPacket::Type>>;

        void handlePubsubEvent(const XmlElement& event,
                               const Jid& from);

        void handleIqSet(const XmlElement& iq,
                         const Jid& from,
                         const std::string& id);

        void handleIqResult(const XmlElement& iq,
                            const Jid& from,
                            const std::string& id);

        void handleIqError(const XmlElement& iq,
                           const Jid& from,
                           const std::string& id);

        void handleInvalidStanza(const XmlElement& iq,
                                 const std::string& errorType,
                                 const std::string& condition,
                                 const std::string& ns,
                                 const std::string& appSpecificCondition = "",
                                 const std::string& appSpecificNs = "");

        void dispatch(InPacket::Type type, const InPacket& packet);

        DispatchMap mDispatchMap;
    };
}

#endif
