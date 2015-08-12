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

}

UbuntuBackend::~UbuntuBackend()
{

}

bool UbuntuBackend::send(const PushNotification& notification)
{
    // DEBUG:
    std::cout << "DEBUG: in UbuntuBackend::send" << std::endl;

    Json::Value jsonNotification {Json::objectValue};
    Json::Value data {Json::objectValue};

    for(const PayloadT::value_type& p : notification.payload)
    {
        data[p.first] = p.second;
    }

    std::string expireOn
    {
        getIso8601Date(
            std::chrono::system_clock::now() +
            std::chrono::seconds(Parameters::NotificationExpireTime)
        )
    };

    jsonNotification["appid"] = notification.appId;
    jsonNotification["expire_on"] = expireOn;
    jsonNotification["token"] = notification.token;
    // FIXME: set clear_pending if "too-many-pending" received
    jsonNotification["clear_pending"] = false;
    jsonNotification["data"] = data;

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
        {CURLOPT_POSTFIELDS, mWriter.write(jsonNotification)}
    );

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

std::string UbuntuBackend::getIso8601Date(const std::chrono::system_clock::time_point& date)
{
    std::time_t t {std::chrono::system_clock::to_time_t(date)};

    char buf[sizeof "2011-10-08T07:07:09Z"];
    std::strftime(buf, sizeof buf, "%FT%TZ", gmtime(&t));

    return buf;
}
