#ifndef DEF_FLAG_HPP
#define DEF_FLAG_HPP

#pragma region Includes

#include <string>
#include <variant>
#include <optional>
#include <unordered_map>

#pragma endregion // Includes

namespace def
{
    class Flag
    {
    public:
        template <class T>
        struct Entry
        {
            std::string usage;
            T value;
            T defaultValue;
        };

        struct Exception : std::exception
        {
        public:
            Exception(const std::string& text) : std::exception(text.c_str()) {}
        };

    public:
        Flag() = default;

        void Parse(const size_t arguments_count, char* arguments[], bool start_from_next = true);

        bool& Set(const std::string& name, const bool defaultValue, const std::string& usage);
        std::string& Set(const std::string& name, const char* defaultValue, const std::string& usage);

        float& Set(const std::string& name, const float defaultValue, const std::string& usage);
        double& Set(const std::string& name, const double defaultValue, const std::string& usage);

        int32_t& Set(const std::string& name, const int32_t defaultValue, const std::string& usage);
        int64_t& Set(const std::string& name, const int64_t defaultValue, const std::string& usage);

        uint32_t& Set(const std::string& name, const uint32_t defaultValue, const std::string& usage);
        uint64_t& Set(const std::string& name, const uint64_t defaultValue, const std::string& usage);

        template <class T>
        std::optional<std::reference_wrapper<const Entry<T>>> Lookup(const std::string& name);

    private:
        enum class ParserState
        {
            NewToken,
            Name,
            Value
        };

        template <class... Ts> struct Overload : Ts... { using Ts::operator()...; };

    private:
        std::unordered_map<std::string, std::variant<
            Entry<bool>, Entry<float>, Entry<double>,
            Entry<int32_t>, Entry<int64_t>,
            Entry<uint32_t>, Entry<uint64_t>, Entry<std::string>
        >> m_Entries;

    };
}

#ifdef DEF_FLAG_IMPL
#undef DEF_FLAG_IMPL

void def::Flag::Parse(const size_t argumentsCount, char* arguments[], bool startFromNext)
{
    if (!arguments || argumentsCount == 1 && startFromNext)
        throw Exception("[defFlag] No arguments were provided");

    ParserState stateNow = ParserState::NewToken;
    ParserState stateNext = stateNow;

    std::string name;
    std::string value;

    for (size_t i = startFromNext ? 1 : 0; i < argumentsCount; i++)
    {
        char* arg = arguments[i];
        size_t len = strlen(arg);

        for (size_t i = 0; i < len; i++)
        {
            switch (stateNow)
            {
            case ParserState::NewToken:
            {
                if (arg[i] == '-')
                {
                    // That is definetly must be a flag name
                    stateNext = ParserState::Name;
                }
                else
                    throw Exception("[defFlag] You can't start a flag without '-' symbol");
            }
            break;

            case ParserState::Name:
            {
                if (arg[i] == '-' && i == 1)
                {
                    // Skip second dash
                    stateNext = ParserState::Name;
                }
                else
                {
                    if (arg[i] == '=')
                        stateNext = ParserState::Value;
                    else
                    {
                        name += arg[i];
                        stateNext = ParserState::Name;
                    }
                }
            }
            break;

            case ParserState::Value:
            {
                value += arg[i];
                // It's not necessary because stateNext is always
                // ParserState::Value at this point but let's leave it for clarity
                // stateNext = ParserState::Name;
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
                // A dash is the only character in the current text chunk
                throw Exception("[defFlag] You can't do '- ...'");
            }

            // Check if we have a flag with the such name
            stateNext = m_Entries.contains(name) ? ParserState::Value : ParserState::NewToken;
        }
        break;

        // Check if we've just finished parsing the value
        case ParserState::Value:
        {
            // At this point we assume that we have the name and the value of the flag

            if (!m_Entries.contains(name))
                throw Exception("[defFlag] Can't find the such flag: " + name);
            
            // We have the such flag so save the value for it
            std::visit(Overload {
                [&](Entry<bool>& entry) {
                    if (value == "true" || value == "True" || value == "")
                        entry.value = true;
                    else if (value == "false" || value == "False")
                        entry.value = false;
                    else
                        throw Exception("[defFlag] Can't parse a value of the boolean flag " + name);
                },
                [&](Entry<float>& entry) { entry.value = std::stof(value); },
                [&](Entry<double>& entry) { entry.value = std::stod(value); },
                [&](Entry<int32_t>& entry) { entry.value = std::stoi(value); },
                [&](Entry<int64_t>& entry) { entry.value = std::stoll(value); },
                [&](Entry<uint32_t>& entry) { entry.value = (uint32_t)std::stoul(value); },
                [&](Entry<uint64_t>& entry) { entry.value = std::stoull(value); },
                [&](Entry<std::string>& entry) { entry.value = value; },
                [&](auto& entry) { (void)entry; throw Exception("[defFlag] Unreachable"); },
            }, m_Entries[name]);

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
        if (std::holds_alternative<Entry<bool>>(m_Entries[name]))
            std::get<Entry<bool>>(m_Entries[name]).value = true;
    }
}

#define SET_VALUE(T) \
    m_Entries[name] = Entry<T>{ usage, defaultValue, defaultValue }; \
    return std::get<Entry<T>>(m_Entries[name]).value;

bool& def::Flag::Set(const std::string& name, const bool defaultValue, const std::string& usage) { SET_VALUE(bool); }

std::string& def::Flag::Set(const std::string& name, const char* defaultValue, const std::string& usage) { SET_VALUE(std::string); }

float& def::Flag::Set(const std::string& name, const float defaultValue, const std::string& usage) { SET_VALUE(float); }
double& def::Flag::Set(const std::string& name, const double defaultValue, const std::string& usage) { SET_VALUE(double); }

int32_t& def::Flag::Set(const std::string& name, const int32_t defaultValue, const std::string& usage) { SET_VALUE(int32_t); }
int64_t& def::Flag::Set(const std::string& name, const int64_t defaultValue, const std::string& usage) { SET_VALUE(int64_t); }

uint32_t& def::Flag::Set(const std::string& name, const uint32_t defaultValue, const std::string& usage) { SET_VALUE(uint32_t); }
uint64_t& def::Flag::Set(const std::string& name, const uint64_t defaultValue, const std::string& usage) { SET_VALUE(uint64_t); }

#undef SET_VALUE

template <class T>
std::optional<std::reference_wrapper<const def::Flag::Entry<T>>> def::Flag::Lookup(const std::string& name)
{
    if (m_Entries.contains(name))
        return std::nullopt;

    return m_Entries.at(name);
}

#endif // DEF_FLAG_IMPL

#endif // DEF_FLAG_HPP
