#pragma once
#include "domain.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(catalogue::detail::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct RenderSettings {
    double width_;
    double height_;

    double padding_;

    double line_width_;
    double stop_radius_;

    double bus_label_font_size_;
    std::pair<double, double> bus_label_offset_;

    double stop_label_font_size_;
    std::pair<double, double> stop_label_offset_;

    svg::Color underlayer_color_;
    double underlayer_width_;

    std::vector<svg::Color> color_palette_;
};

class MapRenderer {
public:
    MapRenderer(const RenderSettings& settings);

    SphereProjector GetSphereProjector(std::vector<catalogue::detail::Coordinates> geo_coords) const;

    int GetPaletteSize() const;

    void GetMapDocument(svg::Document& document, const catalogue::TransportCatalogue& catalogue) const;

private:
    const RenderSettings settings_;


    void SetLine(svg::Polyline& line, int palette) const;

    void SetBusText(svg::Text& bus_name, svg::Text& bus_help, int palette) const;

    void SetStopCircle(svg::Circle& stop_circle) const;

    void SetStopText(svg::Text& stop_name, svg::Text& stop_help) const;

    void RenderPolyline(svg::Document& doc, const std::vector<std::pair<BusPtr, int>>& buses_palette, const SphereProjector& proj) const;

    void RenderBusName(svg::Document& doc, const std::vector<std::pair<BusPtr, int>>& buses_palette, const SphereProjector& proj) const;

    void RenderStopCircle(svg::Document& doc, const std::vector<StopPtr>& stops, const SphereProjector& proj) const;

    void RenderStopName(svg::Document& doc, const std::vector<StopPtr>& stops, const SphereProjector& proj) const;
};
