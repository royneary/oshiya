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

#ifndef OSHIYA_IN_PACKET__H
#define OSHIYA_IN_PACKET__H

#include "Jid.hpp"
#include "XData.hpp"
#include "XmlElement.hpp"

namespace Oshiya
{
    struct InPacket
    {
        enum class Type
        {
            AdhocCommand,
            IqResult,
            IqError,
            PushNotification,
            Invalid
        };

        virtual ~InPacket() = 0;
    };

    inline InPacket::~InPacket() { }

    template <InPacket::Type>
    struct InStanza;

    // XEP-0050 adhoc command
    template <>
    struct InStanza<InPacket::Type::AdhocCommand> : public InPacket
    {
        InStanza(const Jid& _from,
               const std::string& _id,
               const std::string& _action,
               const std::string& _node,
               const XData& _payload)
            :
                from {_from},
                id {_id},
                action {_action},
                node {_node},
                payload {_payload}
        { }

        const Jid from;
        const std::string id;
        const std::string action;
        const std::string node;
        const XData payload;
    };

    // generic iq of type 'result'
    template <>
    struct InStanza<InPacket::Type::IqResult> : public InPacket
    {
        InStanza(const Jid& _from,
               const std::string& _id)
            :
                from {_from},
                id {_id}
        { }

        const Jid from;
        const std::string id;
    };

    // generic iq of type 'error'
    template <>
    struct InStanza<InPacket::Type::IqError> : public InPacket
    {
        InStanza(const Jid& _from,
               const std::string& _id,
               const std::string& _errorType,
               const std::vector<std::string>& _errors)
            :
                from {_from},
                id {_id},
                errorType {_errorType},
                errors {_errors}
        { }

        const Jid from;
        const std::string id;
        const std::string errorType;
        const std::vector<std::string> errors;
    };

    // XEP-0357 push notification (pubsub item with notification payload)
    template <>
    struct InStanza<InPacket::Type::PushNotification> : public InPacket
    {
        InStanza(const Jid& _from,
               const std::string& _node,
               const XData& _payload)
            :
                from {_from},
                node {_node},
                payload {_payload}
        { }
    
        const Jid from;
        const std::string node;
        const XData payload;
    };

    // invalid stanza (non of the above)
    template <>
    struct InStanza<InPacket::Type::Invalid> : public InPacket
    {
        InStanza(const XmlElement& _stanzaError)
            :
                stanzaError {_stanzaError}
        { }

        // a stanza error to be sent if stanzaError.isValid()
        const XmlElement stanzaError;
    };
}

#endif
