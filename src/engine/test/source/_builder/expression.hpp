#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <algorithm>
#include <functional>
#include <memory>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <fmt/format.h>

namespace builder
{
enum class Formula
{
    Term,
    Implication,
    And,
    Or,
    Chain
};

std::string formulaStr(Formula formula)
{
    switch (formula)
    {
        case Formula::Term: return "Term";
        case Formula::Implication: return "Implication";
        case Formula::And: return "And";
        case Formula::Or: return "Or";
        case Formula::Chain: return "Chain";
        default: throw std::runtime_error("Unknown formula");
    }
}

template<typename T>
class Expression
{
private:
    using uid = unsigned int;
    uid m_countId;

    uid m_rootId;

    std::unordered_map<uid, Formula> m_nodes;
    std::unordered_map<uid, std::vector<uid>> m_edges;

    std::unordered_map<uid, T> m_termData;

    std::unordered_map<std::string, uid> m_namedNodes;
    std::unordered_map<uid, std::string> m_namedNodesReverse;

    uid generateId()
    {
        if (m_countId == std::numeric_limits<uid>::max())
        {
            throw std::runtime_error(
                "Expression::generateId: maximum number of nodes reached");
        }
        if (m_nodes.find(m_countId) != m_nodes.end())
        {
            throw std::runtime_error(
                "Expression::generateId: node already exists");
        }

        return m_countId++;
    }

    void addRelation(uid parentId, uid childId)
    {
        if (m_nodes.find(parentId) == m_nodes.end())
        {
            throw std::runtime_error(
                "Expression::_addRelation: parent node does not exist");
        }
        if (m_nodes.find(childId) == m_nodes.end())
        {
            throw std::runtime_error(
                "Expression::_addRelation: child node does not exist");
        }

        if (std::find(m_edges[parentId].begin(),
                      m_edges[parentId].end(),
                      childId) != m_edges[parentId].end())
        {
            throw std::runtime_error(
                "Expression::_addRelation: relation already exists");
        }

        if (m_nodes[parentId] == Formula::Term)
        {
            throw std::runtime_error(
                "Expression::_addRelation: parent node is a term");
        }
        if (m_nodes[parentId] == Formula::Implication &&
            m_edges[parentId].size() > 1)
        {
            throw std::runtime_error(
                "Expression::_addRelation: parent node is an implication and "
                "already has two children");
        }

        m_edges[parentId].push_back(childId);
    }

    void addName(uid nodeId, std::string name)
    {
        if (m_nodes.find(nodeId) == m_nodes.end())
        {
            throw std::runtime_error(
                "Expression::_updateName: node does not exist");
        }

        if (!name.empty())
        {
            if (m_namedNodes.find(name) != m_namedNodes.end())
            {
                throw std::runtime_error(
                    "Expression::_updateName: name already exists");
            }

            m_namedNodes[name] = nodeId;
        }

        m_namedNodesReverse[nodeId] = name;
    }

    uid addNode(Formula formula, uid parentId, std::string name)
    {
        uid id = generateId();
        m_nodes[id] = formula;
        m_edges[id] = {};
        addRelation(parentId, id);
        addName(id, name);

        return id;
    }

public:
    explicit Expression(Formula rootOperation, std::string name = "")
    {
        if (rootOperation == Formula::Term)
        {
            throw std::runtime_error(
                "Expression::Expression: Expected an operation but got a term");
        }

        m_rootId = generateId();
        m_nodes[m_rootId] = rootOperation;
        m_edges[m_rootId] = {};

        addName(m_rootId, name);
    }

    explicit Expression(T termData, std::string name = "")
    {
        m_rootId = generateId();
        m_nodes[m_rootId] = Formula::Term;
        m_termData[m_rootId] = termData;

        addName(m_rootId, name);
    }

