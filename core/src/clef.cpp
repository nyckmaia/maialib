#include "clef.h"

// Clef::Clef()
//{

//}

Clef::Clef(const std::string& sign, int line) : _sign(sign), _line(line) {}

Clef::~Clef() {}

std::string Clef::getSign() const { return _sign; }

void Clef::setSign(const std::string& sign) { _sign = sign; }

int Clef::getLine() const { return _line; }

void Clef::setLine(int line) { _line = line; }
