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

#include "Registration.hpp"

using namespace Oshiya;

Registration::Registration(const Jid& user,
                           const std::string& deviceId,
                           const std::string& deviceName,
                           const std::string& token,
                           const std::string& appId,
                           Backend::IdT backendId,
                           std::time_t timestamp)
    :
        mUser {user},
        mDeviceId {deviceId},
        mDeviceName {deviceName},
        mToken {token},
        mAppId {appId},
        mBackendId {backendId},
        mTimestamp {timestamp}
{

}
