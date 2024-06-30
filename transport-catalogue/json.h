#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    // —охраните объ€влени€ Dict и Array без изменени€
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Ёта ошибка должна выбрасыватьс€ при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        /* –еализуйте Node, использу€ std::variant */
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;
        Node(nullptr_t null);
        Node(Array array);
        Node(Dict map);
        Node(bool value);
        Node(int value);
        Node(double value);
        Node(std::string value);

        bool IsArray() const;
        bool IsMap() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;
        bool IsNull() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;

        const Value& GetValue() const;

    private:
        Value value_;
    };

    bool operator==(const Node& lhs, const Node& rhs);
    bool operator!=(const Node& lhs, const Node& rhs);

    class Document {
    public:
        Document() = default;
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    bool operator==(const Document& lhs, const Document& rhs);
    bool operator!=(const Document& lhs, const Document& rhs);

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json