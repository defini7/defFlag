#ifndef DEF_FLAG_HPP
#define DEF_FLAG_HPP

#pragma region License
/**
    BSD 3-Clause License

    Copyright (c) 2024, Alex

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/
#pragma endregion // License

#pragma region Includes

#include <string>
#include <variant>
#include <optional>
#include <unordered_map>
#include <stdexcept>
#include <iostream>

#pragma endregion // Includes

#define DEF_FLAG_CHECK_TRUE_BOOLEAN(v) (v == "" || v == "true" || v == "True" || v == "TRUE" || v == "t" || v == "T" || v == "1")
#define DEF_FLAG_CHECK_FALSE_BOOLEAN(v) (v == "false" || v == "False" || v == "FALSE" || v == "f" || v == "F" || v == "0")

namespace def
{
    class Flag
    {
    public:
        struct Entry
        {
            using Type = std::variant<bool, float, double, int32_t, int64_t, uint32_t, uint64_t, std::string>;

            std::string usage;
            Type value;
            Type defaultValue;
        };

        struct Exception : std::exception
        {
        public:
            Exception(const std::string& text) : std::exception(text.c_str()) {}
        };

    public:
        Flag() = default;

        size_t Parse(const size_t argumentsCount, char* arguments[], const bool ignoreUnexpectedFlags = true, const bool startFromNext = true);

        bool& Set(const std::string& name, const bool defaultValue, const std::string& usage);
        std::string& Set(const std::string& name, const char* defaultValue, const std::string& usage);

        float& Set(const std::string& name, const float defaultValue, const std::string& usage);
        double& Set(const std::string& name, const double defaultValue, const std::string& usage);

        int32_t& Set(const std::string& name, const int32_t defaultValue, const std::string& usage);
        int64_t& Set(const std::string& name, const int64_t defaultValue, const std::string& usage);

        uint32_t& Set(const std::string& name, const uint32_t defaultValue, const std::string& usage);
        uint64_t& Set(const std::string& name, const uint64_t defaultValue, const std::string& usage);

        std::optional<std::reference_wrapper<const Entry>> Lookup(const std::string& name);
        void PrintUsage(std::ostream& os = std::cout);

    private:
        enum class ParserState
        {
            NewToken,
            Name,
            Value
        };

        template <class... Ts> struct Overload : Ts... { using Ts::operator()...; };

        void MakeTrueOnBool(const std::string& name);

        template <class Out, class Return>
        void ParseInteger(const std::string& input, Out& output, const std::string& name, Return (*convert)(const std::string&, size_t*, int), bool isSigned);

    private:
        std::unordered_map<std::string, Entry> m_Entries;

    };
}

#ifdef DEF_FLAG_IMPL
#undef DEF_FLAG_IMPL

#define CHECK_VALUE(target, source, name) try { target = source; } catch (const std::exception& e) { (void)e; throw Exception("[defFlag] Value of an unexpected type for the " + name + " flag"); }

size_t def::Flag::Parse(const size_t argumentsCount, char* arguments[], const bool ignoreUnexpectedFlags, const bool startFromNext)
{
    if (!arguments || argumentsCount == 1 && startFromNext)
        throw Exception("[defFlag] No arguments were provided");

    ParserState stateNow = ParserState::NewToken;
    ParserState stateNext = stateNow;

    std::string name;
    std::string value;

    size_t i = startFromNext ? 1 : 0;

    if (strcmp(arguments[i], "-h") == 0 || strcmp(arguments[i], "--help") == 0 || strcmp(arguments[i], "-help") == 0)
    {
        PrintUsage();
        i++;
    }

    for (; i < argumentsCount; i++)
    {
        char* arg = arguments[i];

        if (!arg)
        {
            // The user tried to decieve us... MHA-HA-HA
            throw Exception("[defFlag] Tried to read an argument outside of the array, you probably need to change argumentsCount argument");
        }

        size_t len = strlen(arg);

        for (size_t j = 0; j < len; j++)
        {
            switch (stateNow)
            {
            case ParserState::NewToken:
            {
                if (arg[j] == '-')
                {
                    // That is definetly must be a flag name
                    stateNext = ParserState::Name;
                }
                else
                {
                    // Now we have something like this: -flag a b c d
                    // so return an index of the first argument in a tail
                    return i;
                }
            }
            break;

            case ParserState::Name:
            {
                if (arg[j] == '-' && j == 1)
                {
                    // Skip second dash
                    stateNext = ParserState::Name;
                }
                else
                {
                    if (arg[j] == '=')
                        stateNext = ParserState::Value;
                    else
                    {
                        name += arg[j];
                        stateNext = ParserState::Name;
                    }
                }
            }
            break;

            case ParserState::Value:
            {
                if (j == 0 && arg[j] == '-')
                {
                    // We have reached the point where we specified something like this
                    // on the previous chunk: -previous_flag -current_flag.
                    // So to solve this problem we jump to the ParserState::Name and
                    // if it was a boolean flag set it to true
                    MakeTrueOnBool(name);
                    name.clear();
                    
                    stateNext = ParserState::Name;
                }
                else
                {
                    // Otherwise we have something like this: "-flag value" or "-flag=value"
                    value += arg[j];
                }

                // It's not necessary because stateNext is always
                // ParserState::Value at this point but let's leave it for clarity
                // stateNext = ParserState::Value;
            }
            break;

            }

            stateNow = stateNext;
        }

        switch (stateNow)
        {

        // Check if we've just only parsed the name (e.g.: -name ...)
        case ParserState::Name:
        {
            if (name.empty())
            {
                // A dash is the only character in the current chunk
                throw Exception("[defFlag] You can't do '- ...'");
            }

            // Check if we have a flag with the such name
            if (m_Entries.contains(name))
                stateNext = ParserState::Value;
            else
            {
                if (ignoreUnexpectedFlags)
                {
                    name.clear();
                    stateNext = ParserState::NewToken;
                }
                else
                    throw Exception("[defFlag] Unexpected flag: " + name);
            }
        }
        break;

        // Check if we've just finished parsing the value
        case ParserState::Value:
        {
            // At this point we assume that we have the name

            if (value.empty())
                throw Exception("[defFlag] No value was provided after '='");

            if (!m_Entries.contains(name))
                throw Exception("[defFlag] Can't find the such flag: " + name);
            
            // We have the such flag so save the value for it
            std::visit(Overload {
                [&](bool& v) {
                    if (DEF_FLAG_CHECK_TRUE_BOOLEAN(value))
                        v = true;
                    else if (DEF_FLAG_CHECK_FALSE_BOOLEAN(value))
                        v = false;
                    else
                        throw Exception("[defFlag] Value of an unexpected type for the " + name + " flag");
                },
                [&](float& v) { CHECK_VALUE(v, std::stof(value), name) },
                [&](double& v) { CHECK_VALUE(v, std::stod(value), name) },
                [&](int32_t& v) { ParseInteger(value, v, name, std::stoi, true); },
                [&](int64_t& v) { ParseInteger(value, v, name, std::stoll, true); },
                [&](uint32_t& v) { ParseInteger(value, v, name, std::stoul, false); },
                [&](uint64_t& v) { ParseInteger(value, v, name, std::stoull, false); },
                [&](std::string& v) { v = value; },
                [&](auto& v) { (void)v; throw Exception("[defFlag] Unreachable"); },
            }, m_Entries[name].value);

            name.clear();
            value.clear();

            stateNext = ParserState::NewToken;
        }
        break;

        default: break;

        }

        stateNow = stateNext;
    }

    if (stateNow == ParserState::Value)
    {
        // The last flag was without a value
        // so if it was a boolean flag then set it's value to true
        MakeTrueOnBool(name);
    }

    // In case of no tail just return the number of command line arguments
    return argumentsCount;
}

#define SET_VALUE(T) \
    m_Entries[name] = Entry{ usage, defaultValue, defaultValue }; \
    return std::get<T>(m_Entries[name].value);

bool& def::Flag::Set(const std::string& name, const bool defaultValue, const std::string& usage) { SET_VALUE(bool); }

std::string& def::Flag::Set(const std::string& name, const char* defaultValue, const std::string& usage) { SET_VALUE(std::string); }

float& def::Flag::Set(const std::string& name, const float defaultValue, const std::string& usage) { SET_VALUE(float); }
double& def::Flag::Set(const std::string& name, const double defaultValue, const std::string& usage) { SET_VALUE(double); }

int32_t& def::Flag::Set(const std::string& name, const int32_t defaultValue, const std::string& usage) { SET_VALUE(int32_t); }
int64_t& def::Flag::Set(const std::string& name, const int64_t defaultValue, const std::string& usage) { SET_VALUE(int64_t); }

uint32_t& def::Flag::Set(const std::string& name, const uint32_t defaultValue, const std::string& usage) { SET_VALUE(uint32_t); }
uint64_t& def::Flag::Set(const std::string& name, const uint64_t defaultValue, const std::string& usage) { SET_VALUE(uint64_t); }

#undef SET_VALUE

std::optional<std::reference_wrapper<const def::Flag::Entry>> def::Flag::Lookup(const std::string& name)
{
    if (m_Entries.contains(name))
        return std::nullopt;

    return m_Entries.at(name);
}

void def::Flag::PrintUsage(std::ostream& os)
{
    os << "Usage:" << std::endl;
    
    for (const auto& [name, entry] : m_Entries)
    {
        const std::string defaultValue = std::visit(Overload {
            [](const std::string& v) { return v; },
            [](const auto& v) { return std::to_string(v); }
        }, entry.defaultValue);

        os << "\t-" << name << '=' << defaultValue << ": " << entry.usage << std::endl;
    }
}

void def::Flag::MakeTrueOnBool(const std::string& name)
{
    auto& value = m_Entries[name].value;

    if (std::holds_alternative<bool>(value))
        std::get<bool>(value) = true;
}

template <class Out, class Return>
void def::Flag::ParseInteger(const std::string& input, Out& output, const std::string& name, Return (*convert)(const std::string&, size_t*, int), bool isSigned)
{
    auto charNow = input.begin();

    if (isSigned)
    {
        if (*charNow == '-')
            charNow++;
    }

    if (*charNow == '0')
    {
        charNow++;
        
        if (charNow == input.end())
        {
            output = 0;
            return;
        }

        std::string number(charNow + 1, input.end());

        switch (*charNow)
        {
        case 'x': CHECK_VALUE(output, convert(number, nullptr, 16), name); return;
        case 'b': CHECK_VALUE(output, convert(number, nullptr, 2), name); return;
        }

        throw Exception("[defFlag] Numeric literal of base 10 can't start with '0'");
    }

    CHECK_VALUE(output, convert(std::string(charNow, input.end()), nullptr, 10), name);
}

#undef CHECK_VALUE

#endif // DEF_FLAG_IMPL

#endif // DEF_FLAG_HPP
