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

#ifndef OSHIYA_XML_ELEMENT__H
#define OSHIYA_XML_ELEMENT__H

extern "C"
{
    #include "strophe.h"
}

namespace Oshiya
{
    class XmlElement
    {
        public:
        ///////
     
        XmlElement();

        XmlElement(xmpp_stanza_t* el);
        //XmlElement(XmlElement&& other);
        XmlElement(const XmlElement& other);

        //XmlElement& operator=(XmlElement&& other);
        XmlElement& operator=(const XmlElement& other);

        ~XmlElement();

        xmpp_stanza_t* getStanzaPtr() const { return mStanza; }

        bool isValid() const { return mStanza != nullptr; }

        private:
        ////////

        void free();

        xmpp_stanza_t* mStanza;
    };
}

#endif
