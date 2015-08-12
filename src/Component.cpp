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

#include <Component.hpp>

#include <functional>

// DEBUG:
#include <iostream>
#include <algorithm>
#include <cstring>

using namespace Oshiya;

Component::Component(const Config& config)
    :
        mConfig {config},
        mLogger {xmpp_get_default_logger(XMPP_LEVEL_DEBUG)},
        mContext {xmpp_ctx_new(nullptr, mLogger)},
        mConnection {xmpp_conn_new(mContext)},
        mJid {makeJid(mConfig.value("host"))},
        mServerJid {makeJid(mConfig.value("server_host"))},
        mPort {mConfig.value<unsigned short>("port")},
        mPubsubJid {makeJid(mConfig.value("pubsub_host"))},
        mStanzaDispatcher { },
        mShutdown {false}
{
    using Type = InPacket::Type;
    using namespace std::placeholders;

    xmpp_initialize();

    const char* jid {mJid.full().c_str()};
    const char* password {config.value("password").c_str()};

    // DEBUG:
    std::cout << "setting component jid: " << jid << std::endl;
    std::cout << "setting component password: " << password << std::endl;

    xmpp_conn_set_jid(mConnection, jid);
    xmpp_conn_set_pass(mConnection, password);

    mStanzaDispatcher.addHandler(Type::AdhocCommand,
                                 std::bind(&Component::_commandReceived, this, _1));
    mStanzaDispatcher.addHandler(Type::IqResult,
                                 std::bind(&Component::_iqResultReceived, this, _1));
    mStanzaDispatcher.addHandler(Type::IqError,
                                 std::bind(&Component::_iqErrorReceived, this, _1));
    mStanzaDispatcher.addHandler(Type::PushNotification,
                                 std::bind(&Component::_pushNotificationReceived, this, _1));
    mStanzaDispatcher.addHandler(Type::Invalid,
                                 std::bind(&Component::_invalidStanzaReceived, this, _1));
}

Component::~Component()
{
    // DEBUG:
    std::cout << "in Component dtor" << std::endl;

    if(mXmppThread.get_id() != std::thread::id {})
    {
        mXmppThread.join();
    }

    xmpp_conn_release(mConnection);

    xmpp_ctx_free(mContext);

    xmpp_shutdown();
}

void Component::connect()
{
    // DEBUG:
    std::cout << "connnecting, host: " << mServerJid.full() << ", port: " << mPort << std::endl;

    mXmppThread = std::thread {&Component::run, this}; 
}

void Component::sendPacket(std::unique_ptr<OutPacket>&& packet)
{
    std::lock_guard<std::mutex> lock {mOutPacketsMutex};
    mOutPackets.emplace(std::forward<std::unique_ptr<OutPacket>>(packet));
}

std::string Component::makeRandomString(std::size_t length)
{
    return mRNG.getRandomText(length);
}

Jid Component::makeJid(const std::string& str)
{
    return Util::makeJid(str, mContext);
}

void Component::shutdown()
{
    mShutdown = true;
}

void Component::run()
{
    while(true)
    {
        xmpp_connect_component(mConnection,
                               mServerJid.full().c_str(),
                               mPort,
                               connHandler,
                               this);

        if(mContext->loop_status != XMPP_LOOP_NOTSTARTED)
        {
            // DEBUG:
            std::cout << "returning from Component::run" << std::endl;
            return;
        }

        mContext->loop_status = XMPP_LOOP_RUNNING;

        while(mContext->loop_status == XMPP_LOOP_RUNNING)
        {
            {
                std::lock_guard<std::mutex> lock {mOutPacketsMutex};

                while(not mOutPackets.empty())
                {
                    XmlElement stanza
                    {mOutPackets.front().get()->makeXmlElement(mContext)};

                    mOutPackets.pop();
                    xmpp_send(mConnection, stanza.getStanzaPtr());
                }
            }

            if(mShutdown)
            {
                xmpp_disconnect(mConnection);
                return;
            }

            xmpp_run_once(mContext, Parameters::StropheLoopTimeout);
        }

        mContext->loop_status = XMPP_LOOP_NOTSTARTED;

        std::this_thread::sleep_for(
            std::chrono::milliseconds(Parameters::ReconnectInterval)
        );
    }
}

