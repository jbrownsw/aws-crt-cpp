/*
 * Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <aws/crt/JsonObject.h>

#include <aws/crt/external/cJSON.h>

#include <algorithm>
#include <iterator>

namespace Aws
{
    namespace Crt
    {
        JsonObject::JsonObject(Allocator *a)
            : m_allocator(a), m_wasParseSuccessful(true), m_errorMessage(StlAllocator<char>(a))
        {
            m_value = nullptr;
        }

        JsonObject::JsonObject(cJSON *value, Allocator *a)
            : m_allocator(a), m_value(cJSON_Duplicate(value, 1 /* recurse */)), m_wasParseSuccessful(true),
              m_errorMessage(StlAllocator<char>(a))
        {
        }

        JsonObject::JsonObject(const String &value, Allocator *a)
            : m_allocator(a), m_wasParseSuccessful(true), m_errorMessage(StlAllocator<char>(a))
        {
            const char *return_parse_end;
            m_value = cJSON_ParseWithOpts(value.c_str(), value.length(), &return_parse_end);

            if (m_value == nullptr || cJSON_IsInvalid(m_value) == 1)
            {
                m_wasParseSuccessful = false;
                m_errorMessage = "Failed to parse JSON at: ";
                m_errorMessage += return_parse_end;
            }
        }

        JsonObject::JsonObject(const JsonObject &value)
            : m_allocator(value.m_allocator), m_value(cJSON_Duplicate(value.m_value, 1 /*recurse*/)),
              m_wasParseSuccessful(value.m_wasParseSuccessful), m_errorMessage(value.m_errorMessage)
        {
        }

        JsonObject::JsonObject(JsonObject &&value) noexcept
            : m_allocator(value.m_allocator), m_value(value.m_value), m_wasParseSuccessful(value.m_wasParseSuccessful),
              m_errorMessage(std::move(value.m_errorMessage))
        {
            value.m_value = nullptr;
        }

        void JsonObject::Destroy() { cJSON_Delete(m_value); }

        JsonObject::~JsonObject() { Destroy(); }

        JsonObject &JsonObject::operator=(const JsonObject &other)
        {
            if (this == &other)
            {
                return *this;
            }

            Destroy();
            m_value = cJSON_Duplicate(other.m_value, 1 /*recurse*/);
            m_wasParseSuccessful = other.m_wasParseSuccessful;
            m_errorMessage = other.m_errorMessage;
            return *this;
        }

        JsonObject &JsonObject::operator=(JsonObject &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            using std::swap;
            swap(m_value, other.m_value);
            swap(m_errorMessage, other.m_errorMessage);
            m_wasParseSuccessful = other.m_wasParseSuccessful;
            return *this;
        }

        static void AddOrReplace(cJSON *root, const char *key, cJSON *value)
        {
            const auto existing = cJSON_GetObjectItemCaseSensitive(root, key);
            if (existing != nullptr)
            {
                cJSON_ReplaceItemInObjectCaseSensitive(root, key, value);
            }
            else
            {
                cJSON_AddItemToObject(root, key, value);
            }
        }

        JsonObject &JsonObject::WithString(const char *key, const String &value)
        {
            if (m_value == nullptr)
            {
                m_value = cJSON_CreateObject();
            }

            const auto val = cJSON_CreateString(value.c_str());
            AddOrReplace(m_value, key, val);
            return *this;
        }

        JsonObject &JsonObject::WithString(const String &key, const String &value)
        {
            return WithString(key.c_str(), value);
        }

        JsonObject &JsonObject::AsString(const String &value)
        {
            Destroy();
            m_value = cJSON_CreateString(value.c_str());
            return *this;
        }

        JsonObject &JsonObject::WithBool(const char *key, bool value)
        {
            if (m_value == nullptr)
            {
                m_value = cJSON_CreateObject();
            }

            const auto val = cJSON_CreateBool((cJSON_bool)value);
            AddOrReplace(m_value, key, val);
            return *this;
        }

        JsonObject &JsonObject::WithBool(const String &key, bool value) { return WithBool(key.c_str(), value); }

        JsonObject &JsonObject::AsBool(bool value)
        {
            Destroy();
            m_value = cJSON_CreateBool((cJSON_bool)value);
            return *this;
        }

        JsonObject &JsonObject::WithInteger(const char *key, int value)
        {
            return WithDouble(key, static_cast<double>(value));
        }

        JsonObject &JsonObject::WithInteger(const String &key, int value)
        {
            return WithDouble(key.c_str(), static_cast<double>(value));
        }

        JsonObject &JsonObject::AsInteger(int value)
        {
            Destroy();
            m_value = cJSON_CreateNumber(static_cast<double>(value));
            return *this;
        }

        JsonObject &JsonObject::WithInt64(const char *key, int64_t value)
        {
            return WithDouble(key, static_cast<double>(value));
        }

        JsonObject &JsonObject::WithInt64(const String &key, int64_t value)
        {
            return WithDouble(key.c_str(), static_cast<double>(value));
        }

        JsonObject &JsonObject::AsInt64(int64_t value) { return AsDouble(static_cast<double>(value)); }

        JsonObject &JsonObject::WithDouble(const char *key, double value)
        {
            if (m_value == nullptr)
            {
                m_value = cJSON_CreateObject();
            }

            const auto val = cJSON_CreateNumber(value);
            AddOrReplace(m_value, key, val);
            return *this;
        }

        JsonObject &JsonObject::WithDouble(const String &key, double value) { return WithDouble(key.c_str(), value); }

        JsonObject &JsonObject::AsDouble(double value)
        {
            Destroy();
            m_value = cJSON_CreateNumber(value);
            return *this;
        }

        JsonObject &JsonObject::WithArray(const char *key, const Vector<String> &array)
        {
            if (m_value == nullptr)
            {
                m_value = cJSON_CreateObject();
            }

            auto arrayValue = cJSON_CreateArray();
            for (const auto &i : array)
            {
                cJSON_AddItemToArray(arrayValue, cJSON_CreateString(i.c_str()));
            }

            AddOrReplace(m_value, key, arrayValue);
            return *this;
        }

        JsonObject &JsonObject::WithArray(const String &key, const Vector<String> &array)
        {
            return WithArray(key.c_str(), array);
        }

        JsonObject &JsonObject::WithArray(const String &key, const Vector<JsonObject> &array)
        {
            if (m_value == nullptr)
            {
                m_value = cJSON_CreateObject();
            }

            auto arrayValue = cJSON_CreateArray();
            for (const auto &i : array)
            {
                cJSON_AddItemToArray(arrayValue, cJSON_Duplicate(i.m_value, 1 /*recurse*/));
            }

            AddOrReplace(m_value, key.c_str(), arrayValue);
            return *this;
        }

        JsonObject &JsonObject::WithArray(const String &key, Vector<JsonObject> &&array)
        {
            if (m_value == nullptr)
            {
                m_value = cJSON_CreateObject();
            }

            auto arrayValue = cJSON_CreateArray();
            for (auto &i : array)
            {
                cJSON_AddItemToArray(arrayValue, i.m_value);
                i.m_value = nullptr;
            }

            AddOrReplace(m_value, key.c_str(), arrayValue);
            return *this;
        }

        JsonObject &JsonObject::AsArray(const Vector<JsonObject> &array)
        {
            auto arrayValue = cJSON_CreateArray();
            for (const auto &i : array)
            {
                cJSON_AddItemToArray(arrayValue, cJSON_Duplicate(i.m_value, 1 /*recurse*/));
            }

            Destroy();
            m_value = arrayValue;
            return *this;
        }

        JsonObject &JsonObject::AsArray(Vector<JsonObject> &&array)
        {
            auto arrayValue = cJSON_CreateArray();
            for (auto &i : array)
            {
                cJSON_AddItemToArray(arrayValue, i.m_value);
                i.m_value = nullptr;
            }

            Destroy();
            m_value = arrayValue;
            return *this;
        }

        JsonObject &JsonObject::AsNull()
        {
            m_value = cJSON_CreateNull();
            return *this;
        }

        JsonObject &JsonObject::WithObject(const char *key, const JsonObject &value)
        {
            if (m_value == nullptr)
            {
                m_value = cJSON_CreateObject();
            }

            const auto copy =
                value.m_value == nullptr ? cJSON_CreateObject() : cJSON_Duplicate(value.m_value, 1 /*recurse*/);
            AddOrReplace(m_value, key, copy);
            return *this;
        }

        JsonObject &JsonObject::WithObject(const String &key, const JsonObject &value)
        {
            return WithObject(key.c_str(), value);
        }

        JsonObject &JsonObject::WithObject(const char *key, JsonObject &&value)
        {
            if (m_value == nullptr)
            {
                m_value = cJSON_CreateObject();
            }

            AddOrReplace(m_value, key, value.m_value == nullptr ? cJSON_CreateObject() : value.m_value);
            value.m_value = nullptr;
            return *this;
        }

        JsonObject &JsonObject::WithObject(const String &key, JsonObject &&value)
        {
            return WithObject(key.c_str(), std::move(value));
        }

        JsonObject &JsonObject::AsObject(const JsonObject &value)
        {
            *this = value;
            return *this;
        }

        JsonObject &JsonObject::AsObject(JsonObject &&value)
        {
            *this = std::move(value);
            return *this;
        }

        bool JsonObject::operator==(const JsonObject &other) const
        {
            return cJSON_Compare(m_value, other.m_value, 1 /*case-sensitive*/) != 0;
        }

        bool JsonObject::operator!=(const JsonObject &other) const { return !(*this == other); }

        JsonView JsonObject::View() const { return *this; }

        JsonView::JsonView(Allocator *a) : m_allocator(a), m_value(nullptr) {}

        JsonView::JsonView(const JsonObject &val) : m_allocator(val.m_allocator), m_value(val.m_value) {}

        JsonView::JsonView(cJSON *val, Allocator *a) : m_allocator(a), m_value(val) {}

        JsonView &JsonView::operator=(const JsonObject &v)
        {
            m_value = v.m_value;
            return *this;
        }

        JsonView &JsonView::operator=(cJSON *val)
        {
            m_value = val;
            return *this;
        }

        String JsonView::GetString(const String &key) const { return GetString(key.c_str()); }

        String JsonView::GetString(const char *key) const
        {
            AWS_ASSERT(m_value);
            auto item = cJSON_GetObjectItemCaseSensitive(m_value, key);
            auto str = cJSON_GetStringValue(item);
            return String(str != nullptr ? str : "", StlAllocator<char>(m_allocator));
        }

        String JsonView::AsString() const
        {
            const char *str = cJSON_GetStringValue(m_value);
            return String(str != nullptr ? str : "", StlAllocator<char>(m_allocator));
        }

        bool JsonView::GetBool(const String &key) const { return GetBool(key.c_str()); }

        bool JsonView::GetBool(const char *key) const
        {
            AWS_ASSERT(m_value);
            auto item = cJSON_GetObjectItemCaseSensitive(m_value, key);
            AWS_ASSERT(item);
            return item->valueint != 0;
        }

        bool JsonView::AsBool() const
        {
            AWS_ASSERT(cJSON_IsBool(m_value));
            return cJSON_IsTrue(m_value) != 0;
        }

        int JsonView::GetInteger(const String &key) const { return GetInteger(key.c_str()); }

        int JsonView::GetInteger(const char *key) const
        {
            AWS_ASSERT(m_value);
            auto item = cJSON_GetObjectItemCaseSensitive(m_value, key);
            AWS_ASSERT(item);
            return item->valueint;
        }

        int JsonView::AsInteger() const
        {
            AWS_ASSERT(cJSON_IsNumber(m_value)); // can be double or value larger than int_max, but at least not UB
            return m_value->valueint;
        }

        int64_t JsonView::GetInt64(const String &key) const { return static_cast<int64_t>(GetDouble(key)); }

        int64_t JsonView::GetInt64(const char *key) const { return static_cast<int64_t>(GetDouble(key)); }

        int64_t JsonView::AsInt64() const
        {
            AWS_ASSERT(cJSON_IsNumber(m_value));
            return static_cast<int64_t>(m_value->valuedouble);
        }

        double JsonView::GetDouble(const String &key) const { return GetDouble(key.c_str()); }

        double JsonView::GetDouble(const char *key) const
        {
            AWS_ASSERT(m_value);
            auto item = cJSON_GetObjectItemCaseSensitive(m_value, key);
            AWS_ASSERT(item);
            return item->valuedouble;
        }

        double JsonView::AsDouble() const
        {
            AWS_ASSERT(cJSON_IsNumber(m_value));
            return m_value->valuedouble;
        }

        JsonView JsonView::GetJsonObject(const String &key) const { return GetJsonObject(key.c_str()); }

        JsonView JsonView::GetJsonObject(const char *key) const
        {
            AWS_ASSERT(m_value);
            auto item = cJSON_GetObjectItemCaseSensitive(m_value, key);
            return item;
        }

        JsonObject JsonView::GetJsonObjectCopy(const String &key) const { return GetJsonObjectCopy(key.c_str()); }

        JsonObject JsonView::GetJsonObjectCopy(const char *key) const
        {
            AWS_ASSERT(m_value);
            /* force a deep copy */
            return JsonObject(cJSON_GetObjectItemCaseSensitive(m_value, key), m_allocator);
        }

        JsonView JsonView::AsObject() const
        {
            AWS_ASSERT(cJSON_IsObject(m_value));
            return m_value;
        }

        Vector<JsonView> JsonView::GetArray(const String &key) const { return GetArray(key.c_str()); }

        Vector<JsonView> JsonView::GetArray(const char *key) const
        {
            AWS_ASSERT(m_value);
            auto array = cJSON_GetObjectItemCaseSensitive(m_value, key);
            AWS_ASSERT(cJSON_IsArray(array));
            Vector<JsonView> returnArray(
                static_cast<size_t>(cJSON_GetArraySize(array)), StlAllocator<JsonView>(m_allocator));

            auto element = array->child;
            for (size_t i = 0; element != nullptr && i < returnArray.size(); ++i, element = element->next)
            {
                returnArray[i] = element;
            }

            return returnArray;
        }

        Vector<JsonView> JsonView::AsArray() const
        {
            AWS_ASSERT(cJSON_IsArray(m_value));
            Vector<JsonView> returnArray(
                static_cast<size_t>(cJSON_GetArraySize(m_value)), StlAllocator<JsonView>(m_allocator));

            auto element = m_value->child;

            for (size_t i = 0; element != nullptr && i < returnArray.size(); ++i, element = element->next)
            {
                returnArray[i] = element;
            }

            return returnArray;
        }

        Map<String, JsonView> JsonView::GetAllObjects() const
        {
            StlAllocator<std::pair<const String, JsonView>> allocator(m_allocator);
            Map<String, JsonView> valueMap(allocator);
            if (m_value == nullptr)
            {
                return valueMap;
            }

            for (auto iter = m_value->child; iter != nullptr; iter = iter->next)
            {
                valueMap.emplace(std::make_pair(String(iter->string, allocator), JsonView(iter)));
            }

            return valueMap;
        }

        bool JsonView::ValueExists(const String &key) const { return ValueExists(key.c_str()); }

        bool JsonView::ValueExists(const char *key) const
        {
            if (cJSON_IsObject(m_value) == 0)
            {
                return false;
            }

            auto item = cJSON_GetObjectItemCaseSensitive(m_value, key);
            return !(item == nullptr || cJSON_IsNull(item) != 0);
        }

        bool JsonView::KeyExists(const String &key) const { return KeyExists(key.c_str()); }

        bool JsonView::KeyExists(const char *key) const
        {
            if (cJSON_IsObject(m_value) == 0)
            {
                return false;
            }

            return cJSON_GetObjectItemCaseSensitive(m_value, key) != nullptr;
        }

        bool JsonView::IsObject() const { return cJSON_IsObject(m_value) != 0; }

        bool JsonView::IsBool() const { return cJSON_IsBool(m_value) != 0; }

        bool JsonView::IsString() const { return cJSON_IsString(m_value) != 0; }

        bool JsonView::IsIntegerType() const
        {
            if (cJSON_IsNumber(m_value) == 0)
            {
                return false;
            }

            return m_value->valuedouble == static_cast<int64_t>(m_value->valuedouble);
        }

        bool JsonView::IsFloatingPointType() const
        {
            if (cJSON_IsNumber(m_value) == 0)
            {
                return false;
            }

            return m_value->valuedouble != static_cast<int64_t>(m_value->valuedouble);
        }

        bool JsonView::IsListType() const { return cJSON_IsArray(m_value) != 0; }

        bool JsonView::IsNull() const { return cJSON_IsNull(m_value) != 0; }

        String JsonView::WriteCompact(bool treatAsObject) const
        {
            StlAllocator<char> allocator(m_allocator);

            if (m_value == nullptr)
            {
                if (treatAsObject)
                {
                    return String("{}", allocator);
                }
                return String("", allocator);
            }

            auto temp = cJSON_PrintUnformatted(m_value);
            String out(temp, allocator);
            cJSON_free(temp);
            return out;
        }

        String JsonView::WriteReadable(bool treatAsObject) const
        {
            StlAllocator<char> allocator(m_allocator);

            if (m_value == nullptr)
            {
                if (treatAsObject)
                {
                    return String("{\n}\n", allocator);
                }
                return String("", allocator);
            }

            auto temp = cJSON_Print(m_value);
            String out(temp, allocator);
            cJSON_free(temp);
            return out;
        }

        JsonObject JsonView::Materialize() const { return JsonObject(m_value, m_allocator); }
    } // namespace Crt
} // namespace Aws
