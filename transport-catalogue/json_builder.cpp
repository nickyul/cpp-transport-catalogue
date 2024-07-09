#include "json_builder.h"

namespace json {
    Builder::DictContext Builder::StartDict() {
        Node* node = new Node(Dict());
        nodes_stack_.emplace_back(node);

        return DictContext(*this);
    }

    Builder::KeyContext Builder::Key(std::string key) {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Can't add key. Try StartDict");
        }
        Node* node = new Node(key);

        if (nodes_stack_.back()->IsMap()) {
            nodes_stack_.emplace_back(std::move(node));
        }

        return KeyContext(*this);
    }

    Builder& Builder::Value(Node::Value value) {
        Node node;

        if (std::holds_alternative<nullptr_t>(value)) {
            node = Node(std::get<nullptr_t>(value));
        }
        else if (std::holds_alternative<Array>(value)) {
            node = Node(std::get<Array>(value));
        }
        else if (std::holds_alternative<Dict>(value)) {
            node = Node(std::get<Dict>(value));
        }
        else if (std::holds_alternative<bool>(value)) {
            node = Node(std::get<bool>(value));
        }
        else if (std::holds_alternative<int>(value)) {
            node = Node(std::get<int>(value));
        }
        else if (std::holds_alternative<double>(value)) {
            node = Node(std::get<double>(value));
        }
        else if (std::holds_alternative<std::string>(value)) {
            node = Node(std::get<std::string>(value));
        }

        CompileNode(node);

        return *this;
    }

    Builder::ArrayContext Builder::StartArray() {
        Node* node = new Node(Array());
        nodes_stack_.emplace_back(node);

        return ArrayContext(*this);
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Can't create dict");
        }
        Node node = *nodes_stack_.back();

        if (!node.IsMap()) {
            throw std::logic_error("Can't close dict. Dict wasn't opened");
        }
        nodes_stack_.pop_back();
        CompileNode(node);

        return *this;
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Can't create array");
        }
        Node node = *nodes_stack_.back();

        if (!node.IsArray()) {
            throw std::logic_error("Can't close array. Array wasn't opened");
        }
        nodes_stack_.pop_back();
        CompileNode(node);

        return *this;
    }

    Node Builder::Build() {

        if (root_.IsNull()) {
            throw std::logic_error("Json is not created");
        }

        if (!nodes_stack_.empty()) {
            throw std::logic_error("Comething wrong. Try to close opened container");
        }

        return root_;
    }

    void Builder::CompileNode(Node node) {
        if (nodes_stack_.empty()) {
            if (!root_.IsNull()) {
                throw std::logic_error("Json was created before");
            }
            root_ = node;

            return;
        }
        else {
            if (!nodes_stack_.back()->IsArray() && !nodes_stack_.back()->IsString()) {
                throw std::logic_error("Cant create the node");
            }
            if (nodes_stack_.back()->IsArray()) {
                Array array = nodes_stack_.back()->AsArray();
                array.emplace_back(node);
                nodes_stack_.pop_back();
                Node* node_p = new Node(array);
                nodes_stack_.emplace_back(std::move(node_p));

                return;
            }

            if (nodes_stack_.back()->IsString()) {
                std::string str = nodes_stack_.back()->AsString();
                nodes_stack_.pop_back();

                if (nodes_stack_.back()->IsMap()) {
                    Dict dict = nodes_stack_.back()->AsMap();
                    dict.emplace(std::move(str), node);
                    nodes_stack_.pop_back();
                    Node* node_p = new Node(dict);
                    nodes_stack_.emplace_back(std::move(node_p));
                }

                return;
            }
        }
    }
}
