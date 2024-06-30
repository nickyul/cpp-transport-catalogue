#include "map_renderer.h"

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(catalogue::detail::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

MapRenderer::MapRenderer(const RenderSettings& settings)
    : settings_(settings) {}

SphereProjector MapRenderer::GetSphereProjector(std::vector<catalogue::detail::Coordinates> geo_coords) const {
    return SphereProjector{ geo_coords.begin(), geo_coords.end(), settings_.width_, settings_.height_, settings_.padding_ };
}

int MapRenderer::GetPaletteSize() const {
    return settings_.color_palette_.size();
}

void MapRenderer::SetLine(svg::Polyline& line, int palette) const {
    using namespace std::literals;

    line.SetStrokeWidth(settings_.line_width_).
        SetStrokeColor(settings_.color_palette_[palette]).
        SetFillColor("none"s).
        SetStrokeLineCap(svg::StrokeLineCap::ROUND).
        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

void MapRenderer::SetBusText(svg::Text& bus_name, svg::Text& bus_help, int palette) const {
    bus_name.SetOffset({ settings_.bus_label_offset_.first, settings_.bus_label_offset_.second })
        .SetFontSize(settings_.bus_label_font_size_)
        .SetFillColor(settings_.color_palette_[palette]);

    bus_help.SetOffset({ settings_.bus_label_offset_.first, settings_.bus_label_offset_.second })
        .SetFontSize(settings_.bus_label_font_size_)
        .SetFillColor(settings_.underlayer_color_)
        .SetStrokeColor(settings_.underlayer_color_)
        .SetStrokeWidth(settings_.underlayer_width_)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

void MapRenderer::SetStopCircle(svg::Circle& stop_circle) const {
    stop_circle.SetRadius(settings_.stop_radius_);
}

void MapRenderer::SetStopText(svg::Text& stop_name, svg::Text& stop_help) const {
    stop_name.SetOffset({ settings_.stop_label_offset_.first, settings_.stop_label_offset_.second })
        .SetFontSize(settings_.stop_label_font_size_);
    stop_help.SetOffset({ settings_.stop_label_offset_.first, settings_.stop_label_offset_.second })
        .SetFontSize(settings_.stop_label_font_size_)
        .SetFillColor(settings_.underlayer_color_)
        .SetStrokeColor(settings_.underlayer_color_)
        .SetStrokeWidth(settings_.underlayer_width_)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}
