#pragma once
#include <string>
#include <iostream>
#include <variant>
#include <vector>
#include <memory>
#include <optional>
#include <string_view>

namespace svg {

    struct Rgb {
    public:
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b)
            : red(r), green(g), blue(b) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
    public:
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double op)
            : red(r), green(g), blue(b), opacity(op) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    inline void PrintColor(std::ostream& out, Rgb& rgb);

    inline void PrintColor(std::ostream& out, Rgba& rgba);

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{ "none" };

    inline void PrintColor(std::ostream& out, std::monostate);

    inline void PrintColor(std::ostream& out, const std::string& color);

    std::ostream& operator<<(std::ostream& out, Color color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    inline std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
        using namespace std::literals;
        if (line_cap == StrokeLineCap::BUTT) {
            out << "butt"sv;
        }
        else if (line_cap == StrokeLineCap::ROUND) {
            out << "round"sv;
        }
        else if (line_cap == StrokeLineCap::SQUARE) {
            out << "square"sv;
        }
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
        using namespace std::literals;
        if (line_join == StrokeLineJoin::ARCS) {
            out << "arcs"sv;
        }
        else if (line_join == StrokeLineJoin::BEVEL) {
            out << "bevel"sv;
        }
        else if (line_join == StrokeLineJoin::MITER) {
            out << "miter"sv;
        }
        else if (line_join == StrokeLineJoin::MITER_CLIP) {
            out << "miter-clip"sv;
        }
        else if (line_join == StrokeLineJoin::ROUND) {
            out << "round"sv;
        }
        return out;
    }

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * ��������������� ���������, �������� �������� ��� ������ SVG-��������� � ���������.
     * ������ ������ �� ����� ������, ������� �������� � ��� ������� ��� ������ ��������
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * ����������� ������� ����� Object ������ ��� ���������������� ��������
     * ���������� ����� SVG-���������
     * ��������� ������� "��������� �����" ��� ������ ����������� ����
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(const Color& color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(const Color& color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        // ����� RenderAttrs ������� � ����� ����� ��� ���� ����� �������� fill � stroke
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast ��������� ����������� *this � Owner&,
            // ���� ����� Owner � ��������� PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    /*
     * ����� Circle ���������� ������� <circle> ��� ����������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
     // <polyline points = "0,100 50,25 50,75 100,0" / >
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // ��������� ��������� ������� � ������� �����
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    /*
     * ����� Text ���������� ������� <text> ��� ����������� ������
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        // ����� ���������� ������� ����� (�������� x � y)
        Text& SetPosition(Point pos);

        // ����� �������� ������������ ������� ����� (�������� dx, dy)
        Text& SetOffset(Point offset);

        // ����� ������� ������ (������� font-size)
        Text& SetFontSize(uint32_t size);

        // ����� �������� ������ (������� font-family)
        Text& SetFontFamily(std::string font_family);

        // ����� ������� ������ (������� font-weight)
        Text& SetFontWeight(std::string font_weight);

        // ����� ��������� ���������� ������� (������������ ������ ���� text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_ = { .0 ,.0 };
        Point offset_ = { .0, .0 };
        uint32_t size_ = 1;
        std::string font_family_ = {};
        std::string font_weight_ = {};
        std::string data_ = {};
    };

    class ObjectContainer {
    public:
        virtual ~ObjectContainer() = default;

        template<typename Obj>
        void Add(Obj obj) {
            objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&&) = 0;

    protected:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
    public:

        // ��������� � svg-�������� ������-��������� svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override {
            objects_.emplace_back(std::move(obj));
        }

        // ������� � ostream svg-������������� ���������
        void Render(std::ostream& out) const;
    };

}  // namespace svg