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

#include "curl_header.h"

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

bool GcmBackend::send(const PushNotification& notification)
{
    // DEBUG:
    std::cout << "DEBUG: in GcmBackend::send" << std::endl;

    Json::Value jsonNotification {Json::objectValue};
    Json::Value data {Json::objectValue};

    for(const PayloadT::value_type& p : notification.payload)
    {
        data[p.first] = p.second;
    }

    jsonNotification["to"] = notification.token;
    jsonNotification["expiry_time"] = Parameters::NotificationExpireTime;
    jsonNotification["data"] = data;

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
        {CURLOPT_POSTFIELDS, mWriter.write(jsonNotification)}
    );

    // DEBUG:
    std::cout << "DEBUG: sending notification with payload " << mWriter.write(jsonNotification) << std::endl;

    bool responseOk {false};

    try
    {
        // DEBUG:
        std::cout << "before mCurl.perform()" << std::endl;

        mCurl.perform();

        // DEBUG:
        std::cout << "after mCurl.perform()" << std::endl;
        
        std::unique_ptr<long> responseCode {mCurl.get_info<long>(CURLINFO_RESPONSE_CODE)};

        responseOk =
        responseCode.get() != nullptr and
        (*responseCode == 200 or *responseCode == 400);

        // DEBUG:
        std::cout << "responseCode.get() == nullptr? " << (responseCode.get() == nullptr)
                  << std::endl;
        std::cout << "*responseCode =" << *responseCode << std::endl;

        if(*responseCode == 400)
        {
            notification.unregisterCb();     
        }

        // TODO: decode status line
    }

    catch(curl_easy_exception error)
    {
        // DEBUG:
        std::cout << "DEBUG: curl exception!" << std::endl;
        error.print_traceback(); 
    }

    // DEBUG:
    std::cout << "DEBUG: before mCurl.reset()" << std::endl;

    mCurl.reset();

    // DEBUG:
    std::cout << "DEBUG: after mCurl.reset()" << std::endl;

    return responseOk;
}
