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

#include <AppServer.hpp>

// DEBUG:
#include <iostream>

using namespace Oshiya;

AppServer::AppServer(const Config& config)
    :
        Component {config},
        mBackends {makeBackends()},
        mRegs {readRegs()}
{
    connect();
}

AppServer::~AppServer()
{
    shutdown();
    writeRegs();
}

void AppServer::commandReceived(const Jid& from,
                                const std::string& id,
                                const std::string& action,
                                const std::string& node,
                                const XData& payload)
{
    if(action != "execute")
    {
        sendCommandError(from, id, node, action, "modify", "bad-request", "bad-action");
        return;
    }

    if(node == "list-push-registrations")
    {
        XData xdata {"result"};

        {
            std::lock_guard<std::mutex> lk {mRegsMutex};

            for(const auto& p : mRegs)
            {
                XData::Item item;

                if(p.second.getUser().bare() == from.bare())
                {
                    std::string deviceName {p.second.getDeviceName()};

                    if(not deviceName.empty())
                    {
                        item.addField({"", "device-name", {deviceName}});
                    }

                    item.addField({"", "node", {p.first}});
                }

                xdata.addItem(item);
            }
        }
        
        sendCommandCompleted(from, id, node, xdata);
    }

    else if(node == "unregister-push")
    {
        deleteRegistrations(from, id, payload);
    }

    else
    {
        addRegistration(from, id, node, payload);
    }
}

void AppServer::iqResultReceived(const Jid& from,
                                 const std::string& id)
{
    using Action = typename PendingReg::Action;

    auto result = mPendingActions.find(id);

    if(result != mPendingActions.end())
    {
        Action action {result->second.first};
        NodeIdT& node = result->second.second;

        auto pendingIt = mPendingRegs.find(node);

        /**
         * pending registration might alredy be deleted because of a previous
         * stanza error
         */
        if(pendingIt != mPendingRegs.end())
        {
            PendingReg& pending = pendingIt->second;

            int completedActions {pending.completeAction(action)};

            if(completedActions == Action::CreateNode)
            {
                NodeIdT setAffiliationId {makeRandomString()};
                NodeIdT subscribeId {makeRandomString()};

                mPendingActions.insert(
                    {setAffiliationId, {Action::SetAffiliation, node}}
                );
                mPendingActions.insert(
                    {subscribeId, {Action::Subscribe, node}}
                );

                setPubsubAffiliation(
                    setAffiliationId,
                    node,
                    Jid::removeResource(pending.getRegistration().getUser()),
                    "publish-only"
                );

                pubsubSubscribe(subscribeId, node);
            }

            else if(completedActions ==
               (Action::CreateNode | Action::SetAffiliation | Action::Subscribe))
            {
                XData xdata
                {
                    "result",
                    {
                        {"", "jid", {getPubsubJid().full()}},
                        {"", "node", {node}},
                        {"", "secret", {pending.getSecret()}}
                    }
                };
                
                std::string command
                {
                    "register-push-" +
                    Backend::getTypeStr(getRegType(pending.getRegistration()))
                };

                sendCommandCompleted(pending.getRegistration().getUser(),
                                     pending.getRegisterId(),
                                     command,
                                     xdata);
                
                {
                    std::lock_guard<std::mutex> lk {mRegsMutex};
                    mRegs.emplace(node, pending.getRegistration());
                }

                mPendingRegs.erase(pendingIt);
            }
        }

        mPendingActions.erase(result);
    }
}

