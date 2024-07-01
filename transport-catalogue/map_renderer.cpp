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

void MapRenderer::GetMapDocument(svg::Document& document, const std::vector<BusPtr>& buses, const std::vector<StopPtr>& stops) const {

    std::vector<catalogue::detail::Coordinates> geo_coords;
    std::vector<std::pair<BusPtr, int>> buses_palette;
    int palette_size = GetPaletteSize();
    int palette_index = 0;

    for (BusPtr bus : buses) {

        if (bus->stops.size() > 0) {

            buses_palette.emplace_back(std::make_pair(bus, palette_index));
            palette_index++;

            if (palette_index == palette_size) {
                palette_index = 0;
            }
        }
    }

    for (StopPtr stop : stops) {
        geo_coords.emplace_back(stop->coordinates);
    }

    const SphereProjector proj = GetSphereProjector(geo_coords);

    RenderPolyline(document, buses_palette, proj);

    RenderBusName(document, buses_palette, proj);

    RenderStopCircle(document, stops, proj);

    RenderStopName(document, stops, proj);
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


void MapRenderer::RenderPolyline(svg::Document& doc, const std::vector<std::pair<BusPtr, int>>& buses_palette, const SphereProjector& proj) const {

    for (const auto& [bus, palette] : buses_palette) {
        std::vector<catalogue::detail::Coordinates> bus_stops;
        for (StopPtr stop : bus->stops) {
            bus_stops.emplace_back(stop->coordinates);
        }

        svg::Polyline line;
        bool bus_empty = true;

        for (const catalogue::detail::Coordinates& point : bus_stops) {
            bus_empty = false;
            line.AddPoint(proj(point));
        }

        if (!bus_empty) {
            SetLine(line, palette);
            doc.Add(line);
        }
        bus_stops.clear();
    }
}

void MapRenderer::RenderBusName(svg::Document& doc, const std::vector<std::pair<BusPtr, int>>& buses_palette, const SphereProjector& proj) const {
    using namespace std::literals;

    for (const auto& [bus, palette] : buses_palette) {

        if (bus->stops.size() > 0) {
            svg::Text bus_name_first;
            svg::Text bus_help_first;

            bus_name_first.SetData(bus->bus_name)
                .SetPosition(proj(bus->stops[0]->coordinates))
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s);
            bus_help_first.SetData(bus->bus_name)
                .SetPosition(proj(bus->stops[0]->coordinates))
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s);
            SetBusText(bus_name_first, bus_help_first, palette);
            doc.Add(bus_help_first);
            doc.Add(bus_name_first);

            if (!(bus->is_roundtrip) && bus->stops[(bus->stops.size() / 2)]->stop_name != bus->stops[0]->stop_name) {
                svg::Text bus_name_second;
                svg::Text bus_help_second;
                bus_name_second.SetData(bus->bus_name)
                    .SetPosition(proj(bus->stops[(bus->stops.size() / 2)]->coordinates))
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s);
                bus_help_second.SetData(bus->bus_name)
                    .SetPosition(proj(bus->stops[(bus->stops.size() / 2)]->coordinates))
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s);
                SetBusText(bus_name_second, bus_help_second, palette);
                doc.Add(bus_help_second);
                doc.Add(bus_name_second);
            }
        }
    }
}

void MapRenderer::RenderStopCircle(svg::Document& doc, const std::vector<StopPtr>& stops, const SphereProjector& proj) const {
    using namespace std::literals;

    for (StopPtr stop : stops) {
        svg::Circle stop_circle;
        stop_circle.SetCenter(proj(stop->coordinates))
            .SetFillColor("white"s);
        SetStopCircle(stop_circle);
        doc.Add(stop_circle);
    }
}

void MapRenderer::RenderStopName(svg::Document& doc, const std::vector<StopPtr>& stops, const SphereProjector& proj) const {
    using namespace std::literals;

    for (StopPtr stop : stops) {
        svg::Text stop_name;
        svg::Text stop_help;
        stop_name.SetPosition(proj(stop->coordinates))
            .SetFontFamily("Verdana"s)
            .SetData(stop->stop_name)
            .SetFillColor("black"s);
        stop_help.SetPosition(proj(stop->coordinates))
            .SetFontFamily("Verdana"s)
            .SetData(stop->stop_name);
        SetStopText(stop_name, stop_help);
        doc.Add(stop_help);
        doc.Add(stop_name);
    }
}
