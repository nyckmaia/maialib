#pragma once

#include <string>
#include "helper.h"

class Barline
{
private:
    std::string _barStyle;
    std::string _direction;
    std::string _location;

public:

    Barline();

    void clean();

    std::string getBarStyle() const;
    void setBarStyle(const std::string& barStyle);

    std::string getDirection() const;
    void setDirection(const std::string& direction);

    std::string getLocation() const;
    void setLocation(const std::string& location);

    void setRepeatStart();
    void setRepeatEnd();

    std::string toXML(const int identSize = 2) const;
};
