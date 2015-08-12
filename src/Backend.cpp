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

#include <Backend.hpp>

#include <sstream>
#include <type_traits>

// DEBUG
#include <iostream>

using namespace Oshiya;

Backend::Backend(Type _type,
                 const Jid& _host,
                 const std::string& _appName,
                 const std::string& _certFile)
    :
        type {_type},
        host {_host},
        appName {_appName},
        certFile {_certFile},
        mWorkerThread {&Backend::doWork, this}
{
    //mWorkerThread = std::thread {&Backend::doWork, this};
}

Backend::~Backend()
{
    {
        std::lock_guard<std::mutex> lk {mDispatchMutex};

        mShutdown = true;
    }

    mDispatchCv.notify_one();

    if(mWorkerThread.get_id() != std::thread::id {})
    {
        mWorkerThread.join();
    }
}

Backend::IdT Backend::getId()
{
    return makeBackendId(type, host);
}

Backend::IdT Backend::makeBackendId(Type type,
                                    const Jid& host)
{
    using TypeT = std::underlying_type<Type>::type;

    std::stringstream s;
    s << static_cast<TypeT>(type) << '_' << host.full();
    std::hash<std::string> hashFun;
    return hashFun(s.str());
}

Backend::Type Backend::makeType(const std::string& typeStr)
{
    if(typeStr == "apns") {return Type::Apns;}
    if(typeStr == "gcm") {return Type::Gcm;}
    if(typeStr == "mozilla") {return Type::Mozilla;}
    if(typeStr == "ubuntu") {return Type::Ubuntu;}
    if(typeStr == "wns") {return Type::Wns;}
    return Type::Invalid;
}

std::string Backend::getTypeStr(Type type)
{
    if(type == Type::Apns) {return "apns";}
    if(type == Type::Gcm) {return "gcm";}
    if(type == Type::Mozilla) {return "mozilla";}
    if(type == Type::Ubuntu) {return "ubuntu";}
    if(type == Type::Wns) {return "wns";}
    return "";
}

void Backend::dispatch(const PayloadT& payload,
                       const std::string& token,
                       const std::string& appId,
                       std::function<void()> unregisterCb)
{
    {
        std::lock_guard<std::mutex> lk {mDispatchMutex};

        mDispatchQueue.emplace(payload, token, appId, unregisterCb);
    }

    // DEBUG:
    std::cout << "Backend::dispatch: notifying worker" << std::endl;

    mDispatchCv.notify_one(); 
}

void Backend::doWork()
{
    while(true)
    {
        std::unique_lock<std::mutex> lk {mDispatchMutex};

        if(not mDispatchQueue.empty())
        {
            mDispatchCv.wait_for(
                lk, 
                std::chrono::milliseconds(Parameters::RetryPeriod)
            );
        }

        else
        {
            mDispatchCv.wait(lk);
        }

        // DEBUG:
        std::cout << "DEBUG: worker woke up!" << std::endl;

        if(mShutdown)
        {
            return;
        }

        bool sendSuccessful {true};

        while(sendSuccessful and not mDispatchQueue.empty())
        {
            sendSuccessful = send(mDispatchQueue.front());

            if(sendSuccessful)
            {
                // DEBUG:
                std::cout << "DEBUG: send successful!" << std::endl;
                mDispatchQueue.pop();
            }
            else
            {
                // DEBUG:
                std::cout << "DEBUG: send not successful!" << std::endl;
            }
        }
    }
}
