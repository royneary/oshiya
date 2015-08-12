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

#include <XData.hpp>

#include <algorithm>

using namespace Oshiya;

XData::Field::Field(const std::string& _type,
                    const std::string& _var,
                    const std::vector<std::string>& _values)
    : type {_type}, var {_var}, values {_values}
{

}

bool XData::Field::isValid() const
{
    return not var.empty() or (var.empty() and type == "fixed");
}

std::string XData::Field::singleValue() const
{
    return values.empty() ? "" : values.front();
}

XData::Item::Item(const std::vector<Field>& fields)
    : mFields {fields}
{

}

XData::XData(const std::string& type)
    : mType {type}
{

}

XData::XData(const std::string& type, const std::vector<Field>& fields)
    : mType {type}, mFields {fields}
{

}

XData::XData(const std::string& type, const std::vector<Item>& items)
    : mType {type}, mItems {items}
{

}

void XData::addField(const Field& field)
{
    mFields.push_back(field);
}

void XData::addItem(const Item& item)
{
    mItems.push_back(item);
}

XData::Field XData::getField(const std::string& var,
                             const std::string& type) const
{
    auto result =
    std::find_if(mFields.begin(), mFields.end(),
                 [&var](const Field& f) {return f.var == var;});

    if(result != mFields.end())
    {
        if(result->type == type or (result->type.empty() and type == "text-single"))
        {
            return *result;
        }
    }

    return Field {};
}
