#pragma once

#include "svg.h"
#include "transport_catalogue.h"

#include <string>
#include <vector>
#include <map>
#include <algorithm>

struct RenderSettings{
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    int bus_label_font_size;
    std::pair<double, double> bus_label_offset;
    int stop_label_font_size;
    std::pair<double, double> stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector <svg::Color> color_palette;
};

bool IsZero(double value);

class SphereProjector {
public:
    SphereProjector() = default;

    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding) : padding_(padding) {
    if (points_begin == points_end) {
        return;
    }

    const auto [left_it, right_it]
        = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
              return lhs.lng < rhs.lng;
          });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    const auto [bottom_it, top_it]
        = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
              return lhs.lat < rhs.lat;
          });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        zoom_coeff_ = *height_zoom;
    }
}

class MapRenderer {
public:

    MapRenderer(RenderSettings& render_settings, TransportCatalogue::TransportCatalogue& tc);

    void LinkSphereProjector(const SphereProjector& sp) ;

    svg::Document Solve() ;

    std::string PrintResult(svg::Document& document);

private:
    SphereProjector sphere_projector_;
    RenderSettings render_settings_;
    TransportCatalogue::TransportCatalogue& transport_catalogue_;

    svg::Color& GetNextColor(int& color_index, int max_index);

    void DrawLines(svg::Document& document, std::deque<Bus>& buses);
    void DrawFirstAndLastStop(svg::Document& document, std::deque<Bus>& buses);
    void DrawStopNames(svg::Document& document, std::deque<Stop>& stops);
    void DrawCircles(svg::Document& document, std::deque<Stop>& stops);
};

