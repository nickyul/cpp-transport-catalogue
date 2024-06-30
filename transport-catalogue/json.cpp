#include "json.h"

using namespace std;

namespace json {

    namespace {

        using Number = std::variant<int, double>;

        Node LoadNode(istream& input);

        string LoadLine(istream& input) {
            string s;
            while (isalpha(input.peek())) {
                s.push_back(input.get());
            }
            return s;
        }

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (!input) {
                throw ParsingError("Failed to read Node in array from stream"s);
            }

            return Node(move(result));
        }

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
            }
            else {
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
                        return Node{ std::stoi(parsed_num) };
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node{ std::stod(parsed_num) };
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream& input) {
            using namespace std::literals;

            auto it = istreambuf_iterator<char>(input);
            auto end = istreambuf_iterator<char>();
            string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node{ move(s) };
        }

        Node LoadDict(std::istream& input) {
            Dict result;

            for (char ch; input >> ch && ch != '}';) {

                if (ch == '"') {
                    // key = "key"
                    std::string key = LoadString(input).AsString();

                    if (input >> ch && ch == ':') {
                        // map[key] = Node 
                        // {"key":Node}
                        result.emplace(std::move(key), LoadNode(input));

                    }
                    else {
                        throw ParsingError("Failed to read Map Node from stream (can't parse : )"s);
                    }

                }
            }

            if (!input) {
                throw ParsingError("Failed to read Node in Map from stream"s);
            }

            return Node(result);
        }

        Node LoadBool(istream& input) {
            string s(LoadLine(input));

            if (s == "true"s) {
                return Node{ true };
            }
            else if (s == "false"s) {
                return Node{ false };
            }
            else {
                throw ParsingError("Failed to read bool Node from stream");
            }
        }

        Node LoadNull(istream& input) {
            if (LoadLine(input) == "null"s) {
                return Node{ nullptr };
            }
            else {
                throw ParsingError("Failed to read null Node from stream");
            }
        }

        Node LoadNode(istream& input) {
            char c;

            if (!input) {
                throw ParsingError(""s);
            }

            input >> c;

            switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            case 't': case 'f':
                input.putback(c);
                return LoadBool(input);
            default:
                input.putback(c);
                return LoadNumber(input);

            }
        }
    }  // namespace


    Node::Node(nullptr_t null)
        : value_(null) {}

    Node::Node(Array array)
        : value_(move(array)) {}

    Node::Node(Dict map)
        : value_(move(map)) {}

    Node::Node(bool value)
        : value_(value) {}

    Node::Node(int value)
        : value_(value) {}

    Node::Node(double value)
        : value_(value) {}

    Node::Node(string value)
        : value_(move(value)) {}


    bool Node::IsArray() const {
        return holds_alternative<Array>(value_) ? true : false;
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(value_) ? true : false;
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(value_) ? true : false;
    }

    bool Node::IsInt() const {
        return holds_alternative<int>(value_) ? true : false;
    }

    bool Node::IsDouble() const {
        return (holds_alternative<double>(value_) || holds_alternative<int>(value_)) ? true : false;
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(value_) ? true : false;
    }

    bool Node::IsString() const {
        return holds_alternative<string>(value_) ? true : false;
    }

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(value_) ? true : false;
    }


    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw logic_error("Is not Array");
        }
        return get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw logic_error("Is not Map");
        }
        return get<Dict>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw logic_error("Is not Bool");
        }
        return get<bool>(value_);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw logic_error("Is not Int");
        }
        return get<int>(value_);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw logic_error("Is not Double");
        }
        return holds_alternative<double>(value_) ? get<double>(value_) : static_cast<double>(get<int>(value_));
    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw logic_error("Is not String");
        }
        return get<string>(value_);
    }

    const Node::Value& Node::GetValue() const {
        return value_;
    }


    bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }


    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs.GetRoot() == rhs.GetRoot());
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintString(const string& s, ostream& out) {
        out << '"';
        for (const char& ch : s) {
            // Обрабатываем одну из последовательностей
            switch (ch) {
            case '\n':
                out << "\\n"sv;
                continue;
            case '\t':
                out << "\\t"sv;
                continue;
            case '\r':
                out << "\\r"sv;
                continue;
            case '"':
                out << "\\\""sv;
                continue;
            case '\\':
                out << "\\\\"sv;
                continue;
            default:
                out << ch;
                continue;
            }
        }
        out << '"';
    }

    void PrintNode(const Node& node, const PrintContext& context);

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& context) {
        context.out << value;
    }

    void PrintValue(nullptr_t, const PrintContext& context) {
        context.out << "null"sv;
    }

    void PrintValue(bool b, const PrintContext& context) {
        context.out << boolalpha << b;
    }

    void PrintValue(const string& s, const PrintContext& context) {
        PrintString(s, context.out);
    }

    void PrintValue(const Dict& dic, const PrintContext& context) {
        context.out << "{\n"sv;
        PrintContext help = context.Indented();

        bool first = true;
        for (const auto& [key, value] : dic) {
            if (first) {
                first = false;
            }
            else {
                context.out << ",\n"sv;
            }
            help.PrintIndent();
            PrintString(key, context.out);
            context.out << ": "sv;
            PrintNode(value, help);
        }
        context.out << '\n';
        context.PrintIndent();
        context.out << "}"sv;
    }

    void PrintValue(const Array& arr, const PrintContext& context) {
        context.out << "[\n"sv;
        PrintContext help = context.Indented();

        bool first = true;
        for (const Node& node : arr) {
            if (first) {
                first = false;
            }
            else {
                context.out << ",\n"sv;
            }
            help.PrintIndent();
            PrintNode(node, help);
        }
        context.out << '\n';
        context.PrintIndent();
        context.out << "]"sv;
    }


    void PrintNode(const Node& node, const PrintContext& context) {
        std::visit(
            [&context](const auto& value) {
                PrintValue(value, context);
            }, node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }

}  // namespace json