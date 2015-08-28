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

#ifndef OSHIYA_APP_SERVER__H
#define OSHIYA_APP_SERVER__H

#include <Component.hpp>
#include <ApnsBackend.hpp>
#include <GcmBackend.hpp>
//#include <MozillaBackend.hpp>
#include <UbuntuBackend.hpp>
//#include <WnsBackend.hpp>
#include "Registration.hpp"
#include "config.h"

#include <map>
#include <queue>
#include <algorithm>
#include <fstream>

namespace Oshiya
{
    class AppServer : public Component
    {
        public:
        ///////

        AppServer(const Config& config);

        AppServer(const AppServer&) = delete;
        AppServer(AppServer&&) = delete;

        ~AppServer();

        private:
        ////////

        using NodeIdT = std::string;
        using StanzaIdT = std::string;

        struct PendingReg
        {
            enum Action
            {
                CreateNode = 1 << 0,
                SetAffiliation = 1 << 1,
                Subscribe = 1 << 2
            };

            PendingReg(const std::string& secret,
                       const std::string& registerId,
                       const Registration& registration)
                :
                    mCompletedActions {0},
                    mSecret {secret},
                    mRegisterId {registerId},
                    mRegistration {registration}
            { }

            int completeAction(Action action) {return mCompletedActions |= action;}
            int getCompletedActions() const {return mCompletedActions;}
            std::string getSecret() const {return mSecret;}
            std::string getRegisterId() const {return mRegisterId;}
            Registration& getRegistration() {return mRegistration;}
            const Registration& getRegistration() const {return mRegistration;}

            private:
            ////////

            int mCompletedActions;
            const std::string mSecret;
            const std::string mRegisterId;
            Registration mRegistration;
        };
        
        void commandReceived(const Jid& from,
                             const std::string& id,
                             const std::string& action,
                             const std::string& node,
                             const XData& payload) override;

        void iqResultReceived(const Jid& from,
                              const std::string& id) override;

        void iqErrorReceived(const Jid& from,
                             const std::string& id,
                             const std::string& errorType,
                             const std::vector<std::string>& errors) override;

        void pushNotificationReceived(const Jid& from,
                                      const std::string& node,
                                      const XData& payload) override;

        void addRegistration(const Jid& user,
                             const std::string& stanzaId,
                             const std::string& node,
                             const XData& payload);

        void deleteRegistrations(const Jid& user,
                                 const std::string& stanzaId,
                                 const XData& payload);

        template <typename UnaryPredicate>
        bool deleteRegistration(const std::string& node, UnaryPredicate pred);

        void deleteRegCb(const std::string& node, std::time_t timestamp);

        std::unordered_map<Backend::IdT, std::unique_ptr<Backend>> makeBackends();

        std::unique_ptr<Backend> makeBackendPtr(Backend::Type type,
                                                const Jid& host,
                                                const std::string& appName,
                                                const std::string& certFile,
                                                const std::string& authKey);

        Backend::Type getRegType(const Registration& reg);

        static std::size_t makeDeviceHash(const Jid& user, const std::string& deviceId);

        std::unordered_map<NodeIdT, Registration> readRegs() const;
        void writeRegs() const;

        std::unordered_map<Backend::IdT, std::unique_ptr<Backend>> mBackends;
        std::unordered_map<NodeIdT, Registration> mRegs;
        std::unordered_map<NodeIdT, PendingReg> mPendingRegs;
        std::unordered_map<StanzaIdT, std::pair<PendingReg::Action, NodeIdT>>
        mPendingActions;
        std::mutex mRegsMutex;
    };
}

#endif
