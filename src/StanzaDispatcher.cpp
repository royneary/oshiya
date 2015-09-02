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

#include "StanzaDispatcher.hpp"

// DEBUG:
#include <iostream>

using namespace Oshiya;

void StanzaDispatcher::handleMessage(const XmlElement& message)
{
    const auto& makeString = Util::makeString;

    xmpp_stanza_t* const stanza {message.getStanzaPtr()};
    xmpp_ctx_t* ctx {stanza->ctx};

    Jid from
    {Util::makeJid(makeString(xmpp_stanza_get_attribute(stanza, "from")), ctx)};

    xmpp_stanza_t* event
    {xmpp_stanza_get_child_by_ns(stanza, "http://jabber.org/protocol/pubsub#event")};

    if(event)
    {
        if(makeString(xmpp_stanza_get_name(event)) == "event")
        {
            handlePubsubEvent(XmlElement {event}, from);
        }
    }
}

void StanzaDispatcher::handleIq(const XmlElement& iq)
{
    const auto& makeString = Util::makeString;

    xmpp_stanza_t* const stanza {iq.getStanzaPtr()};
    xmpp_ctx_t* const ctx {stanza->ctx};

    std::string type {makeString(xmpp_stanza_get_type(stanza))};
    std::string id {makeString(xmpp_stanza_get_id(stanza))};
    Jid from {Util::makeJid(makeString(xmpp_stanza_get_attribute(stanza, "from")), ctx)};

    if(type == "set")
    {
        if(not id.empty())
        {
            handleIqSet(iq, from, id);
            return;
        }
    }

    else if(type == "result")
    {
        if(not id.empty())
        {
            handleIqResult(iq, from, id);
        }
        
        else
        {
            // we're ignoring iq results without id

            // TODO:
            // log warning
            std::cout << "WARNING: received iq result without id" << std::endl;
        }

        return;
    }

    else if(type == "error")
    {
        if(not id.empty())
        {
            handleIqError(iq, from, id);
        }
        
        else
        {
            // we're ignoring iq errors without id
            
            // TODO:
            // log warning
            std::cout << "WARNING: received iq error without id" << std::endl;
        }

        return;
    }

    handleInvalidStanza(iq,
                        "modify",
                        "bad-request",
                        "urn:ietf:params:xml:ns:xmpp-stanzas");
}

void StanzaDispatcher::handlePubsubEvent(const XmlElement& event,
                                         const Jid& from)
{
    using Type = InPacket::Type;
    const auto& makeString = Util::makeString;
    
    xmpp_stanza_t* stanzaPtr {event.getStanzaPtr()};

    xmpp_stanza_t* items {xmpp_stanza_get_child_by_name(stanzaPtr, "items")};

    if(items)
    {
        std::string node {xmpp_stanza_get_attribute(items, "node")};

        xmpp_stanza_t* item {xmpp_stanza_get_child_by_name(items, "item")};

        if(item)
        {
            xmpp_stanza_t* notification
            {xmpp_stanza_get_child_by_ns(item, "urn:xmpp:push:0")};

            if(notification)
            {
                if(makeString(xmpp_stanza_get_name(notification)) == "notification")
                {
                    xmpp_stanza_t* xdata
                    {xmpp_stanza_get_child_by_ns(notification, "jabber:x:data")};

                    if(xdata and makeString(xmpp_stanza_get_name(xdata)) == "x")
                    {
                        XData parsedXData {Util::parseXData(XmlElement {xdata})};

                        dispatch(
                            InStanza<Type::PushNotification>
                            {
                                getStanzaHandler<Type::PushNotification>(),
                                from,
                                node,
                                parsedXData
                            }
                        );
                    }
                }
            }
        }
    }
}

void StanzaDispatcher::handleIqSet(const XmlElement& iq,
                                   const Jid& from,
                                   const std::string& id)
{
    using Type = InPacket::Type;
    const auto& makeString = Util::makeString;
    
    xmpp_stanza_t* stanza {iq.getStanzaPtr()};

    xmpp_stanza_t* command
    {
        xmpp_stanza_get_child_by_ns(stanza, "http://jabber.org/protocol/commands")
    };

    if(command)
    {
        std::string name {makeString(xmpp_stanza_get_name(command))};
        std::string action {makeString(xmpp_stanza_get_attribute(command, "action"))};
        std::string node {makeString(xmpp_stanza_get_attribute(command, "node"))};

        if(name == "command")
        {
            xmpp_stanza_t* xdata
            {xmpp_stanza_get_child_by_ns(command, "jabber:x:data")};

            XData parsedXData;

            if(xdata and makeString(xmpp_stanza_get_name(xdata)) == "x")
            {
                try
                {
                    parsedXData = Util::parseXData(XmlElement {xdata});
                }
                catch(std::runtime_error)
                {
                    handleInvalidStanza(iq,
                                        "modify",
                                        "not-acceptable",
                                        "urn:ietf:params:xml:ns:xmpp-stanzas");

                    return;
                }
            }

            dispatch(
                InStanza<Type::AdhocCommand>
                {
                    getStanzaHandler<Type::AdhocCommand>(),
                    from,
                    id,
                    action,
                    node,
                    parsedXData
                }
            );

            return;
        }
    }
    
    handleInvalidStanza(iq,
                        "modify",
                        "bad-request",
                        "urn:ietf:params:xml:ns:xmpp-stanzas");
}

void StanzaDispatcher::handleIqResult(const XmlElement&,
                                      const Jid& from,
                                      const std::string& id)
{
    using Type = InPacket::Type;

    // DEBUG:
    std::cout << "handleIq: dispatching IqResult" << std::endl;

    dispatch(
        InStanza<Type::IqResult>
        {
            getStanzaHandler<Type::IqResult>(),
            from,
            id
        }
    );
}

void StanzaDispatcher::handleIqError(const XmlElement& iq,
                                     const Jid& from,
                                     const std::string& id)
{
    using Type = InPacket::Type;
    const auto& makeString = Util::makeString;
 
    xmpp_stanza_t* stanza {iq.getStanzaPtr()};

    xmpp_stanza_t* error
    {xmpp_stanza_get_child_by_name(stanza, "error")};

    if(error)
    {
        std::string type {makeString(xmpp_stanza_get_type(error))};
        std::vector<std::string> errors;

        xmpp_stanza_t* child {xmpp_stanza_get_children(error)};

        while(child)
        {
            errors.emplace_back(makeString(xmpp_stanza_get_name(child)));

            child = xmpp_stanza_get_next(child);
        }

        dispatch(
            InStanza<Type::IqError>
            {
                getStanzaHandler<Type::IqError>(),
                from,
                id,
                type,
                errors
            }
        );
    }
}

void StanzaDispatcher::handleInvalidStanza(const XmlElement& iq,
                                           const std::string& errorType,
                                           const std::string& condition,
                                           const std::string& ns,
                                           const std::string& appSpecificCondition,
                                           const std::string& appSpecificNs)
{
    using Type = InPacket::Type;

    // DEBUG:
    std::cout << "DEBUG: in handleInvalidStanza" << std::endl;

    XmlElement error
    {
        Util::makeStanzaError(iq,
                              errorType,
                              condition,
                              ns,
                              appSpecificCondition,
                              appSpecificNs)
    };

    dispatch(InStanza<Type::Invalid> {getStanzaHandler<Type::Invalid>(), error});
}

void StanzaDispatcher::dispatch(const InPacket& packet)
{
    if(packet.hasHandler())
    {
        packet.callHandler();
    }
}