void AppServer::iqErrorReceived(const Jid& from,
                                const std::string& id,
                                const std::string& errorType,
                                const std::vector<std::string>& errors)
{
    using Action = typename PendingReg::Action;

    auto result = mPendingActions.find(id);

    if(result != mPendingActions.end())
    {
        Action action {result->second.first};
        NodeIdT& node = result->second.second;

        auto pendingIt = mPendingRegs.find(node);

        /**
         * pending registration might alredy be deleted because of a previous
         * stanza error
         */
        if(pendingIt != mPendingRegs.end())
        {
            const PendingReg& pending = pendingIt->second;

            std::string command
            {
                "register-push-" +
                Backend::getTypeStr(getRegType(pending.getRegistration()))
            };

            if(errorType == "wait")
            {
                sendCommandError(pending.getRegistration().getUser(),
                                 pending.getRegisterId(),
                                 command,
                                 "execute",
                                 "wait",
                                 "resource-contraint");
            }

            else
            {
                sendCommandError(pending.getRegistration().getUser(),
                                 pending.getRegisterId(),
                                 command,
                                 "execute",
                                 "cancel",
                                 "internal-server-error");
            }

            switch(action)
            {
                case Action::CreateNode:
                {
                    // TODO:
                    // log WARNING
                    std::cout << "WARNING: Could not create node" << std::endl;

                    mPendingRegs.erase(pendingIt);
                    break;
                }

                case Action::SetAffiliation:
                {
                    // TODO:
                    // log WARNING
                    std::cout << "WARNING: Could not set affiliation" << std::endl;

                    deletePubsubNode(makeRandomString(), node);
                    mPendingRegs.erase(pendingIt);
                    break;
                }

                case Action::Subscribe:
                {
                    // TODO:
                    // log WARNING
                    std::cout << "WARNING: Could not subscribe to node" << std::endl;

                    deletePubsubNode(makeRandomString(), node);
                    mPendingRegs.erase(pendingIt);
                    break;
                }
            }
        }

    }
}

void AppServer::pushNotificationReceived(const Jid& from,
                                         const std::string& node,
                                         const XData& payload)
{
    std::cout << "pubsub item received!" << std::endl;
    
    if(from != getPubsubJid())
    {
        // TODO: log warning
        std::cout << "WARNING received push notification from unknown pubsub service"
                  << std::endl;

        return;
    }

    auto result = mRegs.find(node);

    if(result == mRegs.end())
    {
        // TODO: log warning
        std::cout << "WARNING received push notifications on unknown node" << std::endl;

        return;
    }

    const Registration& reg = result->second;
    std::time_t timestamp {reg.getTimestamp()};

    Backend::PayloadT backendPayload;

    for(const auto& f : payload.getFields())
    {
        if(f.isValid() and (f.type.empty() or f.type == "text-single"))
        {
            backendPayload.insert({f.var, f.values.front()});
        }
    }

    auto unregisterCb = [this, node, timestamp]() {deleteRegCb(node, timestamp);};

    mBackends.at(reg.getBackendId())->dispatch(
        backendPayload,
        reg.getToken(),
        reg.getAppId(),
        unregisterCb
    );
}

