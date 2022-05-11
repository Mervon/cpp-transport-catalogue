#include "map_renderer.h"
#include "json.h"

#include <deque>

using namespace std;

svg::Document MapRenderer::Solve() {
    svg::Document document;
    vector<geo::Coordinates> coords_for_sphere_projector = transport_catalogue_.GetAllCoords();
    SphereProjector sp(coords_for_sphere_projector.begin(), coords_for_sphere_projector.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
    LinkSphereProjector(sp);
    deque<Bus> buses = transport_catalogue_.GetAllBusesSortedByName();
    deque<Stop> stops = transport_catalogue_.GetAllStopsSortedByName();
    DrawLines(document, buses);
    DrawFirstAndLastStop(document, buses);
    DrawStopNames(document, stops);
    DrawCircles(document, stops);

    return document;
}

string MapRenderer::PrintResult(svg::Document& document) {
    ostringstream os2;
    document.Render(os2);
    /*json::Node node(os2.str());
    json::Document doc(node);
    json::Print(doc, os);*/
    return os2.str();
}

svg::Color& MapRenderer::GetNextColor(int& color_index, int max_color_index) {
    if (color_index > max_color_index) {
        color_index = 0;
    }

    return render_settings_.color_palette[color_index++];
}

inline const double EPSILON = 1e-6;

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

MapRenderer::MapRenderer(RenderSettings& render_settings, TransportCatalogue::TransportCatalogue& tc) : render_settings_(render_settings), transport_catalogue_(tc) {

}

void MapRenderer::LinkSphereProjector(const SphereProjector& sp) {
    sphere_projector_ = sp;
}

void MapRenderer::DrawLines(svg::Document& document, deque<Bus>& buses) {
    int max_color_index = 0;
    if (render_settings_.color_palette.size() != 0) {
        max_color_index = static_cast<int>(render_settings_.color_palette.size()) - 1;
    } else {
        throw std::out_of_range("No colors in pallite"s);
    }
    int color_index = 0;
    svg::Color bus_color = render_settings_.color_palette[color_index++];

    bool is_first = false;
    for (auto& bus : buses) {
        if (bus.bus_stops.size() == 0) {
            continue;
        }
        if (is_first) {
            bus_color = GetNextColor(color_index, max_color_index);
        }
        bus.color = bus_color;
        svg::Polyline lines_for_bus;
        for (auto& stop: bus.bus_stops) {
            lines_for_bus.AddPoint(sphere_projector_(stop->stop_coords));
        }

        lines_for_bus
            .SetFillColor(svg::NoneColor)
            .SetStrokeColor(bus_color)
            .SetStrokeWidth(render_settings_.line_width)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND);


        document.Add(lines_for_bus);

        is_first = true;
    }
}

void MapRenderer::DrawFirstAndLastStop(svg::Document& document, deque<Bus>& buses) {
    for (auto& bus : buses) {
        if (bus.bus_stops.size() == 0) {
            continue;
        }

        svg::Text bus_name_underline;
        svg::Text bus_name;

        Stop* first_stop = bus.bus_stops[0];

        bus_name
            .SetPosition(sphere_projector_(first_stop->stop_coords))
            .SetOffset(svg::Point{render_settings_.bus_label_offset.first, render_settings_.bus_label_offset.second})
            .SetFontSize(render_settings_.bus_label_font_size)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold"s)
            .SetData(bus.bus_name)
            .SetFillColor(bus.color);

        bus_name_underline
            .SetPosition(sphere_projector_(first_stop->stop_coords))
            .SetOffset(svg::Point{render_settings_.bus_label_offset.first, render_settings_.bus_label_offset.second})
            .SetFontSize(render_settings_.bus_label_font_size)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold"s)
            .SetData(bus.bus_name)
            .SetStrokeColor(render_settings_.underlayer_color)
            .SetFillColor(render_settings_.underlayer_color)
            .SetStrokeWidth(render_settings_.underlayer_width)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND);

        std::map<std::string, AditionalInfo>& add_info = transport_catalogue_.GetAditionalInfo();
        if (add_info.find(bus.bus_name) != add_info.end()) {
            Stop* last_stop = transport_catalogue_.GetStopByName(add_info[bus.bus_name].last_stop);

            svg::Text bus_name_underline2;
            svg::Text bus_name2;

            bus_name2
                .SetPosition(sphere_projector_(last_stop->stop_coords))
                .SetOffset(svg::Point{render_settings_.bus_label_offset.first, render_settings_.bus_label_offset.second})
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetData(bus.bus_name)
                .SetFillColor(bus.color);

            bus_name_underline2
                .SetPosition(sphere_projector_(last_stop->stop_coords))
                .SetOffset(svg::Point{render_settings_.bus_label_offset.first, render_settings_.bus_label_offset.second})
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetData(bus.bus_name)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            document.Add(bus_name_underline);
            document.Add(bus_name);
            document.Add(bus_name_underline2);
            document.Add(bus_name2);
        } else {
            document.Add(bus_name_underline);
            document.Add(bus_name);
        }
    }
}

void MapRenderer::DrawStopNames(svg::Document& document, deque<Stop>& stops) {
    for (auto& stop : stops) {
        if (transport_catalogue_.HaveBuses(stop.stop_name)) {
            Stop* stop_pointer = transport_catalogue_.GetStopByName(stop.stop_name);
            svg::Circle circle_for_stop;

            circle_for_stop
                .SetCenter(sphere_projector_(stop_pointer->stop_coords))
                .SetFillColor("white"s)
                .SetRadius(render_settings_.stop_radius);

            document.Add(circle_for_stop);
        }
    }
}

void MapRenderer::DrawCircles(svg::Document& document, deque<Stop>& stops) {
    for (auto& stop : stops) {
        if (transport_catalogue_.HaveBuses(stop.stop_name)) {
            Stop* stop_pointer = transport_catalogue_.GetStopByName(stop.stop_name);

            svg::Text stop_name;
            svg::Text stop_name_underline;

            stop_name
                .SetPosition(sphere_projector_(stop_pointer->stop_coords))
                .SetOffset(svg::Point{render_settings_.stop_label_offset.first, render_settings_.stop_label_offset.second})
                .SetFontSize(render_settings_.stop_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetData(stop.stop_name)
                .SetFillColor("black"s);

            stop_name_underline
                .SetPosition(sphere_projector_(stop_pointer->stop_coords))
                .SetOffset(svg::Point{render_settings_.stop_label_offset.first, render_settings_.stop_label_offset.second})
                .SetFontSize(render_settings_.stop_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetData(stop.stop_name)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND);

            document.Add(stop_name_underline);
            document.Add(stop_name);
        }
    }
}

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
