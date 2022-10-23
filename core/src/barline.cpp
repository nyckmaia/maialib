#include "barline.h"

Barline::Barline() {}

void Barline::clean() {
    _barStyle.clear();
    _direction.clear();
    _location.clear();
}

std::string Barline::getBarStyle() const { return _barStyle; }

void Barline::setBarStyle(const std::string& barStyle) { _barStyle = barStyle; }

std::string Barline::getDirection() const { return _direction; }

void Barline::setDirection(const std::string& direction) { _direction = direction; }

std::string Barline::getLocation() const { return _location; }

void Barline::setLocation(const std::string& location) { _location = location; }

void Barline::setRepeatStart() {
    setBarStyle("heavy-light");
    setDirection("forward");
}

void Barline::setRepeatEnd() {
    setBarStyle("light-heavy");
    setDirection("backward");
}

std::string Barline::toXML(const int identSize) const {
    std::string xml;

    if (getLocation().empty()) {
        xml.append(Helper::generateIdentation(3, identSize) + "<barline>\n");
    } else {
        xml.append(Helper::generateIdentation(3, identSize) + "<barline location=\"" +
                   getLocation() + "\">\n");
    }

    xml.append(Helper::generateIdentation(4, identSize) + "<bar-style>" + getBarStyle() +
               "</bar-style>\n");

    if (!getDirection().empty()) {
        xml.append(Helper::generateIdentation(4, identSize) + "<repeat direction=\"" +
                   getDirection() + "\"/>\n");
    }

    xml.append(Helper::generateIdentation(3, identSize) + "</barline>\n");

    return xml;
}
