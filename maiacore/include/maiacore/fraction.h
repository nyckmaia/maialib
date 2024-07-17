#pragma once

#include <iostream>
#include <numeric>  // Para std::gcd
#include <sstream>  // Para std::ostringstream e std::istringstream
#include <string>

class Fraction {
   private:
    int _numerador;
    int _denominador;

    void simplify() {
        int gcd = std::gcd(_numerador, _denominador);
        _numerador /= gcd;
        _denominador /= gcd;
        if (_denominador < 0) {  // Manter o denominador positivo
            _numerador = -_numerador;
            _denominador = -_denominador;
        }
    }

   public:
    // Construtor
    Fraction(int num = 1, int denom = 1) : _numerador(num), _denominador(denom) { simplify(); }

    // Método para retornar a fração como string
    std::string toString() const {
        std::ostringstream oss;
        oss << _numerador << "/" << _denominador;
        return oss.str();
    }

    // Método para retornar o valor da fração como float
    float getFloatValue() const {
        return static_cast<float>(_numerador) / static_cast<float>(_denominador);
    }

    void setValues(const int numerador, const int denominador) {
        _numerador = numerador;
        _denominador = denominador;
        simplify();
    }

    // Operador de soma
    Fraction operator+(const Fraction& other) const {
        int num = _numerador * other._denominador + other._numerador * _denominador;
        int denom = _denominador * other._denominador;
        return Fraction(num, denom);
    }

    // Operador de subtração
    Fraction operator-(const Fraction& other) const {
        int num = _numerador * other._denominador - other._numerador * _denominador;
        int denom = _denominador * other._denominador;
        return Fraction(num, denom);
    }

    // Operador de multiplicação
    Fraction operator*(const Fraction& other) const {
        int num = _numerador * other._numerador;
        int denom = _denominador * other._denominador;
        return Fraction(num, denom);
    }

    // Operador de divisão
    Fraction operator/(const Fraction& other) const {
        int num = _numerador * other._denominador;
        int denom = _denominador * other._numerador;
        return Fraction(num, denom);
    }

    // Operador +=
    Fraction& operator+=(const Fraction& other) {
        _numerador = _numerador * other._denominador + other._numerador * _denominador;
        _denominador = _denominador * other._denominador;
        simplify();
        return *this;
    }

    // Operador -=
    Fraction& operator-=(const Fraction& other) {
        _numerador = _numerador * other._denominador - other._numerador * _denominador;
        _denominador = _denominador * other._denominador;
        simplify();
        return *this;
    }

    // Operador *=
    Fraction& operator*=(const Fraction& other) {
        _numerador *= other._numerador;
        _denominador *= other._denominador;
        simplify();
        return *this;
    }

    // Operador /=
    Fraction& operator/=(const Fraction& other) {
        _numerador *= other._denominador;
        _denominador *= other._numerador;
        simplify();
        return *this;
    }

    // Operador ==
    bool operator==(const Fraction& other) const {
        return _numerador == other._numerador && _denominador == other._denominador;
    }

    // Operador >
    bool operator>(const Fraction& other) const {
        return _numerador * other._denominador > other._numerador * _denominador;
    }

    // Operador >=
    bool operator>=(const Fraction& other) const { return *this > other || *this == other; }

    // Operador <=
    bool operator<=(const Fraction& other) const { return !(*this > other); }

    // Método para simplificar a fração explicitamente
    void simplifyFraction() { simplify(); }

    static Fraction stringToFraction(const std::string& fractionAsString) {
        int num, denom;
        char slash;
        std::istringstream iss(fractionAsString);
        iss >> num >> slash >> denom;
        return Fraction(num, denom);
    }

    // Operadores de inserção e extração
    friend std::ostream& operator<<(std::ostream& os, const Fraction& fraction) {
        os << fraction.toString();
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Fraction& fraction) {
        char slash;
        is >> fraction._numerador >> slash >> fraction._denominador;
        fraction.simplify();
        return is;
    }
};