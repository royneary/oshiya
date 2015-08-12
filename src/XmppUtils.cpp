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

#include "XmppUtils.hpp"

extern "C"
{
    char *xmpp_jid_node(xmpp_ctx_t *ctx, const char *jid);
    char *xmpp_jid_domain(xmpp_ctx_t *ctx, const char *jid);
    char *xmpp_jid_resource(xmpp_ctx_t *ctx, const char *jid);
}

using namespace Oshiya;

Jid Util::makeJid(const std::string& str, xmpp_ctx_t* ctx)
{
    char* tmp;
    std::string user, server, resource;

    tmp = xmpp_jid_node(ctx, str.c_str());
    if(tmp)
    {
        user = makeString(tmp);
        xmpp_free(ctx, tmp);
    }

    tmp = xmpp_jid_domain(ctx, str.c_str());
    if(tmp)
    {
        server = makeString(tmp);
        xmpp_free(ctx, tmp);
    }

    tmp = xmpp_jid_resource(ctx, str.c_str());
    if(tmp)
    {
        resource = makeString(tmp);
        xmpp_free(ctx, tmp);
    }

    return Jid{ user, server, resource };
}

std::string Util::makeString(const char* maybeNull)
{
    return std::string {maybeNull ? maybeNull : ""};
}

void Util::addErrorToStanza(XmlElement& stanza,
                            const std::string& errorType,
                            const std::string& condition,
                            const std::string& ns,
                            const std::string& appSpecificCondition,
                            const std::string& appSpecificNs)
{
    xmpp_stanza_t* stanzaPtr {stanza.getStanzaPtr()};
    xmpp_ctx_t* ctx {stanzaPtr->ctx};
    xmpp_stanza_t* errorElem {xmpp_stanza_new(ctx)};
    xmpp_stanza_t* cond {xmpp_stanza_new(ctx)};
    
    xmpp_stanza_set_name(errorElem, "error");
    xmpp_stanza_set_type(errorElem, errorType.c_str());
    xmpp_stanza_set_name(cond, condition.c_str());
    xmpp_stanza_set_ns(cond, ns.c_str());
    xmpp_stanza_add_child(errorElem, cond);

    if(not appSpecificCondition.empty() and not appSpecificNs.empty())
    {
        xmpp_stanza_t* otherCond {xmpp_stanza_new(ctx)};

        xmpp_stanza_set_name(otherCond, appSpecificCondition.c_str());
        xmpp_stanza_set_ns(otherCond, appSpecificNs.c_str());
        xmpp_stanza_add_child(errorElem, otherCond);
    }

    xmpp_stanza_set_attribute(stanzaPtr, "type", "error");
    xmpp_stanza_add_child(stanzaPtr, errorElem);
}

void Util::switchFromTo(XmlElement& stanza)
{
    xmpp_stanza_t* stanzaPtr {stanza.getStanzaPtr()};
    std::string from {makeString(xmpp_stanza_get_attribute(stanzaPtr, "from"))};
    std::string to {makeString(xmpp_stanza_get_attribute(stanzaPtr, "to"))};
    xmpp_stanza_set_attribute(stanzaPtr, "from", to.c_str());
    xmpp_stanza_set_attribute(stanzaPtr, "to", from.c_str());
}

XmlElement Util::makeStanzaError(const XmlElement& stanza,
                                 const std::string& errorType,
                                 const std::string& condition,
                                 const std::string& ns,
                                 const std::string& appSpecificCondition,
                                 const std::string& appSpecificNs)
{
    XmlElement error {stanza};

    addErrorToStanza(error, errorType, condition, ns, appSpecificCondition, appSpecificNs);

    switchFromTo(error);
    
    return error;
}

