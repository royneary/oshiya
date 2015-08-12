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

#ifndef OSHIYA_CONFIG__H
#define OSHIYA_CONFIG__H

#include <yaml-cpp/yaml.h>

#include <string>

// DEBUG:
#include <iostream>

namespace Oshiya
{
    class Config
    {
        public:
        ///////

        using NodeT = YAML::Node;
        using IteratorT = YAML::const_iterator;

        struct InvalidConfig : public std::runtime_error
        {
            InvalidConfig(const std::string& msg)
                : std::runtime_error {msg}
            { }
        };

        Config(const std::string& fileName);
        Config(const NodeT& node);

        template <typename T = std::string>
        T value(const std::string& key) const
        {
            NodeT node {mYamlRoot[key]};

            if(not node.IsDefined())
            {
                std::cout << "throwing InvalidConfig 1" << std::endl;
                throw InvalidConfig {"Invalid config: Option " + key + " not defined"};
            }

            try
            {
                return node.as<T>();
            }
            catch(const YAML::Exception&)
            {
                std::cout << "throwing InvalidConfig 2" << std::endl;
                throw
                InvalidConfig {"Invalid config: Option " + key + " has invalid value"};
            }
        }

        template <typename T = std::string>
        const T value(const std::string& key, const T& defaultValue) const
        {
            NodeT node {mYamlRoot[key]};

            if(not node.IsDefined())
            {
                return defaultValue;
            }

            try
            {
                return node.as<T>();
            }
            catch(const YAML::Exception&)
            {
                return defaultValue;
            }
        }

        NodeT getNode(const std::string& key) const
        {
            NodeT node {mYamlRoot[key]};

            if(not node.IsDefined())
            {
                std::cout << "throwing InvalidConfig 3" << std::endl;
                throw InvalidConfig {"Invalid config: Option " + key + " not defined"};
            }

            return node;
        }

        private:
        ////////

        YAML::Node mYamlRoot;
    };

    namespace Util
    {
        std::vector<Config> makeComponentConfigs(const Config& config); 
    }
}

#endif
