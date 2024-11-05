/*========================================================

 XEMarkup - YAML Formatter
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

#include "XEMarkup/YAMLFormatter.h"

#include <XEMarkup/MappingNode.h>

#include <yaml-cpp/yaml.h>

#include <cstdint>
#include <fstream>
#include <limits>
#include <regex>
#include <sstream>
#include <string>

using namespace xe;

static void Import(const YAML::Node& in, MappingNode& out)
{
    if (in.IsMap())
    {
        for (YAML::const_iterator it = in.begin(); it != in.end(); ++it)
        {
            Import(it->second, out[it->first.as<std::string>()]);
        }
        return;
    }
    if (in.IsSequence())
    {
        for (const YAML::Node& child : in)
        {
            MappingNode result;
            Import(child, result);
            out.PushBack(result);
        }
        return;
    }

    const std::string& content = in.Scalar();

    // BOOL
    static const std::regex boolRegex("^(true|false|yes|no|on|off)$", std::regex::icase);
    if (std::regex_match(content, boolRegex))
    {
        out = in.as<bool>();
        return;
    }

    //INT
    static const std::regex numberRegex("^-?\\d*\\.?\\d+([eE][-+]?\\d+)?$");
    static const std::regex unsignedRegex("^\\d+$");
    static const std::regex signedRegex("^-\\d+$");

    if (std::regex_match(content, numberRegex))
    {
        if (std::regex_match(content, unsignedRegex) || std::regex_match(content, signedRegex))
        {
            try
            {
                if (content[0] == '-')
                {
                    int64_t int64Val = std::stoll(content);
                    if (int64Val >= std::numeric_limits<int32_t>::min() &&
                        int64Val <= std::numeric_limits<int32_t>::max())
                    {
                        out = static_cast<int32_t>(int64Val);
                        return;
                    }
                    out = int64Val;
                    return;
                }

                else
                {
                    uint64_t uint64Val = std::stoull(content);

                    if (uint64Val <= std::numeric_limits<uint32_t>::max())
                    {
                        out = static_cast<uint32_t>(uint64Val);
                        return;
                    }
                    out = uint64Val;
                    return;
                }
            }
            catch (const std::out_of_range&) { throw std::runtime_error("Bad integral parsing."); }
        }

        // DECIMAL
        double doubleVal = std::stod(content);

        float floatVal = static_cast<float>(doubleVal);
        if (static_cast<double>(floatVal) == doubleVal)
        {
            out = floatVal;
            return;
        }
        out = doubleVal;
        return;
    }

    // STRING
    out = content;
}

static void Export(YAML::Node& out, const MappingNode& in)
{
    if (in.IsMapping())
    {
        for (const MappingNode& child : in)
        {
            Export(out[child.Key()], child);
        }
        return;
    }

    if (in.IsArray())
    {
        for (const MappingNode& child : in)
        {
            YAML::Node result;
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

MappingNode xe::YAMLFormatter::LoadFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::ate);
    size_t length = file.tellg();

    if (length == 0)
        return MappingNode();

    file.seekg(0, std::ios::beg);
    std::vector<char> content(length + 1);
    file.read(content.data(), length);
    content.back() = '\0';
    return LoadContent(content.data());
    
}

MappingNode xe::YAMLFormatter::LoadContent(const std::string& content)
{
    MappingNode result;
    YAML::Node in = YAML::Load(content);
    Import(in, result);
    return result;
}

MappingNode xe::YAMLFormatter::LoadContent(const std::vector<uint8_t>& content)
{
    if (content.empty() || content.back() != '\0')
    {
        throw std::runtime_error("Content should be a string. Expected null termination character not found.");
    }
    
    return LoadContent((const char*)content.data());
}

bool xe::YAMLFormatter::SaveFile(const MappingNode& node, const std::filesystem::path& path)
{
    std::string content;
    SaveContent(node, content);
    std::ofstream file(path);
    if (!file.is_open())
        return false;

    file << content;
    return true;
}

void xe::YAMLFormatter::SaveContent(const MappingNode& node, std::string& out_content)
{
    YAML::Node out;
    Export(out, node);
    std::stringstream stream;
    stream << out;
    out_content = stream.str();
}

void xe::YAMLFormatter::SaveContent(const MappingNode& node, std::vector<uint8_t>& out_content)
{
    std::string contentStr;
    SaveContent(node, contentStr);
    out_content.clear();
    out_content.resize(contentStr.length() + 1);
    memcpy_s(out_content.data(), contentStr.length(), contentStr.c_str(), contentStr.length());
    out_content.back() = 0;
}
