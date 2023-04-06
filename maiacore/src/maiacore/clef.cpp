#include "maiacore/clef.h"

#include <map>

#include "maiacore/helper.h"
#include "maiacore/log.h"
#include "maiacore/utils.h"

const std::map<ClefSign, int> c_mapClefSign_Line = {
    {ClefSign::G, 2},
    {ClefSign::F, 4},
    {ClefSign::C, 3},
    {ClefSign::PERCUSSION, 2},
};

// const std::map<int, ClefSign> c_mapLine_ClefSign = {
//     {2, ClefSign::G},
//     {4, ClefSign::F},
//     {3, ClefSign::C},
//     {2, ClefSign::PERCUSSION},
// };

Clef::Clef(const ClefSign sign, int line) : _sign(sign), _line(line), _isClefChanged(false) {
    switch (sign) {
        case ClefSign::G:
            _line = c_mapClefSign_Line.at(ClefSign::G);
            break;
        case ClefSign::F:
            _line = c_mapClefSign_Line.at(ClefSign::F);
            break;
        case ClefSign::C:
            _line = c_mapClefSign_Line.at(ClefSign::C);
            break;
        case ClefSign::PERCUSSION:
            _line = c_mapClefSign_Line.at(ClefSign::PERCUSSION);
            break;
        default:
            LOG_ERROR("Unknow 'sign': " + std::to_string(int(sign)));
            break;
    }
}

Clef::~Clef() {}

ClefSign Clef::getSign() const { return _sign; }

void Clef::setSign(const ClefSign sign) {
    _sign = sign;
    switch (sign) {
        case ClefSign::G:
            _line = c_mapClefSign_Line.at(ClefSign::G);
            break;
        case ClefSign::F:
            _line = c_mapClefSign_Line.at(ClefSign::F);
            break;
        case ClefSign::C:
            _line = c_mapClefSign_Line.at(ClefSign::C);
            break;
        case ClefSign::PERCUSSION:
            _line = c_mapClefSign_Line.at(ClefSign::PERCUSSION);
            break;
        default:
            LOG_ERROR("Unknow 'sign': " + std::to_string(int(sign)));
            break;
    }
    setClefChanged(true);
}

int Clef::getLine() const { return _line; }

void Clef::setLine(int line) {
    if (_sign != ClefSign::C && _sign != ClefSign::PERCUSSION) {
        return;
    }

    _line = line;
    setClefChanged(true);
}

bool Clef::isClefChanged() const { return _isClefChanged; }

void Clef::setClefChanged(bool isClefChanged) { _isClefChanged = isClefChanged; }

std::string Clef::getClefSignStr() const {
    switch (_sign) {
        case ClefSign::G:
            return "G";
            break;
        case ClefSign::F:
            return "F";
            break;
        case ClefSign::C:
            return "C";
            break;
        case ClefSign::PERCUSSION:
            return "percussion";
            break;

        default:
            LOG_ERROR("Unknown 'clefSign': " + std::to_string((int)_sign));
            break;
    }
}

ClefSign Clef::clefSignStr2ClefSign(const std::string& clefStr) {
    switch (hash(clefStr.c_str())) {
        case hash("G"):
            return ClefSign::G;
            break;
        case hash("F"):
            return ClefSign::F;
            break;
        case hash("C"):
            return ClefSign::C;
            break;
        case hash("percussion"):
            return ClefSign::PERCUSSION;
            break;

        default:
            LOG_ERROR("Unknown 'clefStr': " + clefStr);
            break;
    }
}

std::string Clef::toXML(const int clefNumber, const int identSize) const {
    std::string xml;

    if (clefNumber < 0) {
        xml.append(Helper::generateIdentation(4, identSize) + "<clef>\n");
    } else {
        xml.append(Helper::generateIdentation(4, identSize) + "<clef number=\"" +
                   std::to_string(clefNumber + 1) + "\">\n");
    }

    xml.append(Helper::generateIdentation(5, identSize) + "<sign>" + getClefSignStr() +
               "</sign>\n");
    xml.append(Helper::generateIdentation(5, identSize) + "<line>" + std::to_string(_line) +
               "</line>\n");
    xml.append(Helper::generateIdentation(4, identSize) + "</clef>\n");

    return xml;
}