void AppServer::addRegistration(const Jid& user,
                                const std::string& stanzaId,
                                const std::string& node,
                                const XData& payload)
{
    using Action = typename PendingReg::Action;

    std::string deviceId {payload.getField("device-id").singleValue()};
    std::string deviceName {payload.getField("device-name").singleValue()};
    std::string token {payload.getField("token").singleValue()};
    std::string appId {payload.getField("application-id").singleValue()};

    if(deviceId.empty())
    {
        deviceId = user.getResource();
    }

    bool payloadOk;
    Backend::Type backendType;

    if(node == "register-push-apns")
    {
        payloadOk = not (deviceId.empty() or token.empty());
        backendType = Backend::Type::Apns;   
    }

    else if(node == "register-push-gcm")
    {
        payloadOk = not (deviceId.empty() or token.empty());
        backendType = Backend::Type::Gcm;
    }

    else if(node == "register-push-mozilla")
    {
        payloadOk = not (deviceId.empty() or token.empty());
        backendType = Backend::Type::Mozilla;
    }

    else if(node == "register-push-ubuntu")
    {
        payloadOk = not (deviceId.empty() or token.empty() or appId.empty());
        backendType = Backend::Type::Ubuntu;
    }

    else if(node == "register-push-wns")
    {
        payloadOk = not (deviceId.empty() or token.empty());
        backendType = Backend::Type::Wns;
    }

    else
    {
        sendCommandError(user,
                         stanzaId,
                         node,
                         "execute",
                         "modify",
                         "bad-request",
                         "malformed-action");
        return;
    }

    if(not payloadOk)
    {
        sendCommandError(user,
                         stanzaId,
                         node,
                         "execute",
                         "modify",
                         "bad-request",
                         "bad-payload");
        return;
    }

    Backend::IdT backendId {Backend::makeBackendId(backendType, getJid())};
    
    if(mBackends.find(backendId) == mBackends.cend())
    {
        sendCommandError(user, stanzaId, node, "execute", "modify", "item-not-found");
        return;
    }

    std::string secret {makeRandomString(32)};
    
    Registration reg
    {
        user,
        deviceId,
        deviceName,
        token,
        appId,
        backendId,
        std::time(nullptr)
    };

    // DEBUG:
    std::cout << "DEBUG: adding registration:" << std::endl
              << "user: " << reg.getUser().full() << std::endl
              << "deviceId: " << reg.getDeviceId() << std::endl
              << "deviceName: " << reg.getDeviceName() << std::endl
              << "token: " << reg.getToken() << std::endl
              << "appId: " << reg.getAppId() << std::endl
              << "backendId: " << reg.getBackendId() << std::endl
              << "timestmap: " << reg.getTimestamp() << std::endl;

    auto regPred =
    [&user, &deviceId](const std::pair<NodeIdT, Registration>& p)
    {
        return 
        p.second.getUser().bare() == user.bare() and
        p.second.getDeviceId() == deviceId;
    };

    {
        std::lock_guard<std::mutex> lk {mRegsMutex};

        auto regResult = std::find_if(mRegs.begin(), mRegs.end(), regPred);

        if(regResult != mRegs.end())
        {
            deletePubsubNode(makeRandomString(), regResult->first);
            mRegs.erase(regResult);
        }
    }

    auto pendingPred =
    [&user, &deviceId](const std::pair<StanzaIdT, PendingReg>& p)
    {
        return
        p.second.getRegistration().getUser().bare() == user.bare() and
        p.second.getRegistration().getDeviceId() == deviceId;
    };

    auto pendingResult =
    std::find_if(mPendingRegs.begin(), mPendingRegs.end(), pendingPred);

    if(pendingResult != mPendingRegs.end())
    {
        deletePubsubNode(makeRandomString(), pendingResult->first);
        mPendingRegs.erase(pendingResult); 
    }

    std::string newNode {makeRandomString()};
    std::string createNodeId {makeRandomString()};

    XData nodeConfig
    {
        "submit",
        {
            {"hidden", "FORM_TYPE", {"http://jabber.org/protocol/pubsub#node_config"}},
            {"", "pubsub#secret", {secret}}
        }
    };

    mPendingRegs.insert({newNode, PendingReg {secret, stanzaId, reg}});
    mPendingActions.insert({createNodeId, {Action::CreateNode, newNode}});

    createPubsubNode(createNodeId, newNode, nodeConfig);
}

void AppServer::deleteRegistrations(const Jid& user,
                                    const std::string& stanzaId,
                                    const XData& payload)
{
    XData xdata {"result"};

    std::vector<std::string> nodes
    {
        payload.getField("nodes", "list-multi").values
    };

    if(not nodes.empty())
    {
        for(auto it = nodes.begin(); it != nodes.end(); ++it)
        {
            bool deleted
            {
                deleteRegistration(
                    *it,
                    [&user](const Registration& r)
                    {return r.getUser().bare() == user.bare();}
                )
            };
            
            if(not deleted)
            {
                nodes.erase(it);
            }
        }

        if(not nodes.empty())
        {
            xdata.addField({"list-multi", "nodes", nodes});
        }
    }

    else
    {
        std::string deviceId {payload.getField("device-id").singleValue()};

        if(deviceId.empty())
        {
            deviceId = user.getResource();
        }

        if(deviceId.empty())
        {
            sendCommandError(user,
                             stanzaId,
                             "execute",
                             "modify",
                             "bad-request",
                             "bad-payload");
            return;
        }

        auto pred =
        [&user, &deviceId](const std::pair<NodeIdT, Registration>& p)
        {
            return
            p.second.getUser().bare() == user.bare() and
            p.second.getDeviceId() == deviceId;
        };

        {
            std::lock_guard<std::mutex> lk {mRegsMutex};

            auto result = std::find_if(mRegs.begin(), mRegs.end(), pred);

            if(result == mRegs.end())
            {
                sendCommandError(user,
                                 stanzaId,
                                 "execute",
                                 "modify",
                                 "bad-request",
                                 "bad-payload");
                return;
            }

            deletePubsubNode(makeRandomString(), result->first);
            mRegs.erase(result);
        }
    }

    sendCommandCompleted(user, stanzaId, "unregister-push", xdata);
}

template <typename UnaryPredicate>
bool AppServer::deleteRegistration(const std::string& node, UnaryPredicate pred)
{
    std::lock_guard<std::mutex> lk {mRegsMutex};

    auto result = mRegs.find(node);

    if(result != mRegs.end() and pred(result->second))
    {
        deletePubsubNode(makeRandomString(), node);
        mRegs.erase(result);

        return true;
    }

    return false;
}

