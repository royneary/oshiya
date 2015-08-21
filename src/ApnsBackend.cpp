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

bool ApnsBackend::send(const PushNotification& notification)
{
    bool success {false};

    // DEBUG:
    std::cout << "DEBUG: token = " << notification.token << std::endl;

    std::string binaryToken {Util::base64Decode(notification.token)};
    std::string token {binaryToHex(binaryToken)};

    std::cout << "DEBUG: decoded token length = " << token.size() << std::endl;
    std::cout << "DEBUG: decoded token = " << token << std::endl;

    apn_payload_ctx_ref payloadCtx = nullptr;
    
    if(apn_payload_init(&payloadCtx, &mError) == APN_ERROR)
    {
         // TODO: log error
        std::cout << "ERROR: " << apn_error_message(mError) << std::endl;
        apn_error_free(&mError);
    }

    if (apn_payload_add_token(payloadCtx, token.c_str(), &mError) == APN_ERROR)
    {
         // TODO: log error
        std::cout << "ERROR: " << apn_error_message(mError) << std::endl;
        apn_error_free(&mError);
    }

    apn_payload_set_content_available(payloadCtx, 1, nullptr);

    if(apn_send(mApnCtx, payloadCtx, &mError) == APN_ERROR)
    {
        // TODO: log error
        std::cout << "ERROR: " << apn_error_message(mError) << std::endl;
        std::cout << "error code: " << APN_ERR_CODE_WITHOUT_CLASS(apn_error_code(mError))
                  << std::endl;

        apn_error_free(&mError);

        connectApns();
    }

    else
    {
        // DEBUG:
        std::cout << "DEBUG: Success!" << std::endl;
        success = true;
    }

    apn_payload_free(&payloadCtx);
    
    return success;
}

bool ApnsBackend::connectApns()
{
    if(apn_connect(mApnCtx, &mError) == APN_ERROR)
    {
        std::cout << "ERROR: " << apn_error_message(mError) << std::endl;
        apn_error_free(&mError);

        return false;
    }

    return true;
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

