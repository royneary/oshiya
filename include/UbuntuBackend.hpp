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

#ifndef OSHIYA_UBUNTU_BACKEND__H
#define OSHIYA_UBUNTU_BACKEND__H

#include "Backend.hpp"
#include "json/json.h"
#include "curl_easy.h"

namespace Oshiya
{
    class UbuntuBackend : public Backend
    {
        public:
        ///////

        UbuntuBackend(const Jid& host,
                      const std::string& appName,
                      const std::string& certFile);

        ~UbuntuBackend() override;

        bool send(const PushNotification& notification) override;

        std::string getIso8601Date(const std::chrono::system_clock::time_point& date);

        private:
        ////////

        Json::StyledWriter mWriter;
        curl::curl_easy mCurl;
    };
}

#endif
