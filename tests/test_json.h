#pragma once

#include <simulant/test.h>
#include <simulant/utils/json.h>

namespace {

using namespace smlt;

class JSONTests : public test::SimulantTestCase {
public:

    void test_basic_usage() {
        std::string data = R"(
{
    "array": [1, 2, 3, 4],
    "object": {
        "one": 1,
        "two": 2.0,
        "three": true,
        "four": false,
        "five": null
    }
}
)";

        auto it = json_parse(data);

        assert_equal(it->type(), JSON_OBJECT);
        assert_equal(it->size(), 2u);

        auto array = it["array"];

        assert_equal(array->type(), JSON_ARRAY);
        assert_equal(array->size(), 4);

        auto value = array[0];
        assert_equal(value->type(), JSON_NUMBER);
        assert_equal(value->to_int().value_or(0), 1);

        value = array[1];
        assert_equal(value->type(), JSON_NUMBER);
        assert_equal(value->to_int().value_or(0), 2);

        assert_false(array[5].is_valid());

        auto obj = it["object"];

        assert_equal(obj->type(), JSON_OBJECT);
        assert_equal(obj->size(), 5);

        auto one = obj["one"];

        assert_equal(one->type(), JSON_NUMBER);
        assert_true(one->is_value_type());
        assert_equal(one->to_int().value_or(0), 1);
        assert_equal(one->to_float().value_or(0.0f), 1.0f);
        assert_equal(one->to_str(), "1");
    }

    void test_nested_accesses() {
        std::string data = R"(
{
    "object": {
        "array": [1, 2, 3]
    }
}
)";

        auto json = json_parse(data);
        assert_equal(
            json["object"]["array"][0]->to_int().value_or(0),
            1
        );
    }

    void test_nested_arrays() {
        std::string data = R"(
{
    "array": [[1, 2, 3], "cheese"]
}
)";

        auto json = json_parse(data);

        assert_equal(
            json["array"][0][0]->to_int().value_or(0),
            1
        );

        assert_equal(
            json["array"][0][1]->to_int().value_or(0),
            2
        );

        assert_equal(
            json["array"][1][0]->to_str(),
            "cheese"
        );
    }
};

}
