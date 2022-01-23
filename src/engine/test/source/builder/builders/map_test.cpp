#include <gtest/gtest.h>
#include <rxcpp/rx.hpp>
#include <vector>

#include "map.hpp"
#include "map_value.hpp"
#include "test_utils.hpp"

using namespace builder::internals::builders;

TEST(MapBuilderTest, BuildsMapValue)
{
    // Fake entry point
    observable<event_t> entry_point = observable<>::create<event_t>(
        [](subscriber<event_t> o)
        {
            o.on_next(event_t{R"(
      {
              "field": 1
      }
  )"});
            o.on_next(event_t{R"(
      {
              "field": "1"
      }
  )"});
            o.on_next(event_t{R"(
      {
              "otherfield": 1
      }
  )"});
            o.on_completed();
        });

    // Fake input json
    auto fake_jstring = R"(
      {
          "check": {
              "mapped_field": 1
          }
      }
  )";
    json::Document fake_j{fake_jstring};

    // Build
    auto conditionObs = mapBuilder(entry_point, fake_j.get(".check"));
    auto expectedObs = mapValueBuilder(entry_point, fake_j.get(".check"));

    // Fake subscribers
    vector<event_t> observed;
    auto subscriber = make_subscriber<event_t>([&observed](event_t j) { observed.push_back(j); }, []() {});

    vector<event_t> observedExpected;
    auto subscriberExpected =
        make_subscriber<event_t>([&observedExpected](event_t j) { observedExpected.push_back(j); }, []() {});

    // Operate
    ASSERT_NO_THROW(conditionObs.subscribe(subscriber));
    ASSERT_NO_THROW(expectedObs.subscribe(subscriberExpected));
    ASSERT_EQ(observed.size(), observedExpected.size());
    for (auto i = 0; i < observed.size(); i++)
    {
        ASSERT_EQ(observed[i].get(".mapped_field")->GetInt(), observedExpected[i].get(".mapped_field")->GetInt());
    }
}