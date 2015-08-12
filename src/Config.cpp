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

#include <Config.hpp>

#include <fstream>

using namespace Oshiya;

Config::Config(const std::string& fileName)
    : mYamlRoot {YAML::LoadFile(fileName)}
{

}

Config::Config(const NodeT& node)
    : mYamlRoot {node}
{

}

std::vector<Config> Util::makeComponentConfigs(const Config& config)
{
    std::vector<Config> ret;
    const Config::NodeT& components = config.getNode("components");
    for(Config::IteratorT it {components.begin()}; it != components.end(); ++it)
    {
        ret.push_back(Config{*it});
    }
    return ret;
}
