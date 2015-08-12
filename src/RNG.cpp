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

#include "RNG.hpp"

using namespace Oshiya;

const char RNG::alphabet[]
{
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789"
    "_-+"
};

RNG::RNG()
    :
        mGenerator {std::mt19937 {std::random_device {}()}},
        mDist
        {
            std::uniform_int_distribution<int>
            {0, sizeof(alphabet)/sizeof(*alphabet)-2}
        }
{

}

RNG::~RNG()
{

}

std::string RNG::getRandomText(std::size_t length)
{
    std::string str (length, 0);
    std::generate_n(str.begin(),
                    length,
                    [this]() {return alphabet[mDist(mGenerator)];});
    return str;
}
