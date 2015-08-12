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

#ifndef OSHIYA_OUT_PACKET__H
#define OSHIYA_OUT_PACKET__H

#include "Jid.hpp"
#include "XData.hpp"
#include "XmlElement.hpp"

namespace Oshiya
{
    struct OutPacket
    {
        enum class Type
        {
            CreatePubsubNode,
            DeletePubsubNode,
            SetPubsubAffiliation,
            PubsubSubscribe,
            CommandCompleted,
            CommandError
        };

        virtual ~OutPacket() = 0;

        virtual XmlElement makeXmlElement(xmpp_ctx_t* ctx) const = 0;
    };

    inline OutPacket::~OutPacket() { }
    
    template <OutPacket::Type>
    struct OutStanza;

    template <>
    struct OutStanza<OutPacket::Type::CreatePubsubNode> : public OutPacket
    {
        OutStanza(const Jid& _from,
                  const Jid& _to,
                  const std::string& _id,
                  const std::string& _node,
                  const XData& _nodeConfig)
            :
                from {_from},
                to {_to},
                id {_id},
                node {_node},
                nodeConfig {_nodeConfig}
        { }

        XmlElement makeXmlElement(xmpp_ctx_t* ctx) const override;

        const Jid from;
        const Jid to;
        const std::string id;
        const std::string node;
        const XData nodeConfig;
    };

    template <>
    struct OutStanza<OutPacket::Type::DeletePubsubNode> : public OutPacket
    {
        OutStanza(const Jid& _from,
                  const Jid& _to,
                  const std::string& _id,
                  const std::string& _node)
            :
                from {_from},
                to {_to},
                id {_id},
                node {_node}
        { }

        XmlElement makeXmlElement(xmpp_ctx_t* ctx) const override;

        const Jid from;
        const Jid to;
        const std::string id;
        const std::string node;
    };

    template <>
    struct OutStanza<OutPacket::Type::SetPubsubAffiliation> : public OutPacket
    {
        OutStanza(const Jid& _from,
                  const Jid& _to,
                  const std::string& _id,
                  const std::string& _node,
                  const Jid& _jid,
                  const std::string& _affiliation)
            :
                from {_from},
                to {_to},
                id {_id},
                node {_node},
                jid {_jid},
                affiliation {_affiliation}
        { }

        XmlElement makeXmlElement(xmpp_ctx_t* ctx) const override;

        const Jid from;
        const Jid to;
        const std::string id;
        const std::string node;
        const Jid jid;
        const std::string affiliation;
    };

    template <>
    struct OutStanza<OutPacket::Type::PubsubSubscribe> : public OutPacket
    {
        OutStanza(const Jid& _from,
                  const Jid& _to,
                  const std::string& _id,
                  const std::string& _node)
            :
                from {_from},
                to {_to},
                id {_id},
                node {_node}
        { }

        XmlElement makeXmlElement(xmpp_ctx_t* ctx) const override;

        const Jid from;
        const Jid to;
        const std::string id;
        const std::string node;
    };

    template <>
    struct OutStanza<OutPacket::Type::CommandCompleted> : public OutPacket
    {
        OutStanza(const Jid& _from,
                  const Jid& _to,
                  const std::string& _id,
                  const std::string& _node,
                  const XData& _payload)
            :
                from {_from},
                to {_to},
                id {_id},
                node {_node},
                payload {_payload}
        { }

        XmlElement makeXmlElement(xmpp_ctx_t* ctx) const override;

        const Jid from;
        const Jid to;
        const std::string id;
        const std::string node;
        const XData payload;
    };

    template <>
    struct OutStanza<OutPacket::Type::CommandError> : public OutPacket
    {
        OutStanza(const Jid& _from,
                  const Jid& _to,
                  const std::string& _id,
                  const std::string& _node,
                  const std::string& _action,
                  const std::string& _errorType,
                  const std::string& _condition,
                  const std::string& _appSpecificCondition)
            :
                from {_from},
                to {_to},
                id {_id},
                node {_node},
                action {_action},
                errorType {_errorType},
                condition {_condition},
                appSpecificCondition {_appSpecificCondition}
        { }

        XmlElement makeXmlElement(xmpp_ctx_t* ctx) const override;

        const Jid from;
        const Jid to;
        const std::string id;
        const std::string node;
        const std::string action;
        const std::string errorType;
        const std::string condition;
        const std::string appSpecificCondition;
    };
}

#endif
