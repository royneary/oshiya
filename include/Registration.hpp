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

#ifndef OSHIYA_REGISTRATION__H
#define OSHIYA_REGISTRATION__H

#include "Backend.hpp"

#include <ctime>
#include <iostream>

namespace Oshiya
{
    class Registration
    {
        public:
        ///////

        Registration() = default;

        Registration(const Jid& user,
                     const std::string& deviceId,
                     const std::string& deviceName,
                     const std::string& token,
                     const std::string& appId,
                     Backend::IdT backendId,
                     std::time_t timestamp = std::time(nullptr));

        Jid getUser() const {return mUser;}
        std::string getDeviceId() const {return mDeviceId;}
        std::string getDeviceName() const {return mDeviceName;}
        std::string getToken() const {return mToken;}
        std::string getAppId() const {return mAppId;}
        Backend::IdT getBackendId() const {return mBackendId;}
        std::time_t getTimestamp() const {return mTimestamp;}

        void setUser(const Jid& jid) {mUser = jid;}
        void setDeviceId(const std::string& deviceId) {mDeviceId = deviceId;}
        void setDeviceName(const std::string& deviceName) {mDeviceName = deviceName;}
        void setToken(const std::string& token) {mToken = token;}
        void setAppId(const std::string& appId) {mAppId = appId;}
        void setBackendId(Backend::IdT backendId) {mBackendId = backendId;}
        void setTimestamp(std::time_t timestamp) {mTimestamp = timestamp;}

        private:
        ////////
        
        Jid mUser;
        std::string mDeviceId;
        std::string mDeviceName;
        std::string mToken;
        std::string mAppId;
        Backend::IdT mBackendId;
        std::time_t mTimestamp;
    };

    namespace
    {
        /**
         * serialization / deserialization functions
         */
        std::ostream& operator<<(std::ostream& os, const Registration& reg)
        {
            Jid user {reg.getUser()};
       
            os << user.getUser() << '\n'
               << user.getServer() << '\n'
               << user.getResource() << '\n'
               << reg.getDeviceId() << '\n'
               << reg.getDeviceName() << '\n'
               << reg.getToken() << '\n'
               << reg.getAppId() << '\n'
               << reg.getBackendId() << '\n'
               << reg.getTimestamp() << '\n';
            
            return os;
        }

        std::istream& operator>>(std::istream& is, Registration& reg)
        {
            std::string user, server, resource, deviceId, deviceName, token, appId;
            Backend::IdT backendId;
            std::time_t timestamp;
      
            std::getline(is, user);
            std::getline(is, server);
            std::getline(is, resource);
            std::getline(is, deviceId);
            std::getline(is, deviceName);
            std::getline(is, token);
            std::getline(is, appId);
            is >> backendId;
            is.ignore(1);
            is >> timestamp;
            is.ignore(1);

            reg.setUser({user, server, resource});
            reg.setDeviceId(deviceId);
            reg.setDeviceName(deviceName);
            reg.setToken(token);
            reg.setAppId(appId);
            reg.setBackendId(backendId);
            reg.setTimestamp(timestamp);
        
            return is;
        }
    }
}

#endif
