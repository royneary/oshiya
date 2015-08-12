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

#include "OutPacket.hpp"
#include "XmppUtils.hpp"

using namespace Oshiya;

XmlElement
OutStanza<OutPacket::Type::CreatePubsubNode>::makeXmlElement(xmpp_ctx_t* ctx) const
{
    xmpp_stanza_t* iq {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* pubsub {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* create {xmpp_stanza_new(ctx)};

    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_type(iq, "set");
    xmpp_stanza_set_attribute(iq, "from", from.full().c_str());
    xmpp_stanza_set_attribute(iq, "to", to.full().c_str());
    xmpp_stanza_set_id(iq, id.c_str());

    xmpp_stanza_set_name(pubsub, "pubsub");
    xmpp_stanza_set_ns(pubsub, "http://jabber.org/protocol/pubsub");
    xmpp_stanza_add_child(iq, pubsub);
    
    xmpp_stanza_set_name(create, "create");
    xmpp_stanza_set_attribute(create, "node", node.c_str());
    xmpp_stanza_add_child(pubsub, create);

    if(not nodeConfig.empty())
    {
        xmpp_stanza_t* configure {xmpp_stanza_new(ctx)};
        xmpp_stanza_t* xdata
        {xmpp_stanza_clone(Util::makeXDataElement(nodeConfig, ctx).getStanzaPtr())};

        xmpp_stanza_set_name(configure, "configure");
        xmpp_stanza_add_child(configure, xdata);
        xmpp_stanza_add_child(pubsub, configure);
    }

    XmlElement ret {iq};
    xmpp_stanza_release(iq);

    return ret;
}

XmlElement
OutStanza<OutPacket::Type::DeletePubsubNode>::makeXmlElement(xmpp_ctx_t* ctx) const
{
    xmpp_stanza_t* iq {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* pubsub {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* del {xmpp_stanza_new(ctx)};

    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_type(iq, "set");
    xmpp_stanza_set_attribute(iq, "from", from.full().c_str());
    xmpp_stanza_set_attribute(iq, "to", to.full().c_str());
    xmpp_stanza_set_id(iq, id.c_str());

    xmpp_stanza_set_name(pubsub, "pubsub");
    xmpp_stanza_set_ns(pubsub, "http://jabber.org/protocol/pubsub#owner");
    xmpp_stanza_add_child(iq, pubsub);

    xmpp_stanza_set_name(del, "delete");
    xmpp_stanza_set_attribute(del, "node", node.c_str());
    xmpp_stanza_add_child(pubsub, del);

    XmlElement ret {iq};
    xmpp_stanza_release(iq);

    return ret;
}

XmlElement
OutStanza<OutPacket::Type::SetPubsubAffiliation>::makeXmlElement(xmpp_ctx_t* ctx) const
{
    xmpp_stanza_t* iq {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* pubsub {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* affiliations {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* affil {xmpp_stanza_new(ctx)};
    
    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_type(iq, "set");
    xmpp_stanza_set_attribute(iq, "from", from.full().c_str());
    xmpp_stanza_set_attribute(iq, "to", to.full().c_str());
    xmpp_stanza_set_id(iq, id.c_str());

    xmpp_stanza_set_name(pubsub, "pubsub");
    xmpp_stanza_set_ns(pubsub, "http://jabber.org/protocol/pubsub#owner");
    xmpp_stanza_add_child(iq, pubsub);

    xmpp_stanza_set_name(affiliations, "affiliations");
    xmpp_stanza_set_attribute(affiliations, "node", node.c_str());
    xmpp_stanza_add_child(pubsub, affiliations);

    xmpp_stanza_set_name(affil, "affiliation");
    xmpp_stanza_set_attribute(affil, "jid", jid.full().c_str());
    xmpp_stanza_set_attribute(affil, "affiliation", affiliation.c_str());
    xmpp_stanza_add_child(affiliations, affil);

    XmlElement ret {iq};
    xmpp_stanza_release(iq);

    return ret;
}

XmlElement
OutStanza<OutPacket::Type::PubsubSubscribe>::makeXmlElement(xmpp_ctx_t* ctx) const
{
    xmpp_stanza_t* iq {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* pubsub {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* subscribe {xmpp_stanza_new(ctx)};

    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_type(iq, "set");
    xmpp_stanza_set_attribute(iq, "from", from.full().c_str());
    xmpp_stanza_set_attribute(iq, "to", to.full().c_str());
    xmpp_stanza_set_id(iq, id.c_str());

    xmpp_stanza_set_name(pubsub, "pubsub");
    xmpp_stanza_set_ns(pubsub, "http://jabber.org/protocol/pubsub");
    xmpp_stanza_add_child(iq, pubsub);

    xmpp_stanza_set_name(subscribe, "subscribe");
    xmpp_stanza_set_attribute(subscribe, "node", node.c_str());
    xmpp_stanza_set_attribute(subscribe, "jid", from.full().c_str());
    xmpp_stanza_add_child(pubsub, subscribe);

    XmlElement ret {iq};
    xmpp_stanza_release(iq);

    return ret;
}

XmlElement
OutStanza<OutPacket::Type::CommandCompleted>::makeXmlElement(xmpp_ctx_t* ctx) const
{
    xmpp_stanza_t* iq {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* command {xmpp_stanza_new(ctx)};

    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_type(iq, "result");
    xmpp_stanza_set_attribute(iq, "from", from.full().c_str());
    xmpp_stanza_set_attribute(iq, "to", to.full().c_str());
    xmpp_stanza_set_id(iq, id.c_str());

    xmpp_stanza_set_name(command, "command");
    xmpp_stanza_set_ns(command, "http://jabber.org/protocol/commands");
    xmpp_stanza_set_attribute(command, "node", node.c_str());
    xmpp_stanza_set_attribute(command, "status", "completed");
    xmpp_stanza_add_child(iq, command);

    if(not payload.empty())
    {
        xmpp_stanza_t* xdata
        {xmpp_stanza_clone(Util::makeXDataElement(payload, ctx).getStanzaPtr())};

        xmpp_stanza_add_child(command, xdata);
    }

    XmlElement ret {iq};
    xmpp_stanza_release(iq);

    return ret;
}

XmlElement
OutStanza<OutPacket::Type::CommandError>::makeXmlElement(xmpp_ctx_t* ctx) const
{
    xmpp_stanza_t* iq {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* command {xmpp_stanza_new(ctx)};

    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_attribute(iq, "from", from.full().c_str());
    xmpp_stanza_set_attribute(iq, "to", to.full().c_str());
    xmpp_stanza_set_id(iq, id.c_str());

    xmpp_stanza_set_name(command, "command");
    xmpp_stanza_set_ns(command, "http://jabber.org/protocol/commands");
    xmpp_stanza_set_attribute(command, "node", node.c_str());
    xmpp_stanza_set_attribute(command, "action", action.c_str());
    xmpp_stanza_add_child(iq, command);

    XmlElement ret {iq};
    xmpp_stanza_release(iq);

    Util::addErrorToStanza(ret,
                           errorType,
                           condition,
                           "urn:ietf:params:xml:ns:xmpp-stanzas",
                           appSpecificCondition,
                           "http://jabber.org/protocol/commands");
    
    return ret;
}
