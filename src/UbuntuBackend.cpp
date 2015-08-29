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

#include "UbuntuBackend.hpp"

#include "curl_header.h"
#include <cstring>

using namespace Oshiya;

UbuntuBackend::UbuntuBackend(const Jid& host,
                             const std::string& appName,
                             const std::string& certFile)
    : 
        Backend(Backend::Type::Ubuntu,
                host,
                appName,
                certFile)
{
    startWorker();
}

UbuntuBackend::~UbuntuBackend()
{

}

Backend::NotificationQueueT
UbuntuBackend::send(const NotificationQueueT& notifications)
{
    std::cout << "DEBUG: in UbuntuBackend::send" << std::endl;

    NotificationQueueT retryQueue;

    for(auto it = notifications.cbegin(); it != notifications.cend(); ++it)
    {
        const PushNotification& n {*it}; 

        std::string payload {makePayload(n.appId, n.token, n.payload)};

        if(payload.size() > UbuntuParameters::MaxPayloadSize)
        {
            payload = makePayload(n.appId, n.token, {});
        }

        std::string responseBody;

        curl_header header;
        header.add("Content-Type:application/json");

        mCurl.add(
            curl_pair<CURLoption, curl_header>
            {CURLOPT_HTTPHEADER, header}
        );

        mCurl.add(
            curl_pair<CURLoption, std::string>
            {CURLOPT_URL, "https://push.ubuntu.com/notify"}
        );

        mCurl.add(
            curl_pair<CURLoption, std::string>
            {CURLOPT_SSLCERT, certFile}
        );

        mCurl.add(
            curl_pair<CURLoption, std::string>
            {CURLOPT_SSLKEY, certFile}
        );

        mCurl.add(
            curl_pair<CURLoption, bool>
            {CURLOPT_SSL_VERIFYPEER, true}
        );

        mCurl.add(
            curl_pair<CURLoption, std::string>
            {CURLOPT_POSTFIELDS, payload}
        );

        mCurl.add(
            curl_pair<CURLoption, void*>
            {CURLOPT_WRITEDATA, &responseBody}
        );

        mCurl.add(
            curl_pair<CURLoption, decltype(&UbuntuBackend::bodyWriteCb)>
            {CURLOPT_WRITEFUNCTION, bodyWriteCb}
        );

        try
        {
            mCurl.perform();

            std::unique_ptr<long> responseCode
            {mCurl.get_info<long>(CURLINFO_RESPONSE_CODE)};

            /**
             * Error conditions - extracted from
             * http://bazaar.launchpad.net/~ubuntu-push-hackers/ubuntu-push/trunk/view/head:/server/api/handlers.go
             *
             * 411 - "invalid-request" - "A Content-Length must be provided"
             * 400 - "invalid-request" - "Request body empty"
             * 413 - "invalid-request" - "Request body too large"
             * 415 - "invalid-request" - "Wrong content type, should be application/json"
             * 405 - "invalid-request" - "Wrong request method, should be POST"
             * 405 - "invalid-request" - "Wrong request method, should be GET"
             * 400 - "invalid-request" - "Malformed JSON Object"
             * 400 - "io-error" - "Could not read request body"
             * 400 - "invalid-request" - "Missing id field"
             * 400 - "invalid-request" - "Missing data field"
             * 400 - "invalid-request" - "Data too large"
             * 400 - "invalid-request" - "Invalid expiration date"
             * 400 - "invalid-request" - "Past expiration date"
             * 400 - "unknown-channel" - "Unknown channel"
             * 400 - "unknown-token" - "Unknown token"
             * 500 - "internal" - "Unknown error"
             * 503 - "unavailable" - "Message store unavailable"
             * 503 - "unavailable" - "Could not store n"
             * 503 - "unavailable" - "Could not make token"
             * 503 - "unavailable" - "Could not remove token"
             * 503 - "unavailable" - "Could not resolve token"
             * 401 - "unauthorized" - "Unauthorized"
             * 413 - "too-many-pending" - "Too many pending ns for this application"
             */

            if(responseCode.get() == nullptr)
            {
                // connection error
                retryQueue.insert(retryQueue.end(), it, notifications.cend());
            }
           
            else if(*responseCode == 200)
            {
                // success
            }

            else if(*responseCode == 503)
            {
                // recoverable error
                retryQueue.push_back(n);
            }

            else
            {
                // non-recoverable error
                n.unregisterCb();
            }
        }

        catch(curl_easy_exception error)
        {
            // DEBUG:
            std::cout << "DEBUG: curl exception!" << std::endl;
            error.print_traceback();

            // connection error
            retryQueue.insert(retryQueue.end(), it, notifications.cend());
        }

        // DEBUG:
        std::cout << "DEBUG: before mCurl.reset()" << std::endl;

        mCurl.reset();
    }

    return retryQueue;
}

std::string UbuntuBackend::makePayload(const std::string& appId,
                                       const std::string& token,
                                       const PayloadT& payload)
{
    Json::Value jsonPayload {Json::objectValue};
    Json::Value data {Json::objectValue};

    for(const PayloadT::value_type& p : payload)
    {
        try
        {
            std::size_t convertedStrLength;
            unsigned long converted {std::stoul(p.second, &convertedStrLength)};
            
            if(convertedStrLength == p.second.size() and converted <= UINT32_MAX)
            {
                data[p.first] = static_cast<uint32_t>(converted);
            }

            else
            {
                data[p.first] = p.second;
            }
        }

        catch(const std::logic_error&)
        {
            data[p.first] = p.second;
        }
    }

    std::string expireOn
    {
        getIso8601Date(
            std::chrono::system_clock::now() +
            std::chrono::seconds(Parameters::NotificationExpireTime)
        )
    };

    jsonPayload["appid"] = appId;
    jsonPayload["expire_on"] = expireOn;
    jsonPayload["token"] = token;
    jsonPayload["clear_pending"] = true;
    jsonPayload["data"] = data;

    return mWriter.write(jsonPayload);
}

std::string UbuntuBackend::getIso8601Date(const std::chrono::system_clock::time_point& date)
{
    std::time_t t {std::chrono::system_clock::to_time_t(date)};

    char buf[sizeof "2011-10-08T07:07:09Z"];
    std::strftime(buf, sizeof buf, "%FT%TZ", gmtime(&t));

    return buf;
}

std::size_t UbuntuBackend::bodyWriteCb(char* ptr,
                                       std::size_t size,
                                       std::size_t nmemb,
                                       void* userdata)
{
    std::string& bodyStr = *static_cast<std::string*>(userdata);

    std::size_t newDataLength {size * nmemb};
    std::size_t existingLength {bodyStr.size()};

    bodyStr.resize(existingLength + newDataLength);

    memcpy(&bodyStr[existingLength], ptr, newDataLength);

    return bodyStr.size();
}
