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

#include "ApnsBackend.hpp"
#include "Base64.hpp"
#include "SmartPointerUtil.hpp"

// DEBUG:
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <vector>

using namespace Oshiya;

ApnsBackend::ApnsBackend(const Jid& host,
                         const std::string& appName,
                         const std::string& certFile)
    :
        Backend(Backend::Type::Apns,
                host,
                appName,
                certFile),
        mApnCtx {nullptr},
        mError {nullptr}
{
    if(apn_init(&mApnCtx, certFile.c_str(), certFile.c_str(), nullptr, &mError)
       == APN_ERROR)
    {
        // TODO: log error
        std::cout << "ERROR: " << apn_error_message(mError) << std::endl;
        apn_error_free(&mError);
    }

    apn_set_mode(mApnCtx, APN_MODE_SANDBOX, nullptr);

    connectApns();
}

ApnsBackend::~ApnsBackend()
{
    apn_close(mApnCtx);
    apn_free(&mApnCtx);
}

Backend::NotificationQueueT
ApnsBackend::send(const NotificationQueueT& notifications)
{
    // DEBUG:
    std::cout << "ApnsBackend::send: notifications.size(): "
              << notifications.size() << std::endl;

    if(not mConnected)
    {
        connectApns();
    }

    NotificationQueueT retryQueue;

    for(auto it = notifications.cbegin(); it != notifications.cend(); ++it)
    {
        const PushNotification& n {*it};
        // DEBUG:
        std::cout << "DEBUG: token = " << n.token << std::endl;

        std::string binaryToken {Util::base64Decode(n.token)};
        std::string token {binaryToHex(binaryToken)};

        std::cout << "DEBUG: decoded token length = " << token.size() << std::endl;
        std::cout << "DEBUG: decoded token = " << token << std::endl;

        auto payloadCtxPtr = makePayload(token, n.payload);
        apn_payload_ctx_ref payloadCtx {payloadCtxPtr.get()};
        uint8_t result {apn_send(mApnCtx, payloadCtx, &mError)};
       
        // libcapn tells us about an invalid payload size, retry once with empty
        // payload in that case
        if(result == APN_ERROR and
           apn_error_code(mError) == APN_ERR_INVALID_PAYLOAD_SIZE) 
        {
            payloadCtxPtr = makePayload(token, {});
            apn_payload_ctx_ref fixedPayload {payloadCtxPtr.get()};
            result = apn_send(mApnCtx, fixedPayload, &mError);
        }

        if(result == APN_ERROR)
        {
            int32_t errorCondition {apn_error_code(mError)};

            switch(errorCondition)
            {
                case APN_ERR_SERVICE_SHUTDOWN:
                {
                    // DEBUG:
                    std::cout << "DEBUG: service shutdown" << std::endl;
                    // recoverable error
                    retryQueue.push_back(n);
                    break;
                }

                case APN_ERR_NOT_CONNECTED:
                case APN_ERR_CONNECTION_CLOSED:
                case APN_ERR_SSL_READ_FAILED:
                case APN_ERR_SELECT:
                {
                    // DEBUG:
                    std::cout << "DEBUG: connection error" << std::endl;
                    // connection error
                    disconnectApns();
                    connectApns();
                    retryQueue.insert(retryQueue.end(), it, notifications.cend());
                    break;
                }

                default:
                {
                    // DEBUG:
                    std::cout << "DEBUG: non-recoverable error" << std::endl;
                    // non-recoverable error
                    n.unregisterCb();
                    break;
                }
            }

            apn_error_free(&mError);
        }

        else
        {
            // DEBUG:
            std::cout << "DEBUG: success!" << std::endl;
            // success
        }
    }

    return retryQueue;
}

void ApnsBackend::connectApns()
{
    if(apn_connect(mApnCtx, &mError) == APN_ERROR)
    {
        std::cout << "ERROR: " << apn_error_message(mError) << std::endl;
        apn_error_free(&mError);

        mConnected = false;
    }

    else
    {
        mConnected = true;
    }
}

void ApnsBackend::disconnectApns()
{
    apn_close(mApnCtx);
    mConnected = false;
}

std::unique_ptr<__apn_payload, ApnsBackend::PayloadDeleterT>
ApnsBackend::makePayload(const std::string& token, const PayloadT& payload)
{
    apn_payload_ctx_ref payloadCtx = nullptr;
    
    apn_payload_init(&payloadCtx, &mError);

    apn_payload_add_token(payloadCtx, token.c_str(), &mError);

    apn_payload_set_content_available(payloadCtx, 1, nullptr);

    for(const PayloadT::value_type& p : payload)
    {
        try
        {
            std::size_t convertedStrLength;
            unsigned long converted {std::stoul(p.second, &convertedStrLength)};
            
            if(convertedStrLength == p.second.size() and converted <= UINT32_MAX)
            {
                apn_payload_add_custom_property_integer(payloadCtx,
                                                        p.first.c_str(),
                                                        converted,
                                                        nullptr);
            }

            else
            {
                apn_payload_add_custom_property_string(payloadCtx,
                                                       p.first.c_str(),
                                                       p.second.c_str(),
                                                       nullptr);
            }
        }

        catch(const std::logic_error&)
        {
            apn_payload_add_custom_property_string(payloadCtx,
                                                   p.first.c_str(),
                                                   p.second.c_str(),
                                                   nullptr);
        }
    }

    PayloadDeleterT payloadDeleter
    {[](apn_payload_ctx_ref p) {apn_payload_free(&p);}};

    return
    {payloadCtx, payloadDeleter};
}

std::string ApnsBackend::binaryToHex(const std::string& binaryToken)
{
    std::vector<unsigned char> v (binaryToken.cbegin(), binaryToken.cend());
    
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill( '0' );
    for( int c : v )
    {
        ss << std::setw( 2 ) << c;
    }

    return ss.str();
}