    uid addOperation(Formula operation, uid parentId, std::string name = "")
    {
        if (operation == Formula::Term)
        {
            throw std::runtime_error("Expression::addOperation: Expected an "
                                     "operation but got a term");
        }
        auto id = addNode(operation, parentId);
        return id;
    }

    uid addTerm(T termData, uid parentId, std::string name = "")
    {
        auto id = addNode(Formula::Term, parentId);
        m_termData[id] = termData;

        return id;
    }

    static Expression compose(Formula operation,
                              const std::vector<Expression>& expressions,
                              std::string name = "")
    {
        if (operation == Formula::Term)
        {
            throw std::runtime_error(
                "Expression::compose: Expected an operation but got a term");
        }

        Expression result(operation, name);

        for (const auto& expression : expressions)
        {
            // Result generates new uids, we need and LUT to map the old uids to
            // the new ones
            std::unordered_map<uid, uid> idLookup;
            std::for_each(
                expression.m_nodes.cbegin(),
                expression.m_nodes.cend(),
                [&](const auto& nodePair)
                {
                    // Generate new uid
                    idLookup[nodePair.first] = result.generateId();

                    // Copy node
                    result.m_nodes[idLookup[nodePair.first]] = nodePair.second;

                    // Copy term data
                    if (nodePair.second == Formula::Term)
                    {
                        result.m_termData[idLookup[nodePair.first]] =
                            expression.m_termData[nodePair.first];
                    }

                    // Copy name
                    if (expression.m_namedNodes.find(nodePair.second) !=
                        expression.m_namedNodes.end())
                    {
                        result.m_namedNodes
                            [expression.m_namedNodes[nodePair.second]] =
                            idLookup[nodePair.first];
                    }
                    result.m_namedNodesReverse[idLookup[nodePair.first]] =
                        expression.m_namedNodesReverse[nodePair.first];
                });

            // All nodes inserted and uids translated, now add edges
            // root edge
            result.addRelation(result.m_rootId, idLookup[expression.m_rootId]);
            std::for_each(expression.m_edges.cbegin(),
                          expression.m_edges.cend(),
                          [&](const auto& edgePair)
                          {
                              for (const auto& childId : edgePair.second)
                              {
                                  result.addRelation(idLookup[edgePair.first],
                                                     idLookup[childId]);
                              }
                          });
        }

        return result;
    }

    // depth first visitor
    void visit(std::function<void(uid, Formula)> visitor) const
    {
        std::stack<uid> stack;
        stack.push(m_rootId);

        while (!stack.empty())
        {
            auto id = stack.top();
            stack.pop();

            visitor(id, m_nodes.at(id));

            for (auto childId : m_edges.at(id))
            {
                stack.push(childId);
            }
        }
    }

    // visit leafs
    void visitLeafs(std::function<void(uid, Formula)> visitor) const
    {
        std::stack<uid> stack;
        stack.push(m_rootId);

        while (!stack.empty())
        {
            auto id = stack.top();
            stack.pop();

            if (m_nodes.at(id) == Formula::Term)
            {
                visitor(id, m_nodes.at(id));
            }
            else
            {
                for (auto& childId : m_edges.at(id))
                {
                    stack.push(childId);
                }
            }
        }
    }

    std::string getGraphStr() const
    {
        std::stringstream ss;
        ss << "strict digraph G {\n";

        auto visitor = [&ss, this](uid id, Formula formula)
        {
            if (formula != Formula::Term)
            {
                int childNum = 0;
                for (auto childId : m_edges.at(id))
                {
                    ss << fmt::format(
                        "\"{}[{}][{}]\" -> \"{}[{}][{}]\" [label={}];\n",
                        m_namedNodesReverse.at(id),
                        formulaStr(formula),
                        id,
                        m_namedNodesReverse.at(childId),
                        formulaStr(m_nodes.at(childId)),
                        childId,
                        childNum++);
                }
            }
        };

        visit(visitor);
        ss << "}\n";
        return ss.str();
    }
};

} // namespace builder

#endif // _EXPRESSION_H
