#pragma once
#include "json.h"

namespace json {
    class KeyContext;
    class DictContext;
    class ArrayContext;


    class Builder {
    private:
        class BaseContext;
        class KeyContext;
        class DictContext;
        class ArrayContext;

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


        class BaseContext {
        public:
            BaseContext(Builder& builder)
                : builder_(builder) {}

            KeyContext Key(std::string key) {
                return builder_.Key(key);
            }

            DictContext StartDict() {
                return builder_.StartDict();
            }

            ArrayContext StartArray() {
                return builder_.StartArray();
            }

            Builder& Value(Node::Value value) {
                return builder_.Value(value);
            }

            Builder& EndDict() {
                return builder_.EndDict();
            }

            Builder& EndArray() {
                return builder_.EndArray();
            }

        private:
            Builder& builder_;

        };

        class KeyContext : public BaseContext {
        public:
            KeyContext(Builder& builder)
                : BaseContext(builder) {}

            KeyContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
            BaseContext EndArray() = delete;

            DictContext Value(Node::Value value) {
                return BaseContext::Value(value);
            }
        };

        class DictContext : public BaseContext {
        public:
            DictContext(Builder& builder)
                : BaseContext(builder) {}

            DictContext StartDict() = delete;
            ArrayContext StartArray() = delete;
            Builder& EndArray() = delete;
            Builder& Value(Node::Value value) = delete;
        };

        class ArrayContext : public BaseContext {
        public:
            ArrayContext::ArrayContext(Builder& builder)
                : BaseContext(builder) {}

            KeyContext Key(std::string key) = delete;
            Builder& EndDict() = delete;

            ArrayContext Value(Node::Value value) {
                return BaseContext::Value(value);
            }
        };
    };

} // namespace json
