#define _USE_MATH_DEFINES
#include "svg.h"
#include <cmath>

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (const Point& point : points_) {
            if (first) {
                out << point.x << ","sv << point.y;
                first = false;
                continue;
            }
            out << " "sv << point.x << ","sv << point.y;
        }
        out << "\"";
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>";
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        for (const char& ch : data) {
            switch (ch) {
            case '"':
                data_ += "&quot;";
                continue;
            case '\'':
                data_ += "&apos;";
                continue;
            case '<':
                data_ += "&lt;";
                continue;
            case '>':
                data_ += "&gt;";
                continue;
            case '&':
                data_ += "&amp;";
                continue;
            default:
                data_ += ch;
                continue;
            }
        }
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv
            << offset_.y << "\" font-size=\""sv << size_ << "\"";

        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\"";
        }

        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\"";
        }

        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << ">" << data_ << "</text>";
    }

    // ---------- Document ------------------

    void Document::Render(std::ostream& out) const {
        int indent = 2;
        int indent_step = 2;

        RenderContext context(out, indent_step, indent);

        const std::string_view xml = R"(<?xml version="1.0" encoding="UTF-8" ?>)";
        const std::string_view svg = R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)";

        out << xml << "\n"sv << svg << "\n"sv;

        for (const auto& object : objects_) {
            object->Render(context);
        }

        out << "</svg>"sv;
    }

    // ---------- Color ------------------

    void PrintColor(std::ostream& out, Rgb& rgb) {
        out << "rgb("sv << static_cast<short>(rgb.red) <<
            ","sv << static_cast<short>(rgb.green) <<
            ","sv << static_cast<short>(rgb.blue) << ")"sv;
    }

    void PrintColor(std::ostream& out, Rgba& rgba) {
        out << "rgba("sv << static_cast<short>(rgba.red) <<
            ","sv << static_cast<short>(rgba.green) <<
            ","sv << static_cast<short>(rgba.blue) <<
            ","sv << rgba.opacity << ")"sv;
    }

    void PrintColor(std::ostream& out, std::monostate) {
        out << "none"sv;
    }

    void PrintColor(std::ostream& out, const std::string& color) {
        out << color;
    }

    std::ostream& operator<<(std::ostream& out, Color color) {
        std::visit([&out](auto value) {
            PrintColor(out, value);
            }, color);
        return out;
    }

    namespace shapes {

        class Triangle : public svg::Drawable {
        public:
            Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
                : p1_(p1)
                , p2_(p2)
                , p3_(p3) {
            }

            // Реализует метод Draw интерфейса svg::Drawable
            void Draw(svg::ObjectContainer& container) const override {
                container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
            }

        private:
            svg::Point p1_, p2_, p3_;
        };

        class Star : public svg::Drawable {
        public:
            Star(svg::Point center, double outer_rad, double inner_rad, int num_rays)
                : polyline_(CreateStar(center, outer_rad, inner_rad, num_rays)) {}

            void Draw(svg::ObjectContainer& container) const override {
                container.Add(polyline_);
            }

        private:
            Polyline polyline_;

            Polyline CreateStar(Point center, double outer_rad, double inner_rad, int num_rays) {
                Polyline polyline;
                polyline.SetFillColor("red"s).SetStrokeColor("black"s);
                for (int i = 0; i <= num_rays; ++i) {
                    double angle = 2 * M_PI * (i % num_rays) / num_rays;
                    polyline.AddPoint({ center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
                    if (i == num_rays) {
                        break;
                    }
                    angle += M_PI / num_rays;
                    polyline.AddPoint({ center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
                }
                return polyline;
            }
        };

        class Snowman : public svg::Drawable {
        public:
            Snowman(svg::Point head, double rad)
                : head_(head), rad_(rad) {}

            void Draw(svg::ObjectContainer& container) const override {
                Circle up;
                up.SetCenter(head_)
                    .SetRadius(rad_)
                    .SetFillColor("rgb(240,240,240)"s)
                    .SetStrokeColor("black"s);
                Circle middle;
                middle.SetCenter({ head_.x, head_.y + 2 * rad_ })
                    .SetRadius(1.5 * rad_).SetFillColor("rgb(240,240,240)"s)
                    .SetStrokeColor("black"s);
                Circle down;
                down.SetCenter({ head_.x, head_.y + 5 * rad_ })
                    .SetRadius(2 * rad_)
                    .SetFillColor("rgb(240,240,240)"s)
                    .SetStrokeColor("black"s);
                container.Add(down);
                container.Add(middle);
                container.Add(up);
            }
        private:
            Point head_;
            double rad_;
        };

    } // namespace shapes

}  // namespace svg