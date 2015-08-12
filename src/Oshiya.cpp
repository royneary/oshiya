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

#include "AppServer.hpp"
#include "config.h"

#include <iostream>
#include <csignal>
#include <mutex>
#include <condition_variable>

namespace
{
      std::mutex m;
      std::condition_variable cv;
}

void signalHandler(int signal)
{
    if(signal == SIGINT or signal == SIGTERM)
    {
        std::cout << "Oshiya terminating with signal " << signal << std::endl;
        cv.notify_one();
    }
}

int main()
{
    using namespace Oshiya;

    std::cout << "Oshiya" << std::endl;
    std::cout << "config file: " << CONFIG_FILE << std::endl;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    Config config {CONFIG_FILE};
    
    std::vector<std::unique_ptr<AppServer>> appServers;
    
    try
    {
        Config::NodeT components {config.getNode("components")};

        for(Config::IteratorT it {components.begin()}; it != components.end(); ++it)
        {
            appServers.emplace_back(make_unique<AppServer>(Config {*it}));
        }
    }

    catch(const Config::InvalidConfig& e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
        return -1;
    }

    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk);

    return 0;
}
