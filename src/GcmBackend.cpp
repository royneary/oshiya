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

#include "GcmBackend.hpp"

#include "SmartPointerUtil.hpp"
#include "curl_header.h"
#include <cstring>

using namespace Oshiya;

GcmBackend::GcmBackend(const Jid& host,
                   const std::string& appName,
                   const std::string& certFile,
                   const std::string& authKey)
    :
        Backend(Backend::Type::Gcm,
                host,
                appName,
                certFile),
        mAuthKey {authKey}
{

}

GcmBackend::~GcmBackend()
{

}

Backend::NotificationQueueT GcmBackend::send(const NotificationQueueT& notifications)
{
    // DEBUG:
    std::cout << "DEBUG: in GcmBackend::send" << std::endl;

    NotificationQueueT retryQueue;

    for(auto it = notifications.cbegin(); it != notifications.cend(); ++it)
    {
        const PushNotification& n {*it};

        std::string payload {makePayload(n.token, n.payload)};

        if(payload.size() > GcmParameters::MaxPayloadSize)
        {
            payload = makePayload(n.token, {});
        }

        std::string responseBody;
    
        curl_header header;
        header.add("Content-Type:application/json");
        header.add("Authorization:key=" + mAuthKey);

        mCurl.add(
            curl_pair<CURLoption, curl_header>
            {CURLOPT_HTTPHEADER, header}
        );

        mCurl.add(
            curl_pair<CURLoption, std::string>
            {CURLOPT_URL, "https://gcm-http.googleapis.com/gcm/send"}
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
            curl_pair<CURLoption, decltype(&GcmBackend::bodyWriteCb)>
            {CURLOPT_WRITEFUNCTION, bodyWriteCb}
        );

        try
        {
            mCurl.perform();
            
            std::unique_ptr<long> responseCode
            {mCurl.get_info<long>(CURLINFO_RESPONSE_CODE)};

            if(responseCode.get() == nullptr)
            {
                // connection error
                retryQueue.insert(retryQueue.end(), it, notifications.cend());
            }

            else if(*responseCode == 200)
            {
                bool retry {processSuccessResponse(responseBody, n)};

                if(retry)
                {
                    retryQueue.push_back(n);
                }
            }

            else if(*responseCode >= 500 and *responseCode < 600)
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

        mCurl.reset();
    }

    return retryQueue;
}

std::size_t GcmBackend::bodyWriteCb(char* ptr,
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

bool GcmBackend::processSuccessResponse(const std::string& responseBody,
                                   const PushNotification& notification)
{
    Json::Value root;
    Json::Reader reader;

    bool success {reader.parse(responseBody, root)};

    if(not success)
    {
        // invalid response, treating as non-recoverable error
        notification.unregisterCb();
        return false;
    }

    int failure {root.get("failure", -1).asInt()};

    if(failure == 0)
    {
        // success
        return false;
    }

    if(failure < 0)
    {
        // invalid response, treating as non-recoverable error
        notification.unregisterCb();
        return false;
    }

    Json::Value results {root["results"]};

    if(not results.isArray())
    {
        // invalid response, treating as non-recoverable error
        notification.unregisterCb();
        return false;
    }

    std::string error {results.get("error", "").asString()};

    if(error == "Unavailable" or error == "InternalServerError")
    {
        // recoverable error, retry
        return true;
    }

    // non-recoverable error
    notification.unregisterCb();
    return false;
}

std::string GcmBackend::makePayload(const std::string& token,
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

    jsonPayload["to"] = token;
    jsonPayload["expiry_time"] = Parameters::NotificationExpireTime;
    jsonPayload["data"] = data;

    return mWriter.write(jsonPayload); 
}
