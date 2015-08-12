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

#ifndef OSHIYA_BACKEND__H
#define OSHIYA_BACKEND__H

#include "Jid.hpp"

#include <map>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace Oshiya
{
    class Backend
    {
        public:
        ///////

        using IdT = std::size_t;
        using PayloadT = std::map<std::string, std::string>;

        enum class Type
        {
            Apns,
            Gcm,
            Mozilla,
            Ubuntu,
            Wns,
            Invalid
        };

        struct Parameters
        {
            static const unsigned int NotificationExpireTime {60 * 60 * 24};
            static const unsigned int HttpTimeout {10000};
            static const unsigned int ConnectTimeout {10000};
            static const unsigned int RetryPeriod {10000};
            // TODO: ciphersuites
        };

        struct PushNotification
        {
            PushNotification(const PayloadT& _payload,
                             const std::string& _token,
                             const std::string& _appId,
                             const std::function<void()>& _unregisterCb)
                :
                    payload {_payload},
                    token {_token},
                    appId {_appId},
                    unregisterCb {_unregisterCb}
            { }

            const PayloadT payload;
            const std::string token;
            const std::string appId;
            const std::function<void()> unregisterCb;
        };

        Backend(Type _type,
                const Jid& _host,
                const std::string& _appName,
                const std::string& _certFile);

        Backend(const Backend&) = delete;
        Backend(Backend&&) = default;

        virtual ~Backend() = 0;

        const Type type;
        const Jid host;
        const std::string appName;
        const std::string certFile;

        IdT getId();

        static IdT makeBackendId(Type type,
                                 const Jid& host);

        static Type makeType(const std::string& typeStr);
        static std::string getTypeStr(Type type);

        void dispatch(const PayloadT& payload,
                      const std::string& token,
                      const std::string& appId,
                      std::function<void()> unregisterCb);

        void doWork();

        virtual bool send(const PushNotification& notification) = 0;

        private:
        ////////

        bool mShutdown;
        std::mutex mDispatchMutex;
        std::condition_variable mDispatchCv;
        std::queue<PushNotification> mDispatchQueue;
        std::thread mWorkerThread;
    };
}

#endif
