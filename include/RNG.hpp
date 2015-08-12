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

#ifndef OSHIYA_RNG__H
#define OSHIYA_RNG__H

#include <algorithm>
#include <random>
#include <string>

namespace Oshiya
{
	class RNG
	{
		public:
        ///////

        RNG();
		
        ~RNG();

        static const char alphabet[];

        std::string getRandomText( std::size_t length);

        template <typename IntT>
		IntT getRandomNumber(IntT min = std::numeric_limits<IntT>::min(),
                             IntT max = std::numeric_limits<IntT>::max()) const
        {
            std::uniform_int_distribution<IntT> dist {min, max};

            return dist(mGenerator);
        }

		private:
        ////////

        std::mt19937 mGenerator;
        std::uniform_int_distribution<int> mDist;
	};
}

#endif
