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

#ifndef OSHIYA_JID__H
#define OSHIYA_JID__H

#include <string>

namespace Oshiya
{
    class Jid
    {
        public:
        ///////

        Jid() = default;

        Jid(const std::string& user,
            const std::string& server,
            const std::string& resource)
            : mUser{user}, mServer{server}, mResource{resource}
        { }

        bool operator==(const Jid& j) const
        { 
            return
            j.mUser == mUser and j.mServer == mServer and j.mResource == mResource;
        }

        bool operator!=(const Jid& j) const
        {
            return not (*this == j);
        }

        std::string getUser() const { return mUser; }
        std::string getServer() const { return mServer; }
        std::string getResource() const { return mResource; }

        void setUser(const std::string& user) {mUser = user;}
        void setServer(const std::string& server) {mServer = server;}
        void setResource(const std::string& resource) {mResource = resource;}

        std::string bare() const
        { return mUser.empty() ? mServer : mUser + '@' + mServer; }

        std::string full() const
        { return mResource.empty() ? bare() : bare() + '/' + mResource; }

        static Jid removeResource(const Jid& jid)
        {
            return Jid {jid.mUser, jid.mServer, ""};
        }

        bool isValid()
        { return not mServer.empty(); }

        private:
        ////////

        std::string mUser;
        std::string mServer;
        std::string mResource;
    };
}

#endif
