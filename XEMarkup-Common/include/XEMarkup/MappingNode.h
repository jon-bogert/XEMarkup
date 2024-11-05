/*========================================================

 XEMarkup - MappingNode
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

#ifndef XE_MAPPINGNODE_H
#define XE_MAPPINGNODE_H

#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace xe
{
    class MappingNode;
    class IMappable
    {
    public:
        virtual void Map(MappingNode& node) const = 0;
        virtual void Unmap(const MappingNode& node) = 0;
    };

    class MappingNode
    {
    public:
        enum class Type : uint8_t
        {
            Null = 0,
            String = 1,
            Numeric = 2,
            Boolean = 3,
            Array = 4,
            Mapping = 5,

            // Numeric Flags
            FlagMask = 0xF0,
            Decimal = 0x10,
            Negative = 0x20,
        };

        MappingNode() noexcept : m_type(Type::Null) {}
        virtual ~MappingNode() noexcept
        {
            Clear();
        }

        // Copy constructor
        MappingNode(const MappingNode& other) : m_type(Type::Null)
        {
            try
            {
                m_type = other.m_type;
                m_key = other.m_key;
                m_data = other.m_data;
                m_children = other.m_children;
                m_keyMap = other.m_keyMap;
            }
            catch (...)
            {
                Clear();
                throw;
            }
        }

        // Move constructor
        MappingNode(MappingNode&& other) noexcept
            : m_type(other.m_type),
            m_key(std::move(other.m_key)),
            m_data(std::move(other.m_data)),
            m_children(std::move(other.m_children)),
            m_keyMap(std::move(other.m_keyMap))
        {
            other.m_type = Type::Null;
        }

        // Copy assignment
        MappingNode& operator=(const MappingNode& other)
        {
            if (this != &other)
            {
                MappingNode temp(other);
                *this = std::move(temp);
            }
            return *this;
        }

        // Move assignment
        MappingNode& operator=(MappingNode&& other) noexcept
        {
            if (this != &other)
            {
                Clear();
                m_type = other.m_type;
                m_data = std::move(other.m_data);
                m_children = std::move(other.m_children);
                m_keyMap = std::move(other.m_keyMap);
                other.m_type = Type::Null;
            }
            return *this;
        }

        // String assignment
        MappingNode& operator=(std::string_view value)
        {
            try
            {
                Clear();
                m_type = Type::String;
                m_data.resize(value.length());
                std::memcpy(m_data.data(), value.data(), value.length());
                return *this;
            }
            catch (...)
            {
                Clear();
                throw;
            }
        }

        // C-string assignment with nullptr check
        MappingNode& operator=(const char* value)
        {
            if (!value) throw std::invalid_argument("Null string pointer");
            return operator=(std::string_view(value));
        }

        MappingNode& operator=(const IMappable& obj)
        {
            Clear();
            m_type = Type::Mapping;
            obj.Map(*this);
            return *this;
        }

        // Numeric assignment
        template<typename T>
        std::enable_if_t<std::is_arithmetic_v<T>, MappingNode&>
            operator=(const T& value)
        {
            try
            {
                Clear();
                if constexpr (std::is_same_v<T, bool>)
                {
                    m_type = Type::Boolean;
                    m_data.resize(sizeof(T));
                    std::memcpy(m_data.data(), &value, sizeof(T));
                }
                else if constexpr (std::is_floating_point_v<T>)
                {
                    // Check if the float is actually a whole number
                    double intpart;
                    if (std::modf(static_cast<double>(value), &intpart) == 0.0)
                    {
                        // It's a whole number, store as integer
                        m_type = Type::Numeric;
                        if (intpart < 0)
                        {
                            m_type = static_cast<Type>(static_cast<uint8_t>(m_type) |
                                static_cast<uint8_t>(Type::Negative));
                        }

                        // Choose the smallest type that can hold the value
                        if (intpart >= std::numeric_limits<int32_t>::min() &&
                            intpart <= std::numeric_limits<int32_t>::max())
                        {
                            int32_t int_val = static_cast<int32_t>(intpart);
                            m_data.resize(sizeof(int32_t));
                            std::memcpy(m_data.data(), &int_val, sizeof(int32_t));
                        }
                        else
                        {
                            int64_t int_val = static_cast<int64_t>(intpart);
                            m_data.resize(sizeof(int64_t));
                            std::memcpy(m_data.data(), &int_val, sizeof(int64_t));
                        }
                    }
                    else
                    {
                        // It's a true decimal, store as float
                        m_type = static_cast<Type>(static_cast<uint8_t>(Type::Numeric) |
                            static_cast<uint8_t>(Type::Decimal));
                        if (value < 0)
                        {
                            m_type = static_cast<Type>(static_cast<uint8_t>(m_type) |
                                static_cast<uint8_t>(Type::Negative));
                        }

                        if (sizeof(T) > sizeof(float))
                        {
                            // Store as double for better precision
                            double double_val = static_cast<double>(value);
                            m_data.resize(sizeof(double));
                            std::memcpy(m_data.data(), &double_val, sizeof(double));
                        }
                        else
                        {
                            float float_val = static_cast<float>(value);
                            m_data.resize(sizeof(float));
                            std::memcpy(m_data.data(), &float_val, sizeof(float));
                        }
                    }
                }
                else
                {
                    // Original integer handling
                    m_type = Type::Numeric;
                    if constexpr (std::is_signed_v<T>)
                    {
                        if (value < 0)
                        {
                            m_type = static_cast<Type>(static_cast<uint8_t>(m_type) |
                                static_cast<uint8_t>(Type::Negative));
                        }
                    }
                    m_data.resize(sizeof(T));
                    std::memcpy(m_data.data(), &value, sizeof(T));
                }
                return *this;
            }
            catch (...)
            {
                Clear();
                throw;
            }
        }

        // Type conversion
        template<typename T>
        std::enable_if_t<!std::is_base_of_v<IMappable, T>, T>
            As() const
        {
            if (!IsScalar())
            {
                throw std::runtime_error("Node is not a scalar");
            }

            if constexpr (std::is_base_of_v<std::string, T>)
            {
                if (m_type != Type::String)
                {
                    throw std::runtime_error("Type mismatch: not a string");
                }
                T result;
                result.resize(m_data.size());
                std::memcpy(result.data(), m_data.data(), m_data.size());
                return result;
            }
            else
            {
                return AsNumeric<T>();
            }
        }

        template<typename T>
        std::enable_if_t<std::is_base_of_v<IMappable, T>, T>
            As() const
        {
            return AsMappable<T>();
        }

        // Array operations
        void PushBack(const MappingNode& node)
        {
            if (!IsArray())
            {
                if (IsMapping())
                {
                    throw std::runtime_error("Cannot PushBack to a Mapping node");
                }
                Clear();
                m_type = Type::Array;
            }

            MappingNode newNode = node;
            newNode.m_key.clear();
            m_children.push_back(std::move(newNode));
        }

        template<typename T>
        void PushBack(const T& value)
        {
            MappingNode node;
            node = value;
            PushBack(node);
        }

        // Access operations with bounds checking
        MappingNode& operator[](const std::string& key)
        {
            if (!IsMapping())
            {
                if (IsArray())
                {
                    throw std::runtime_error("Cannot use string key with Array node");
                }
                Clear();
                m_type = Type::Mapping;
            }

            auto it = m_keyMap.find(std::string(key));
            if (it != m_keyMap.end())
            {
                return m_children[it->second];
            }

            m_keyMap[std::string(key)] = m_children.size();
            m_children.emplace_back();
            m_children.back().m_key = std::string(key);
            return m_children.back();
        }

        const MappingNode& operator[](const std::string& key) const
        {
            if (!IsMapping())
            {
                throw std::runtime_error("Node is not a mapping");
            }

            auto it = m_keyMap.find(std::string(key));
            if (it == m_keyMap.end())
            {
                static const MappingNode null_node;
                return null_node;
            }

            return m_children.at(it->second);
        }

        MappingNode& operator[](size_t index)
        {
            if (!IsMapping() && !IsArray())
            {
                throw std::runtime_error("Node is not an array or mapping");
            }
            return m_children.at(index);
        }

        const MappingNode& operator[](size_t index) const
        {
            if (!IsMapping() && !IsArray())
            {
                throw std::runtime_error("Node is not an array or mapping");
            }
            return m_children.at(index);
        }

        // Query operations
        bool ContainsKey(std::string_view key) const noexcept
        {
            return IsMapping() && m_keyMap.find(std::string(key)) != m_keyMap.end();
        }

        std::string_view Key() const noexcept
        {
            return m_key;
        }

        // State management
        void Clear() noexcept
        {
            m_type = Type::Null;
            m_data.clear();
            m_children.clear();
            m_keyMap.clear();
        }

        void Trim()
        {
            for (auto iter = m_children.begin(); iter != m_children.end();)
            {
                if (iter->IsDefined())
                {
                    iter->Trim();
                    ++iter;
                }
                else
                {
                    m_keyMap.erase(iter->m_key);
                    iter = m_children.erase(iter);
                }
            }
        }

        // Type checking methods
        bool IsDefined() const noexcept { return m_type != Type::Null; }
        bool IsScalar() const noexcept
        {
            return m_type != Type::Array && m_type != Type::Mapping && m_type != Type::Null;
        }
        bool IsArray() const noexcept { return m_type == Type::Array; }
        bool IsMapping() const noexcept { return m_type == Type::Mapping; }
        bool IsString() const noexcept { return m_type == Type::String; }
        bool IsBoolean() const noexcept { return m_type == Type::Boolean; }
        bool IsNumeric() const noexcept
        {
            return (static_cast<uint8_t>(m_type) &
                ~static_cast<uint8_t>(Type::FlagMask)) ==
                static_cast<uint8_t>(Type::Numeric);
        }
        bool HasDecimal() const noexcept
        {
            return IsNumeric() && (static_cast<uint8_t>(m_type) &
                static_cast<uint8_t>(Type::Decimal));
        }
        bool IsNegative() const noexcept
        {
            return IsNumeric() && (static_cast<uint8_t>(m_type) &
                static_cast<uint8_t>(Type::Negative));
        }

        //Size in bytes of stored scalar data
        size_t Width() const
        {
            if (!IsScalar())
            {
                throw std::runtime_error("Cannot get width of non-scalar type. Use 'Size()' if looking for map or array length.");
            }
            return m_data.size();
        }

        // Length of Map or Array
        size_t Size() const
        {
            if (!IsMapping() && !IsArray())
            {
                throw std::runtime_error("Cannot get width of non-map/array type. Use 'Width()' if looking for data width.");
            }
            return m_children.size();
        }

        // Iterator support
        using iterator = typename std::vector<MappingNode>::iterator;
        using const_iterator = typename std::vector<MappingNode>::const_iterator;

        iterator begin() noexcept { return m_children.begin(); }
        iterator end() noexcept { return m_children.end(); }
        const_iterator begin() const noexcept { return m_children.begin(); }
        const_iterator end() const noexcept { return m_children.end(); }
        const_iterator cbegin() const noexcept { return m_children.cbegin(); }
        const_iterator cend() const noexcept { return m_children.cend(); }

    private:
        template<typename T>
        T AsNumeric() const
        {
            if (!CanCast<T>())
            {
                throw std::runtime_error("Invalid numeric cast");
            }

            if constexpr (std::is_same_v<T, bool>)
            {
                if (!IsBoolean())
                {
                    throw std::runtime_error("Node is not a boolean");
                }
                T result;
                std::memcpy(&result, m_data.data(), sizeof(T));
                return result;
            }

            // Handle conversion to floating point types
            if constexpr (std::is_floating_point_v<T>)
            {
                if (HasDecimal())
                {
                    return AsFloatingPoint<T>();
                }

                // Handle integer to float conversion
                if (IsNegative())
                {
                    switch (m_data.size())
                    {
                        case sizeof(int8_t) : {
                            int8_t temp;
                            std::memcpy(&temp, m_data.data(), sizeof(int8_t));
                            return static_cast<T>(temp);
                        }
                        case sizeof(int16_t) : {
                            int16_t temp;
                            std::memcpy(&temp, m_data.data(), sizeof(int16_t));
                            return static_cast<T>(temp);
                        }
                        case sizeof(int32_t) : {
                            int32_t temp;
                            std::memcpy(&temp, m_data.data(), sizeof(int32_t));
                            return static_cast<T>(temp);
                        }
                        case sizeof(int64_t) : {
                            int64_t temp;
                            std::memcpy(&temp, m_data.data(), sizeof(int64_t));
                            return static_cast<T>(temp);
                        }
                        default:
                            throw std::runtime_error("Invalid integer size");
                    }
                }
                else
                {
                    switch (m_data.size())
                    {
                        case sizeof(uint8_t) : {
                            uint8_t temp;
                            std::memcpy(&temp, m_data.data(), sizeof(uint8_t));
                            return static_cast<T>(temp);
                        }
                        case sizeof(uint16_t) : {
                            uint16_t temp;
                            std::memcpy(&temp, m_data.data(), sizeof(uint16_t));
                            return static_cast<T>(temp);
                        }
                        case sizeof(uint32_t) : {
                            uint32_t temp;
                            std::memcpy(&temp, m_data.data(), sizeof(uint32_t));
                            return static_cast<T>(temp);
                        }
                        case sizeof(uint64_t) : {
                            uint64_t temp;
                            std::memcpy(&temp, m_data.data(), sizeof(uint64_t));
                            return static_cast<T>(temp);
                        }
                        default:
                            throw std::runtime_error("Invalid integer size");
                    }
                }
            }

            return AsInteger<T>();
        }

        template<typename T>
        T AsFloatingPoint() const
        {
            if (!HasDecimal())
            {
                if (IsNegative())
                {
                    int64_t temp;
                    std::memcpy(&temp, m_data.data(), sizeof(int64_t));
                    return static_cast<T>(temp);
                }
                uint64_t temp;
                std::memcpy(&temp, m_data.data(), sizeof(uint64_t));
                return static_cast<T>(temp);
            }

            if (m_data.size() > sizeof(float))
            {
                // First get the value as a double
                double value;
                std::memcpy(&value, m_data.data(), sizeof(double));

                if constexpr (std::is_same_v<T, float>)
                {
                    // Check if value is within float range
                    if (value > std::numeric_limits<float>::max() ||
                        value < std::numeric_limits<float>::lowest())
                    {
                        throw std::runtime_error("Value exceeds float capacity");
                    }
                    return static_cast<float>(value);
                }
                else
                {
                    return static_cast<T>(value);
                }
            }
            else
            {
                float value;
                std::memcpy(&value, m_data.data(), sizeof(float));
                return static_cast<T>(value);
            }
        }

        template<typename T>
        T AsInteger() const
        {
            if (HasDecimal())
            {
                throw std::runtime_error("Cannot convert decimal to integer");
            }

            if (IsNegative() && std::is_unsigned_v<T>)
            {
                throw std::runtime_error("Cannot convert negative to unsigned");
            }

            // Handle sign extension for signed integers
            if (IsNegative())
            {
                // For negative numbers, first load into the appropriate sized signed integer
                switch (m_data.size())
                {
                    case sizeof(int8_t) : {
                        int8_t temp;
                        std::memcpy(&temp, m_data.data(), sizeof(int8_t));
                        return static_cast<T>(temp);
                    }
                    case sizeof(int16_t) : {
                        int16_t temp;
                        std::memcpy(&temp, m_data.data(), sizeof(int16_t));
                        return static_cast<T>(temp);
                    }
                    case sizeof(int32_t) : {
                        int32_t temp;
                        std::memcpy(&temp, m_data.data(), sizeof(int32_t));
                        return static_cast<T>(temp);
                    }
                    case sizeof(int64_t) : {
                        int64_t temp;
                        std::memcpy(&temp, m_data.data(), sizeof(int64_t));
                        return static_cast<T>(temp);
                    }
                    default:
                        throw std::runtime_error("Invalid integer size");
                }
            }
            else
            {
                // For positive numbers, first load into the appropriate sized unsigned integer
                switch (m_data.size())
                {
                    case sizeof(uint8_t) : {
                        uint8_t temp;
                        std::memcpy(&temp, m_data.data(), sizeof(uint8_t));
                        return static_cast<T>(temp);
                    }
                    case sizeof(uint16_t) : {
                        uint16_t temp;
                        std::memcpy(&temp, m_data.data(), sizeof(uint16_t));
                        return static_cast<T>(temp);
                    }
                    case sizeof(uint32_t) : {
                        uint32_t temp;
                        std::memcpy(&temp, m_data.data(), sizeof(uint32_t));
                        return static_cast<T>(temp);
                    }
                    case sizeof(uint64_t) : {
                        uint64_t temp;
                        std::memcpy(&temp, m_data.data(), sizeof(uint64_t));
                        if (std::is_signed_v<T> && temp > static_cast<uint64_t>(std::numeric_limits<T>::max())) {
                            throw std::runtime_error("Integer overflow");
                        }
                        return static_cast<T>(temp);
                    }
                    default:
                        throw std::runtime_error("Invalid integer size");
                }
            }
        }

        template<typename T>
        T AsMappable() const
        {
            T result;
            result.Unmap(*this);
            return result;
        }

        template<typename T>
        bool CanCast() const noexcept
        {
            if (!IsNumeric() && !IsBoolean()) return false;

            if constexpr (std::is_same_v<T, bool>)
            {
                return IsBoolean();
            }

            if (HasDecimal())
            {
                return std::is_floating_point_v<T>;
            }

            if (IsNegative() && std::is_unsigned_v<T>)
            {
                return false;
            }

            return m_data.size() <= sizeof(T);
        }

        Type m_type;
        std::string m_key;
        std::vector<uint8_t> m_data;
        std::vector<MappingNode> m_children;
        std::unordered_map<std::string, size_t> m_keyMap;
    };
}

#endif // !XE_MAPPINGNODE_H