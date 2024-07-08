#pragma once
#include "json.h"

namespace json {
    class KeyContext;
    class DictContext;
    class ArrayContext;


    class Builder {
    public:
        Builder() = default;

        DictContext StartDict();

        KeyContext Key(std::string key);

        Builder& Value(Node::Value value);

        ArrayContext StartArray();

        Builder& EndDict();

        Builder& EndArray();

        Node Build();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        void CompileNode(Node node);
    };

    class BaseContext {
    public:
        BaseContext(Builder& builder);

        KeyContext Key(std::string key);

        DictContext StartDict();

        ArrayContext StartArray();

        Builder& Value(Node::Value value);

        Builder& EndDict();

        Builder& EndArray();

    private:
        Builder& builder_;

    };

    class KeyContext : public BaseContext {
    public:
        KeyContext(Builder& builder);

        KeyContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;

        DictContext Value(Node::Value value);
    };

    class DictContext : public BaseContext {
    public:
        DictContext(Builder& builder);

        DictContext StartDict() = delete;
        ArrayContext StartArray() = delete;
        Builder& EndArray() = delete;
        Builder& Value(Node::Value value) = delete;
    };

    class ArrayContext : public BaseContext {
    public:
        ArrayContext(Builder& builder);

        KeyContext Key(std::string key) = delete;
        Builder& EndDict() = delete;
        ArrayContext Value(Node::Value value);
    };

} // namespace json
