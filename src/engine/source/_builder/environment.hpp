#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <stack>

#include <_builder/asset.hpp>
#include <_builder/json.hpp>

template<typename T>
class Graph
{
private:
    std::string m_root;
    std::unordered_map<std::string, T> m_nodes;
    std::unordered_map<std::string, std::vector<std::string>> m_edges;

public:
    Graph() = default;

    Graph(const std::string& rootId, T root)
        : m_root {rootId}
        , m_nodes {std::make_pair(rootId, root)}
    {
    }

    void addNode(const std::string& id, T node)
    {
        m_nodes.insert(std::make_pair(id, node));
    }

    void addEdge(const std::string& from, const std::string& to)
    {
        m_edges[from].push_back(to);
    }

    // visit pre-order
    void visit(const std::function<void(const std::string&, const T&)>& visitor)
    {
        std::stack<std::string> stack;
        stack.push(m_root);

        while (!stack.empty())
        {
            auto id = stack.top();
            stack.pop();

            visitor(id, m_nodes.at(id));

            for (auto& edge : m_edges.at(id))
            {
                stack.push(edge);
            }
        }
    }

    // Graphivz
    std::string getGraphStr() const
    {
        std::stringstream ss;
        ss << "strict digraph G {\n";

        auto visit = [&ss, this](const std::string& id, auto& visitRef) -> void
        {
            if (m_edges.find(id) != m_edges.end())
            {
                for (auto& edge : m_edges.at(id))
                {
                    ss << fmt::format("{} -> {}\n", id, edge);
                    visitRef(edge, visitRef);
                }
            }
        };

        visit(m_root, visit);
        ss << "}\n";
        return ss.str();
    }

    std::string root() const
    {
        return m_root;
    }
};

class Environment
{
public:
    std::string m_name;
    std::unordered_map<std::string, std::shared_ptr<Asset>> m_assets;

    Graph<std::shared_ptr<Asset>> m_decoderGraph;
    Graph<std::shared_ptr<Asset>> m_ruleGraph;
    Graph<std::shared_ptr<Asset>> m_outputGraph;

public:
    Environment() = default;
    template<typename T>
    Environment(std::string name, const Json& jsonDefinition, T catalog): m_name{name}
    {
        auto envObj = jsonDefinition.getObject();

        auto decodersPos = std::find_if(
            envObj.begin(),
            envObj.end(),
            [](auto& tuple) { return std::get<0>(tuple) == "decoders"; });

        if (decodersPos != envObj.end())
        {
            auto decodersList = std::get<1>(*decodersPos).getArray();
            for (auto& decoder : decodersList)
            {
                auto name = decoder.getString();
                auto asset =
                    std::make_shared<Asset>(catalog.getAsset(0,name), Asset::Type::DECODER);
                m_assets[name] = asset;
                m_decoderGraph.addNode(name, asset);
                if (asset->parents().size() > 0)
                {
                    for (auto& parent : asset->parents())
                    {
                        m_decoderGraph.addEdge(parent, name);
                    }
                }
                else
                {
                    m_decoderGraph.addEdge(m_decoderGraph.root(), name);
                }
            }
        }

        auto rulesPos = std::find_if(envObj.begin(),
                                     envObj.end(),
                                     [](auto& tuple)
                                     { return std::get<0>(tuple) == "rules"; });

        if (rulesPos != envObj.end())
        {
            auto rulesList = std::get<1>(*rulesPos).getArray();
            for (auto& rule : rulesList)
            {
                auto name = rule.getString();
                auto asset =
                    std::make_shared<Asset>(rule, Asset::Type::RULE);
                m_assets[name] = asset;
                m_ruleGraph.addNode(name, asset);
                for (auto& parent : asset->parents())
                {
                    m_ruleGraph.addEdge(parent, name);
                }
            }
        }

        auto outputsPos = std::find_if(
            envObj.begin(),
            envObj.end(),
            [](auto& tuple) { return std::get<0>(tuple) == "outputs"; });

        if (outputsPos != envObj.end())
        {
            auto outputsList = std::get<1>(*outputsPos).getArray();
            for (auto& output : outputsList)
            {
                auto name = output.getString();
                auto asset = std::make_shared<Asset>(
                    output, Asset::Type::OUTPUT);
                m_assets[name] = asset;
                m_outputGraph.addNode(name, asset);
                for (auto& parent : asset->parents())
                {
                    m_outputGraph.addEdge(parent, name);
                }
            }
        }

        auto filtersPos = std::find_if(
            envObj.begin(),
            envObj.end(),
            [](auto& tuple) { return std::get<0>(tuple) == "filters"; });

        if (filtersPos != envObj.end())
        {
            auto filtersList = std::get<1>(*filtersPos).getArray();
            for (auto& filter : filtersList)
            {
                auto name = filter.getString();
                auto asset =
                    std::make_shared<Asset>(filter, Asset::Type::FILTER);
                m_assets[name] = asset;
            }
        }
    }

    // Expression getExpression() const
    // {

    // }
};

#endif // _ENVIRONMENT_H
