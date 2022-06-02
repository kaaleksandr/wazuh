#include "testUtils.hpp"

#include "_builder/event.hpp"
#include "_builder/json.hpp"
#include "_builder/operation.hpp"

TEST(Result, Constructs)
{
    auto event = Event<Json> {Json {R"({
        "source": "test",
        "type": "A",
        "threat": {
            "level": 6
        },
        "weird": {
            "field": "value"
        }
    })"}};
    auto event1 = Event<Json> {Json {R"({
        "source": "test",
        "type": "A",
        "threat": {
            "level": 6
        },
        "weird": {
            "field": "value"
        }
    })"}};
    auto result = makeSuccess(std::move(event), "Event: {}", event1);
}