void AppServer::deleteRegCb(const std::string& node, std::time_t timestamp)
{
    deleteRegistration(
        node,
        [timestamp](const Registration& r) {return r.getTimestamp() == timestamp;}
    );
}

std::unordered_map<Backend::IdT, std::unique_ptr<Backend>>
AppServer::makeBackends()
{
    Config config {getConfig()};

    Jid host {makeJid(config.value("host"))};

    std::unordered_map<Backend::IdT, std::unique_ptr<Backend>> ret;
    const Config::NodeT backends {config.getNode("backends")};

    for(Config::IteratorT it {backends.begin()}; it != backends.end(); ++it)
    {
        const Config backendConfig {*it};
        
        Backend::Type type {Backend::makeType(backendConfig.value("type"))};
        std::string appName {backendConfig.value("app_name", std::string {"any"})};
        std::string certFile {backendConfig.value("certfile")};
        std::string authKey {backendConfig.value("auth_key", std::string {})};
    
        ret.emplace(
            Backend::makeBackendId(type, host),
            makeBackendPtr(type,
                           host,
                           appName,
                           certFile,
                           authKey)
        );
    }

    return ret;
}

std::unique_ptr<Backend> AppServer::makeBackendPtr(Backend::Type type,
                                                   const Jid& host,
                                                   const std::string& appName,
                                                   const std::string& certFile,
                                                   const std::string& authKey)
{
    std::unique_ptr<Backend> ret;
    switch(type)
    {
        case Backend::Type::Apns:
        {
            ret =
            std::unique_ptr<Backend>
            (
                new ApnsBackend {host, appName,  certFile}
            );
            break;
        }

        case Backend::Type::Gcm:
        {
            ret =
            std::unique_ptr<Backend>
            (
                new GcmBackend {host, appName, certFile, authKey}
            );
            break;
        }

        case Backend::Type::Ubuntu:
        {
            ret = std::unique_ptr<Backend>
            (
                new UbuntuBackend {host, appName, certFile}
            );
            break;
        }

        default:
        {
            break;
        }
    }

    return ret;
}

Backend::Type AppServer::getRegType(const Registration& reg)
{
    Backend::IdT backendId {reg.getBackendId()};

    auto result = mBackends.find(backendId);

    if(result != mBackends.cend())
    {
        return result->second->type;
    }

    return Backend::Type::Invalid;
}

std::unordered_map<AppServer::NodeIdT, Registration> AppServer::readRegs() const
{
    std::unordered_map<NodeIdT, Registration> ret; 

    std::ifstream iFile
    {
        STORAGE_DIR + getJid().full(),
        std::ifstream::in
    };

    if(not iFile.is_open())
    {
        // TODO: log info
        std::cout << "INFO: no input file, skipping reading registrations from disk"
                  << std::endl;
    }

    else
    {
        while(not iFile.eof())
        {
            NodeIdT node;
            Registration reg;

            std::getline(iFile, node);
            // DEBUG:
            std::cout << "DEBUG: read node: " << node << std::endl;
            iFile >> reg;
            
            std::cout << "DEBUG: read reg, eofbit: " << iFile.eof() << std::endl;
            
            if(not iFile.eof())
            {
                // DEBUG:
                std::cout << "DEBUG: read registration from storage:" << std::endl
                          << "user: " << reg.getUser().full() << std::endl
                          << "deviceId: " << reg.getDeviceId() << std::endl
                          << "deviceName" << reg.getDeviceName() << std::endl
                          << "token: " << reg.getToken() << std::endl
                          << "appId: " << reg.getAppId() << std::endl
                          << "backendId: " << reg.getBackendId() << std::endl
                          << "timestmap: " << reg.getTimestamp() << std::endl;
    
                ret.emplace(node, reg);
            }
        }

        iFile.close();
    }

    return ret;
}

void AppServer::writeRegs() const
{
    std::ofstream oFile
    {
        STORAGE_DIR + getJid().full(),
        std::ofstream::out | std::ofstream::trunc
    };

    if(not oFile.is_open())
    {
        // TODO: log error
        std::cout << "ERROR: could not open file for serialization" << std::endl;
    }
    
    else
    {
        for(const auto& r : mRegs)
        {
            oFile << r.first << '\n'
                  << r.second;
        }

        oFile.close();
    }
}
