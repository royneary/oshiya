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

#ifndef OSHIYA_APNS_BACKEND__H
#define OSHIYA_APNS_BACKEND__H

#include "Backend.hpp"
extern "C"
{
    #include "apn.h"
}

namespace Oshiya
{
    class ApnsBackend : public Backend
    {
        public:
        ///////

        using PushNotification = Backend::PushNotification;

        ApnsBackend(const Jid& host,
                    const std::string& appName,
                    const std::string& certFile);

        ~ApnsBackend() override;

        bool send(const PushNotification& notification) override;

        private:
        ////////

        bool connectApns();

        static std::string binaryToHex(const std::string& binaryToken);

        apn_ctx_ref mApnCtx;
        apn_error_ref mError;
    };
}

#endif
