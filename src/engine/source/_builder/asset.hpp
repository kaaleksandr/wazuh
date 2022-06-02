#ifndef _ASSET_H
#define _ASSET_H

#include <string>
#include <unordered_set>
#include <vector>

#include <fmt/format.h>

#include "_builder/expression.hpp"
#include "_builder/json.hpp"
#include "_builder/registry.hpp"

class Asset
{
public:
    enum class Type
    {
        CHAIN,
        AND,
        OR,
        BROADCAST
    };

private:
    std::string m_name;
    Expression m_check;
    std::vector<Expression> m_stages;
    Type m_type;

public:
    Asset(const std::string& name,
          const Json& checkDefinition,
          const Json& stagesDefinition,
          Type type)
        : m_name {name}
        , m_type {type}
    {
        m_check = Registry::getBuilder("stage.check")(checkDefinition);


        if (!stagesDefinition.isObject())
        {
            throw std::runtime_error(fmt::format(
                "Invalid asset definition: expected [object] but got [{}]",
                stagesDefinition.typeName()));
        }

        auto stagesObj = stagesDefinition.getObject();

        // Build stages
        std::transform(stagesObj.begin(),
                       stagesObj.end(),
                       std::back_inserter(m_stages),
                       [](auto tuple)
                       {
                           auto& [key, value] = tuple;
                           return Registry::getBuilder("stage." + key)(value);
                       });
    }

    const std::string& name() const
    {
        return m_name;
    }

    std::string& name()
    {
        return m_name;
    }

    const Expression& check() const
    {
        return m_check;
    }

    Expression& check()
    {
        return m_check;
    }

    const std::vector<Expression>& stages() const
    {
        return m_stages;
    }

    std::vector<Expression>& stages()
    {
        return m_stages;
    }

    Type type() const
    {
        return m_type;
    }

    Type& type()
    {
        return m_type;
    }
};

#endif // _ASSET_H
