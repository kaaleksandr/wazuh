#include <algorithm>
#include <any>
#include <vector>

#include "_builder/environment.hpp"
#include "_builder/registry.hpp"

namespace builder::internals::builders
{

Expression assetBuilder(const std::any& definition)
{
    auto [asset, children] =
            std::any_cast<std::tuple<std::shared_ptr<Asset>, std::vector<Expression>>>(
                definition);

    auto stages = And::create("stages", asset->stages());
    auto consequence =
        And::create("consequence", {stages} );
        consequence->m_operands.push_back(Or::create("children", children));
    auto expression =
        Implication::create(asset->name(), asset->check(), consequence);

    return expression;
}

} // namespace builder::internals::builders
