/*========================================================

 XEMarkup - BSON Formatter
 Copyright (C) 2024 Jon Bogert (jonbogert@gmail.com)

 This software is provided 'as-is', without any express or implied warranty.
 In no event will the authors be held liable for any damages arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it freely,
 subject to the following restrictions:

 1. The origin of this software must not be misrepresented;
    you must not claim that you wrote the original software.
    If you use this software in a product, an acknowledgment
    in the product documentation would be appreciated but is not required.

 2. Altered source versions must be plainly marked as such,
    and must not be misrepresented as being the original software.

 3. This notice may not be removed or altered from any source distribution.

========================================================*/

#include "XEMarkup/BSONFormatter.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

using namespace xe;
using json = nlohmann::json;

static void Import(const json& in, MappingNode& out)
{
    if (in.is_object())
    {
        for (auto it = in.begin(); it != in.end(); ++it)
        {
            Import(it.value(), out[it.key()]);
        }
        return;
    }

    if (in.is_array())
    {
        for (const auto& child : in)
        {
            MappingNode result;
            Import(child, result);
            out.PushBack(result);
        }
        return;
    }

    if (in.is_boolean())
    {
        out = in.get<bool>();
        return;
    }

    if (in.is_number())
    {
        if (in.is_number_float())
        {
            if (in.is_number_float())
            {
                out = in.get<float>();
                return;
            }
            out = in.get<double>();
            return;
        }

        if (!in.is_number_unsigned())
        {
            if (in.is_number_integer() && in.get<int64_t>() >= std::numeric_limits<int32_t>::min() &&
                in.get<int64_t>() <= std::numeric_limits<int32_t>::max())
            {
                out = in.get<int32_t>();
                return;
            }
            out = in.get<int64_t>();
            return;
        }

        if (in.is_number_unsigned() && in.get<uint64_t>() <= std::numeric_limits<uint32_t>::max())
        {
            out = in.get<uint32_t>();
            return;
        }
        out = in.get<uint64_t>();
        return;
    }

    out = in.get<std::string>();
}

static void Export(json& out, const MappingNode& in)
{
    if (in.IsMapping())
    {
        for (const MappingNode& child : in)
        {
            json result;
            Export(result, child);
            out[child.Key()] = result;
        }
        return;
    }

    if (in.IsArray())
    {
        for (const MappingNode& child : in)
        {
            json result;
            Export(result, child);
            out.push_back(result);
        }
        return;
    }

    if (in.IsBoolean())
    {
        out = in.As<bool>();
        return;
    }

    if (in.IsNumeric())
    {
        if (in.HasDecimal())
        {
            if (in.Width() == sizeof(float))
            {
                out = in.As<float>();
                return;
            }
            out = in.As<double>();
            return;
        }

        if (in.IsNegative())
        {
            if (in.Width() <= sizeof(int32_t))
            {
                out = in.As<int32_t>();
                return;
            }
            out = in.As<int64_t>();
            return;
        }

        if (in.Width() <= sizeof(uint32_t))
        {
            out = in.As<uint32_t>();
            return;
        }
        out = in.As<uint64_t>();
        return;
    }

    out = in.As<std::string>();
}

MappingNode xe::BSONFormatter::LoadFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    size_t length = file.tellg();
    
    if (length == 0)
        return MappingNode();
    
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> content(length);
    file.read((char*)content.data(), length);
    return LoadContent(content);
}

MappingNode xe::BSONFormatter::LoadContent(const std::vector<uint8_t>& content)
{
    json in = json::from_bson(content);
    MappingNode result;
    Import(in, result);
    return result;
}

bool xe::BSONFormatter::SaveFile(const MappingNode& node, const std::filesystem::path& path)
{
    std::vector<uint8_t> content;
    SaveContent(node, content);
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
        return false;
    
    file.write((char*)content.data(), content.size());
    return true;
}

void xe::BSONFormatter::SaveContent(const MappingNode& node, std::vector<uint8_t>& out_content)
{
    json content;
    Export(content, node);
    out_content = json::to_bson(content);
}
