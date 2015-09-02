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

#ifndef OSHIYA_COMPONENT__H
#define OSHIYA_COMPONENT__H

#include "Config.hpp"
#include "Jid.hpp"
#include "StanzaDispatcher.hpp"
#include "InPacket.hpp"
#include "OutPacket.hpp"
#include "XmppUtils.hpp"
#include "SmartPointerUtil.hpp"
#include "RNG.hpp"

#include <thread>
#include <mutex>
#include <queue>
extern "C"
{
    #include "strophe.h"
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
    #include "src/common.h"
    #pragma GCC diagnostic pop
}

namespace Oshiya
{
    class Component
    {
        public:
        ///////

        struct Parameters
        {
            static const int StropheLoopTimeout {1}; // 1 ms
            static const int ReconnectInterval {10000}; // 10000 ms
        };

        Component(const Config& config);

        Component(const Component&) = delete;
        Component(Component&&) = delete;

        Jid makeJid(const std::string& str);

        void createPubsubNode(const std::string& id,
                              const std::string& node,
                              const XData& nodeConfig);

        void deletePubsubNode(const std::string& id,
                              const std::string& node);

        void setPubsubAffiliation(const std::string& id,
                                  const std::string& node,
                                  const Jid& jid,
                                  const std::string& affiliation);

        void pubsubSubscribe(const std::string& id,
                             const std::string& node);

        void sendCommandCompleted(const Jid& to,
                                  const std::string& id,
                                  const std::string& node,
                                  const XData& payload);

        void sendCommandError(const Jid& to,
                              const std::string& id,
                              const std::string& node,
                              const std::string& action,
                              const std::string& errorType,
                              const std::string& condition,
                              const std::string& appSpecificCondition = "");

        void connect();
        
        void shutdown();

        protected:
        //////////

        virtual ~Component();

        void sendPacket(std::unique_ptr<OutPacket>&& packet);

        std::string makeRandomString(std::size_t length = 12);

        Config getConfig() const {return mConfig;}
        Jid getJid() const {return mJid;}
        Jid getServerJid() const {return mServerJid;}
        unsigned short getPort() const {return mPort;}
        Jid getPubsubJid() const {return mPubsubJid;}

        virtual void commandReceived(const Jid& from,
                                     const std::string& id,
                                     const std::string& action,
                                     const std::string& node,
                                     const XData& payload) = 0;

        virtual void iqResultReceived(const Jid& from,
                                      const std::string& id) = 0;

        virtual void iqErrorReceived(const Jid& from,
                                     const std::string& id,
                                     const std::string& errorType,
                                     const std::vector<std::string>& errors) = 0;

        virtual void pushNotificationReceived(const Jid& from,
                                              const std::string& node,
                                              const XData& payload) = 0;

        private:
        ////////

        using AdhocCommand = InStanza<InPacket::Type::AdhocCommand>;
        using IqResult = InStanza<InPacket::Type::IqResult>;
        using IqError = InStanza<InPacket::Type::IqError>;
        using PushNotification = InStanza<InPacket::Type::PushNotification>;
        using Invalid = InStanza<InPacket::Type::Invalid>;

        void run();

        void invalidStanzaReceived(const XmlElement& packet);

        // strophe callbacks
        static int handleIq(xmpp_conn_t* const conn,
                            xmpp_stanza_t* const stanza,
                            void* const userdata);
        
        static int handleMessage(xmpp_conn_t* const conn,
                                 xmpp_stanza_t* const stanza,
                                 void* const userdata);

        static void connHandler(xmpp_conn_t* const conn,
                                const xmpp_conn_event_t status,
                                const int error,
                                xmpp_stream_error_t* const streamError,
                                void* const userData);

        Config mConfig;
        xmpp_log_t* mLogger;
        xmpp_ctx_t* mContext;
        xmpp_conn_t* mConnection;
        Jid mJid;
        Jid mServerJid;
        unsigned short mPort;
        Jid mPubsubJid;
        StanzaDispatcher mStanzaDispatcher;
        std::thread mXmppThread;
        volatile bool mShutdown;
        std::mutex mOutPacketsMutex;
        std::queue<std::unique_ptr<OutPacket>> mOutPackets;
        RNG mRNG;
    };
} // namespace Oshiya

#endif
