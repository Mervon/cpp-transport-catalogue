#pragma once

#include "json_builder.h"
#include "json.h"

#include <string>
#include <vector>

using namespace std;

namespace json {

class BaseContext;
class KeyItemContext;
class DictItemContext ;
class ArrItemContext ;
class ValueArrItemContext;
class ValueKeyItemContext;

class Builder {
private:
    friend BaseContext;
    json::Node root_;
    std::vector<json::Node*> nodes_stack_;
    int total_dict_ = 0;
    int total_arr_ = 0;
    bool started_ = false;

public:
    Builder();

    DictItemContext& StartDict();

    KeyItemContext& Key(std::string key);

    BaseContext& Value(json::Node value);

    json::Node& Build();

    ArrItemContext& StartArray();

    BaseContext& EndDict();

    BaseContext& EndArray();

    bool CheckIfReady();
};

class BaseContext {
public:

    BaseContext(Builder& builder);

    BaseContext& Value(json::Node value);

    json::Node& Build();

    BaseContext& EndDict();

    BaseContext& EndArray();

    BaseContext& StartArray();

    BaseContext& StartDict();

    BaseContext& Key(std::string key);

protected:
    Builder& builder_;
};

class ValueKeyItemContext : public BaseContext {
public:

    KeyItemContext& Key(std::string key);

    BaseContext& EndDict();

    BaseContext& EndArray() = delete;
    json::Node& Build() = delete;
    BaseContext& Value(json::Node value) = delete;
    BaseContext& StartArray() = delete;
    BaseContext& StartDict() = delete;
};


class ValueArrItemContext : public BaseContext {
public:

    BaseContext& EndArray();

    ArrItemContext& StartArray();

    DictItemContext& StartDict();

    ValueArrItemContext& Value(json::Node value);

    BaseContext& Key(std::string key) = delete;
    BaseContext& EndDict() = delete;
    json::Node& Build() = delete;
};

class DictItemContext  : public BaseContext {
public:

    KeyItemContext& Key(std::string key);

    BaseContext& EndDict();

    BaseContext& EndArray() = delete;
    json::Node& Build() = delete;
    BaseContext& Value(json::Node value) = delete;
    ArrItemContext& StartArray() = delete;
    DictItemContext& StartDict() = delete;
};


class KeyItemContext : public BaseContext {
public:

    DictItemContext& StartDict();

    ArrItemContext& StartArray();

    ValueKeyItemContext& Value(json::Node value);

    KeyItemContext& Key(std::string key) = delete;
    BaseContext& EndDict() = delete;
    BaseContext& EndArray() = delete;
    json::Node& Build() = delete;
};

class ArrItemContext  : public BaseContext {
public:

    BaseContext& EndArray();

    ArrItemContext& StartArray();

    DictItemContext& StartDict();

    ValueArrItemContext& Value(json::Node value);

    KeyItemContext& Key(std::string key) = delete;
    BaseContext& EndDict() = delete;
    json::Node& Build() = delete;
};
}
