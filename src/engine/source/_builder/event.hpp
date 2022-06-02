#ifndef _EVENT_H
#define _EVENT_H

#include <memory>

#include "_builder/json.hpp"

class Event
{
private:
    std::shared_ptr<Json> m_jsonPtr;

public:
    // Constructors
    Event() = default;
    Event(const Event& other)
    {
        m_jsonPtr = other.m_jsonPtr;
    }
    explicit Event(const char* json)
    {
        m_jsonPtr = std::make_shared<Json>(json);
    }
    Event(Event&& other)
    {
        m_jsonPtr = std::move(other.m_jsonPtr);
    }
    Event& operator=(const Event& other)
    {
        m_jsonPtr = other.m_jsonPtr;
        return *this;
    }
    Event& operator=(Event&& other)
    {
        m_jsonPtr = std::move(other.m_jsonPtr);
        return *this;
    }

    Json& getJson()
    {
        return *m_jsonPtr;
    }

    const Json& getJson() const
    {
        return *m_jsonPtr;
    }
};

#endif // _EVENT_H
