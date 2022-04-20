#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Json_node = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Document;

class Node {
public:
    Node() : current_node_value_(nullptr)
    {}

    template<typename T>
    Node(T value) : current_node_value_(std::move(value))
    {}

    int AsInt() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    bool AsBool() const;
    const std::string& AsString() const;
    double AsDouble() const;

    const Json_node& GetRawValue() const;

    bool IsBool() const;
    bool IsNull() const;
    bool IsString() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsArray() const;
    bool IsMap() const;

    bool operator==(const Node& rhs) const;
    bool operator!=(const Node& rhs) const;
private:
    Json_node current_node_value_;
};

class Document {
public:
    explicit Document() = default;

    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& rhs) const;
    bool operator!=(const Document& rhs) const;
private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
