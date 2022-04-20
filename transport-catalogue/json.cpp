#include "json.h"

#include <sstream>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    bool is_closed = false;
    Array result;
    for (char c; input >> c;) {
        if (c == ']') {
            is_closed = true;
            break;
        }
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (!is_closed) {
        throw ParsingError("LoadArrayFailed"s);
    }

    return Node(move(result));
}

using Number = std::variant<int, double>;

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(istream& input) {
    string line;
    char c;
    while (input.peek() != -1) {
        c = input.get();
        //cout << c << endl;
        if (c == '\\') {
            //cout << c << " : ";
            if (input.peek() != -1) {
                char c2 = input.get();
                //cout << c2 << endl;
                if (c2 == '\\') {
                    line += '\\';
                } else if (c2 == '"') {
                    line += '\"';
                } else if (c2 == 't') {
                    line += '\t';
                } else if (c2 == 'n') {
                    line += '\n';
                } else if (c2 == 'r') {
                    line += '\r';
                } else {
                    throw ParsingError("2"s);
                }
            } else {
                throw ParsingError("1"s);
            }
        } else if (c == '\n' || c == '\r') {
            throw ParsingError("3"s);
        } else if (c == '"') {
            return Node(move(line));
        } else {
            line += c;
        }

    }

    throw ParsingError("LoadStringFailed"s);


}

Node LoadBool(istream& input) {
    string str;
    for (int i = 0; i < 3; ++i) {
        if (input.peek() != -1) {
            str += static_cast<char>(input.get());
        }
    }

    if (str == "rue"s) {
        return Node{true};
    } else if (str == "als"s) {
        if (input.peek() == 'e') {
            str += static_cast<char>(input.get());
            return Node{false};
        } else {
            throw ParsingError("LoadBoolFailed");
        }
    } else {
        throw ParsingError("LoadBoolFailed");
    }

}

Node LoadNull(istream& input) {
    string str;
    for (int i = 0; i < 3; ++i) {
        if (input.peek() != -1) {
            str += static_cast<char>(input.get());
        }
    }

    if (str == "ull"s) {
        return Node{};
    }

    throw ParsingError("LoadNullFailed");
}

Node LoadDict(istream& input) {
    bool is_closed = false;
    Dict result;
    for (char c; input >> c;) {

        if (c == '}') {
            is_closed = true;
            break;
        }

        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();

        input >> c;

        result.insert({move(key), LoadNode(input)});

    }

    if (!is_closed) {
        throw ParsingError("LoadDictFailed"s);
    }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {

        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 't' || c == 'f') {

        return LoadBool(input);
    } else if (c == 'n') {
        return LoadNull(input);
    } else if (!input) {
        throw ParsingError("Cannot parse JSONa"s);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(current_node_value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(current_node_value_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(current_node_value_);
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(current_node_value_);
}

bool Node::IsDouble() const {
    if (std::holds_alternative<int>(current_node_value_) || std::holds_alternative<double>(current_node_value_)) {
        return true;
    }
    return false;
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(current_node_value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(current_node_value_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(current_node_value_);
}

const Array& Node::AsArray() const {
    if (IsArray()) {
        return std::get<Array>(current_node_value_);
    }
    throw std::logic_error("BadTryToAccesAsArray");
}

const Dict& Node::AsMap() const {
    if (IsMap()) {
        return std::get<Dict>(current_node_value_);
    }
    throw std::logic_error("BadTryToAccesAsDict");
}

int Node::AsInt() const {
    if (IsInt()) {
        return std::get<int>(current_node_value_);
    }
    throw std::logic_error("BadTryToAccesAsInt");
}

bool Node::AsBool() const {
    if (IsBool()) {
        return std::get<bool>(current_node_value_);
    }
    throw std::logic_error("BadTryToAccesAsBool");
}

const string& Node::AsString() const {
    if (IsString()) {
        return std::get<std::string>(current_node_value_);
    }
    throw std::logic_error("BadTryToAccesAsString");
}

double Node::AsDouble() const {
    if (IsPureDouble()) {
        return std::get<double>(current_node_value_);
    } else if (IsInt()) {
        return static_cast<double>(std::get<int>(current_node_value_));
    } else {
        throw std::logic_error("BadTryToAccesAsDouble");
    }
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document(LoadNode(input));
}

const Json_node& Node::GetRawValue() const {
    return current_node_value_;
}

struct RootPrinter {
    std::ostream& output;

    void operator()(std::nullptr_t) {
        output << "null"s;
    }
    void operator()(Array a) {
        bool is_first = false;
        output << '[';
        for (auto& item : a) {
            if (is_first) {
                output << ", "s;
            }
            visit(RootPrinter{output}, item.GetRawValue());

            is_first = true;
        }
        output << ']';
    }
    void operator()(Dict dict) {
        output << '{';
        bool is_first = false;
        for (auto& item : dict) {
            if (is_first) {
                output << ", "s;
            }
            visit(RootPrinter{output}, Json_node{item.first});
            output << ": ";
            visit(RootPrinter{output}, item.second.GetRawValue());
            is_first = true;
        }
        output << '}';
    }
    void operator()(bool b) {
        output << std::boolalpha << b << std::noboolalpha;
    }
    void operator()(int i) {
        output << i;
    }
    void operator()(double d) {
        output << d;
    }
    void operator()(std::string s) {
        string tmp;
        tmp += '"';
        for (char c : s) {
            if (c == '"') {
                tmp += '\\';
                tmp += '"';
            } else if (c == '\\') {
                tmp += '\\';
                tmp += '\\';
            } else if (c == '\n') {
                tmp += '\\';
                tmp += 'n';
            } else if (c == '\r') {
                tmp += '\\';
                tmp += 'r';
            } else {
                tmp += c;
            }
        }
        tmp += '"';
        output << tmp;
    }

};

void Print(const Document& doc, std::ostream& output) {
    visit(RootPrinter{output}, doc.GetRoot().GetRawValue());
}

bool Node::operator==(const Node& rhs) const {
    return this->current_node_value_ == rhs.current_node_value_;
}

bool Node::operator!=(const Node& rhs) const {
    return !(*this == rhs);
}

bool Document::operator==(const Document& rhs) const {
    return this->GetRoot() == rhs.GetRoot();
}

bool Document::operator!=(const Document& rhs) const {
    return !(*this == rhs);
}

}  // namespace json
