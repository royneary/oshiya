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

#ifndef OSHIYA_XDATA__H
#define OSHIYA_XDATA__H

#include <string>
#include <vector>
#include <map>

namespace Oshiya
{
    class XData
    {
        public:
        ///////

        struct Field
        {
            Field() = default;

            Field(const std::string& type,
                  const std::string& var,
                  const std::vector<std::string>& values);

            bool isValid() const;

            std::string singleValue() const;

            const std::string type;
            const std::string var;
            const std::vector<std::string> values;
        };

        struct Item
        {
            Item() = default;

            Item(const std::vector<Field>& fields);

            bool empty() const {return mFields.empty();}

            std::vector<Field> getFields() const {return mFields;}

            void addField(const Field& field) {mFields.push_back(field);}

            private:
            ////////

            std::vector<Field> mFields;
        };

        XData(const std::string& type = "");

        XData(const std::string& type, const std::vector<Field>& fields);

        XData(const std::string& type, const std::vector<Item>& items);

        bool empty() const {return mFields.empty() and mItems.empty();}

        std::string getType() const {return mType;}

        Field getField(const std::string& var,
                       const std::string& type = "text-single") const;

        std::vector<Field> getFields() const {return mFields;}

        std::vector<Item> getItems() const {return mItems;}

        void addField(const Field& field);

        void addItem(const Item& item);
        
        private:
        ////////

        std::string mType; 

        std::vector<Field> mFields;

        std::vector<Item> mItems;
    };
}

#endif