XData Util::parseXData(const XmlElement& el)
{
    xmpp_stanza_t* elPtr {el.getStanzaPtr()};
    xmpp_stanza_t* field {xmpp_stanza_get_children(elPtr)};  

    std::string type {makeString(xmpp_stanza_get_type(elPtr))};
     
    XData ret {type};

    while(field)
    {
        if(makeString(xmpp_stanza_get_name(field)) == "field")
        {
            std::string fieldType {makeString(xmpp_stanza_get_type(field))};
            std::string var {makeString(xmpp_stanza_get_attribute(field, "var"))};

            std::vector<std::string> values;

            xmpp_stanza_t* value {xmpp_stanza_get_children(field)};

            while(value)
            {
                if(makeString(xmpp_stanza_get_name(value)) == "value")
                {
                    char* text {xmpp_stanza_get_text(value)};
                    values.emplace_back(makeString(text));
                    xmpp_free(value->ctx, text);
                }

                value = xmpp_stanza_get_next(value);
            }

            XData::Field newField {fieldType, var, values};

            if(not newField.isValid())
            {
                throw std::runtime_error {""};
            }

            ret.addField(newField);
        }

        field = xmpp_stanza_get_next(field);
    }

    return ret;
}

XmlElement Util::makeXDataElement(const XData& xdata, xmpp_ctx_t* ctx)
{
    xmpp_stanza_t* x {xmpp_stanza_new(ctx)};

    xmpp_stanza_set_name(x, "x");
    xmpp_stanza_set_ns(x, "jabber:x:data");
    
    std::string formType {xdata.getType()};

    if(not formType.empty())
    {
        xmpp_stanza_set_type(x, formType.c_str());
    }

    auto addFields =
    [ctx](const std::vector<XData::Field>& fields, xmpp_stanza_t* parent)
    {
        for(const XData::Field& f : fields)
        {
            xmpp_stanza_t* field {xmpp_stanza_new(ctx)};
            
            xmpp_stanza_set_name(field, "field");
            xmpp_stanza_set_attribute(field, "var", f.var.c_str());

            const std::string& fieldType {f.type};

            if(not fieldType.empty())
            {
                xmpp_stanza_set_type(field, fieldType.c_str());
            }

            xmpp_stanza_add_child(parent, field);

            for(const std::string& v : f.values)
            {
                xmpp_stanza_t* value {xmpp_stanza_new(ctx)};
                xmpp_stanza_t* text {xmpp_stanza_new(ctx)};

                xmpp_stanza_set_name(value, "value");
                xmpp_stanza_add_child(field, value);

                xmpp_stanza_set_text(text, v.c_str());
                xmpp_stanza_add_child(value, text);
            }
        }
    };

    std::vector<XData::Field> fields {xdata.getFields()};
   
    if(not fields.empty())
    {
        addFields(fields, x);
        //for(const XData::Field& f : xdata.getFields())
        //{
        //    xmpp_stanza_t* field {xmpp_stanza_new(ctx)};

        //    xmpp_stanza_set_name(field, "field");
        //    xmpp_stanza_set_attribute(field, "var", f.var.c_str());
        //    
        //    const std::string& fieldType {f.type};

        //    if(not fieldType.empty())
        //    {
        //        xmpp_stanza_set_type(field, fieldType.c_str());
        //    }

        //    xmpp_stanza_add_child(x, field);

        //    for(const std::string& v : f.values)
        //    {
        //        xmpp_stanza_t* value {xmpp_stanza_new(ctx)};
        //        xmpp_stanza_t* text {xmpp_stanza_new(ctx)};

        //        xmpp_stanza_set_name(value, "value");
        //        xmpp_stanza_add_child(field, value);

        //        xmpp_stanza_set_text(text, v.c_str());
        //        xmpp_stanza_add_child(value, text);
        //    }
        //}
    }

    else
    {
        for(const XData::Item& i : xdata.getItems())
        {
            xmpp_stanza_t* item {xmpp_stanza_new(ctx)};
            
            xmpp_stanza_set_name(item, "item");
            
            xmpp_stanza_add_child(x, item);

            addFields(i.getFields(), item);
        }
    }

    XmlElement ret {x};
    xmpp_stanza_release(x);

    return x;
}