void Component::connHandler(xmpp_conn_t* const conn,
                            const xmpp_conn_event_t status,
                            const int error,
                            xmpp_stream_error_t* const streamError,
                            void* const userData)
{
    Component* comp {static_cast<Component*>(userData)};

    if(status == XMPP_CONN_CONNECT)
    {
        // DEBUG:
        std::cout << "Component::connHandler: component connected!" << std::endl;

        xmpp_handler_add(conn,
                         handleIq,
                         nullptr,
                         "iq",
                         nullptr,
                         comp);

        xmpp_handler_add(conn,
                         handleMessage,
                         nullptr,
                         "message",
                         nullptr,
                         comp);
    }
    else
    {
        // DEBUG:
        std::cout << "Component::connHandler: component disconnected!" << std::endl;
        conn->error = 0; // in order to reconnect we need to reset the error flag
        xmpp_stop(comp->mContext);
    }
}

void Component::createPubsubNode(const std::string& id,
                                 const std::string& node,
                                 const XData& nodeConfig)
{
    sendPacket(
        make_unique<OutStanza<OutPacket::Type::CreatePubsubNode>>
        (
            mJid,
            mPubsubJid,
            id,
            node,
            nodeConfig
        )
    );
}

void Component::deletePubsubNode(const std::string& id,
                                 const std::string& node)
{
    sendPacket(
        make_unique<OutStanza<OutPacket::Type::DeletePubsubNode>>
        (
            mJid,
            mPubsubJid,
            id,
            node
        )
    );
}

void Component::setPubsubAffiliation(const std::string& id,
                                     const std::string& node,
                                     const Jid& jid,
                                     const std::string& affiliation)
{
    sendPacket(
        make_unique<OutStanza<OutPacket::Type::SetPubsubAffiliation>>
        (
            mJid,
            mPubsubJid,
            id,
            node,
            jid,
            affiliation
        )
    );
}

void Component::pubsubSubscribe(const std::string& id,
                                const std::string& node)
{
    sendPacket(
        make_unique<OutStanza<OutPacket::Type::PubsubSubscribe>>
        (
            mJid,
            mPubsubJid,
            id,
            node
        )
    );
}

void Component::sendCommandCompleted(const Jid& to,
                                     const std::string& id,
                                     const std::string& node,
                                     const XData& payload)
{
    sendPacket(
        make_unique<OutStanza<OutPacket::Type::CommandCompleted>>
        (
            mJid,
            to,
            id,
            node,
            payload
        )
    );
}

void Component::sendCommandError(const Jid& to,
                                 const std::string& id,
                                 const std::string& node,
                                 const std::string& action,
                                 const std::string& errorType,
                                 const std::string& condition,
                                 const std::string& appSpecificCondition)
{
    sendPacket(
        make_unique<OutStanza<OutPacket::Type::CommandError>>
        (
            mJid,
            to,
            id,
            node,
            action,
            errorType,
            condition,
            appSpecificCondition
        )
    );
}

void Component::_commandReceived(const InPacket& packet)
{
    const AdhocCommand& stanza = static_cast<const AdhocCommand&>(packet);

    commandReceived(stanza.from, stanza.id, stanza.action, stanza.node, stanza.payload);
}

void Component::_iqResultReceived(const InPacket& packet)
{
    const IqResult& stanza = static_cast<const IqResult&>(packet);

    iqResultReceived(stanza.from, stanza.id);
}

void Component::_iqErrorReceived(const InPacket& packet)
{
    const IqError& stanza = static_cast<const IqError&>(packet);

    iqErrorReceived(stanza.from, stanza.id, stanza.errorType, stanza.errors);
}

void Component::_pushNotificationReceived(const InPacket& packet)
{
    const PushNotification& stanza = static_cast<const PushNotification&>(packet);

    pushNotificationReceived(stanza.from, stanza.node, stanza.payload);
}

void Component::_invalidStanzaReceived(const InPacket& packet)
{
    const Invalid& stanza = static_cast<const Invalid&>(packet);

    if(stanza.stanzaError.isValid())
    {
        xmpp_send(mConnection, stanza.stanzaError.getStanzaPtr());
    }
}

int Component::handleIq(xmpp_conn_t* const conn,
                        xmpp_stanza_t* const stanza,
                        void* const userdata)
{
    Component* compObj {static_cast<Component*>(userdata)};

    compObj->mStanzaDispatcher.handleIq(XmlElement {stanza});

    return 1;
}

int Component::handleMessage(xmpp_conn_t* const conn,
                             xmpp_stanza_t* const stanza,
                             void* const userdata)
{
    Component* compObj {static_cast<Component*>(userdata)};

    compObj->mStanzaDispatcher.handleMessage(XmlElement {stanza});

    return 1;
}
