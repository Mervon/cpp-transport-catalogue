#define _USE_MATH_DEFINES

#include "svg.h"

#include <utility>
#include <cmath>
#include <cassert>

using namespace std;

namespace svg {

void ColorPrinter::operator()(std::monostate) const {
        out << "\"none\""sv;
}
void ColorPrinter::operator()(svg::Rgb rgb) const {
    out << "\"rgb("sv << std::to_string(rgb.red) << ',' << std::to_string(rgb.green) << ',' << std::to_string(rgb.blue) << ")\""sv;
}
void ColorPrinter::operator()(svg::Rgba rgba) const {
    out << "\"rgba("sv << std::to_string(rgba.red) << ',' << std::to_string(rgba.green) << ',' << std::to_string(rgba.blue) << ',' << rgba.opacity << ")\""sv;
}
void ColorPrinter::operator()(std::string str) const {
    out << "\""sv << str << "\""sv;
}

Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b) {
    red = r;
    green = g;
    blue = b;
}

Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double o) {
    red = r;
    green = g;
    blue = b;
    opacity = o;
}

std::ostream& operator<<(std::ostream &out, const StrokeLineCap& slc) {
    switch(slc) {
        case StrokeLineCap::BUTT  : out << "butt"sv;   break;
        case StrokeLineCap::ROUND: out << "round"sv; break;
        case StrokeLineCap::SQUARE : out << "square"sv;  break;
    }
    return out;
}

std::ostream& operator<<(std::ostream &out, const StrokeLineJoin& slj) {
    switch(slj) {
        case StrokeLineJoin::ARCS  : out << "arcs"sv;   break;
        case StrokeLineJoin::BEVEL: out << "bevel"sv; break;
        case StrokeLineJoin::MITER : out << "miter"sv;  break;
        case StrokeLineJoin::MITER_CLIP : out << "miter-clip"sv;  break;
        case StrokeLineJoin::ROUND : out << "round"sv;  break;
    }
    return out;
}

using namespace std;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(move(point));
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    size_t points_size = points_.size();
    if (points_size == 0) {
        out << "\" />"sv;
        return;
    }
    for (size_t i = 0; i < points_size; ++i) {
        if (points_size - 1 != i) {
            out << points_[i].x << ',' << points_[i].y << ' ';
        } else {
            out << points_[i].x << ',' << points_[i].y << "\" "sv;
        }
    }

    RenderAttrs(out);

    out << "/>"sv;
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
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    RenderAttrs(context.out);
    out << '>';
    if (!data_.empty()) {
        for (char c : data_) {
            if (c == '\"') {
                out << "&quot;"sv;
            } else if (c == '\'') {
                out << "&apos;"sv;
            } else if (c == '<') {
                out << "&lt;"sv;
            } else if (c == '>') {
                out << "&gt;"sv;
            } else if (c == '&') {
                out << "&amp;"sv;
            } else {
                out << c;
            }
        }
    }
    out << "</text>";
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    if (!objects_.empty()) {
        RenderContext ctx(out, 2, 2);
        for (auto& object : objects_) {
            object->Render(ctx);
        }
    }
    out << "</svg>"sv;
}


}  // namespace svg

namespace shapes {

// ---------- Triangle ------------------

Triangle::Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1)
        , p2_(p2)
        , p3_(p3) {
}

void Triangle::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}

// ---------- Snowman ------------------

Snowman::Snowman(svg::Point head_center, double head_radius) : head_center_(head_center), head_radius_(head_radius) {

    }

void Snowman::Draw(svg::ObjectContainer& container) const {

    container.Add(svg::Circle()
                  .SetCenter({head_center_.x, head_center_.y + head_radius_ * 5})
                  .SetRadius(head_radius_ * 2)
                  .SetFillColor("rgb(240,240,240)")
                  .SetStrokeColor("black"));
    container.Add(svg::Circle()
                  .SetCenter({head_center_.x, head_center_.y + head_radius_ * 2})
                  .SetRadius(head_radius_ * 1.5)
                  .SetFillColor("rgb(240,240,240)")
                  .SetStrokeColor("black"));
    container.Add(svg::Circle()
                  .SetCenter({head_center_.x, head_center_.y})
                  .SetRadius(head_radius_)
                  .SetFillColor("rgb(240,240,240)")
                  .SetStrokeColor("black"));

}


// ---------- Star ------------------

Star::Star(svg::Point center, double outer_rad, double inner_rad, int num_rays) : center_(center), outer_rad_(outer_rad), inner_rad_(inner_rad), num_rays_(num_rays) {

}

void Star::Draw(svg::ObjectContainer& container) const {
    using namespace svg;
    Polyline polyline;
    polyline.SetFillColor("red"s);
    polyline.SetStrokeColor("black"s);
    for (int i = 0; i <= num_rays_; ++i) {
        double angle = 2 * M_PI * (i % num_rays_) / num_rays_;
        polyline.AddPoint({center_.x + outer_rad_ * sin(angle), center_.y - outer_rad_ * cos(angle)});
        if (i == num_rays_) {
            break;
        }
        angle += M_PI / num_rays_;
        polyline.AddPoint({center_.x + inner_rad_ * sin(angle), center_.y - inner_rad_ * cos(angle)});
    }
    container.Add(polyline);
}

}  // namespace shapes