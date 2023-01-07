#pragma once

#include <string>

class Clef {
   private:
    std::string _sign;
    int _line;

   public:
    // Clef();
    Clef(const std::string& sign = "G", int line = 2);
    ~Clef();

    int getLine() const;
    void setLine(int line);

    std::string getSign() const;
    void setSign(const std::string& sign);
};
