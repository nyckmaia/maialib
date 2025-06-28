#pragma once

#include <iostream>
#include <numeric>  // Para std::gcd
#include <sstream>  // Para std::ostringstream e std::istringstream
#include <string>

/// @cond IGNORE_DOXYGEN
/**
 * @brief Represents a mathematical fraction (rational number) with integer numerator and denominator.
 *
 * The Fraction class provides methods for arithmetic operations, simplification, conversion to string and float,
 * and parsing from string. It is useful for representing durations, ratios, and other rational values in music analysis.
 */
class Fraction {
   private:
    int _numerador;   ///< Numerator of the fraction.
    int _denominador; ///< Denominator of the fraction.

    /**
     * @brief Simplifies the fraction to its lowest terms and ensures the denominator is positive.
     */
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
    /**
     * @brief Constructs a Fraction with given numerator and denominator.
     * @param num Numerator (default: 1).
     * @param denom Denominator (default: 1).
     */
    Fraction(int num = 1, int denom = 1) : _numerador(num), _denominador(denom) { simplify(); }

    /**
     * @brief Returns the fraction as a string in the form "numerator/denominator".
     * @return String representation of the fraction.
     */
    std::string toString() const {
        std::ostringstream oss;
        oss << _numerador << "/" << _denominador;
        return oss.str();
    }

    /**
     * @brief Returns the value of the fraction as a float.
     * @return Floating-point value of the fraction.
     */
    float getFloatValue() const {
        return static_cast<float>(_numerador) / static_cast<float>(_denominador);
    }

    /**
     * @brief Sets the numerator and denominator of the fraction and simplifies it.
     * @param numerador Numerator.
     * @param denominador Denominator.
     */
    void setValues(const int numerador, const int denominador) {
        _numerador = numerador;
        _denominador = denominador;
        simplify();
    }

    /**
     * @brief Adds two fractions and returns the result as a new Fraction.
     * @param other The other fraction to add.
     * @return Sum as a new Fraction.
     */
    Fraction operator+(const Fraction& other) const {
        int num = _numerador * other._denominador + other._numerador * _denominador;
        int denom = _denominador * other._denominador;
        return Fraction(num, denom);
    }

    /**
     * @brief Subtracts another fraction from this one and returns the result as a new Fraction.
     * @param other The other fraction to subtract.
     * @return Difference as a new Fraction.
     */
    Fraction operator-(const Fraction& other) const {
        int num = _numerador * other._denominador - other._numerador * _denominador;
        int denom = _denominador * other._denominador;
        return Fraction(num, denom);
    }

    /**
     * @brief Multiplies two fractions and returns the result as a new Fraction.
     * @param other The other fraction to multiply.
     * @return Product as a new Fraction.
     */
    Fraction operator*(const Fraction& other) const {
        int num = _numerador * other._numerador;
        int denom = _denominador * other._denominador;
        return Fraction(num, denom);
    }

    /**
     * @brief Divides this fraction by another and returns the result as a new Fraction.
     * @param other The other fraction to divide by.
     * @return Quotient as a new Fraction.
     */
    Fraction operator/(const Fraction& other) const {
        int num = _numerador * other._denominador;
        int denom = _denominador * other._numerador;
        return Fraction(num, denom);
    }

    /**
     * @brief Adds another fraction to this one (in-place).
     * @param other The other fraction to add.
     * @return Reference to this Fraction.
     */
    Fraction& operator+=(const Fraction& other) {
        _numerador = _numerador * other._denominador + other._numerador * _denominador;
        _denominador = _denominador * other._denominador;
        simplify();
        return *this;
    }

    /**
     * @brief Subtracts another fraction from this one (in-place).
     * @param other The other fraction to subtract.
     * @return Reference to this Fraction.
     */
    Fraction& operator-=(const Fraction& other) {
        _numerador = _numerador * other._denominador - other._numerador * _denominador;
        _denominador = _denominador * other._denominador;
        simplify();
        return *this;
    }

    /**
     * @brief Multiplies this fraction by another (in-place).
     * @param other The other fraction to multiply.
     * @return Reference to this Fraction.
     */
    Fraction& operator*=(const Fraction& other) {
        _numerador *= other._numerador;
        _denominador *= other._denominador;
        simplify();
        return *this;
    }

    /**
     * @brief Divides this fraction by another (in-place).
     * @param other The other fraction to divide by.
     * @return Reference to this Fraction.
     */
    Fraction& operator/=(const Fraction& other) {
        _numerador *= other._denominador;
        _denominador *= other._numerador;
        simplify();
        return *this;
    }

    /**
     * @brief Checks if two fractions are equal.
     * @param other The other fraction to compare.
     * @return True if equal, false otherwise.
     */
    bool operator==(const Fraction& other) const {
        return _numerador == other._numerador && _denominador == other._denominador;
    }

    /**
     * @brief Checks if this fraction is greater than another.
     * @param other The other fraction to compare.
     * @return True if this fraction is greater.
     */
    bool operator>(const Fraction& other) const {
        return _numerador * other._denominador > other._numerador * _denominador;
    }

    /**
     * @brief Checks if this fraction is greater than or equal to another.
     * @param other The other fraction to compare.
     * @return True if this fraction is greater or equal.
     */
    bool operator>=(const Fraction& other) const { return *this > other || *this == other; }

    /**
     * @brief Checks if this fraction is less than or equal to another.
     * @param other The other fraction to compare.
     * @return True if this fraction is less or equal.
     */
    bool operator<=(const Fraction& other) const { return !(*this > other); }

    /**
     * @brief Simplifies the fraction explicitly (to lowest terms).
     */
    void simplifyFraction() { simplify(); }

    /**
     * @brief Converts a string in the form "numerator/denominator" to a Fraction object.
     * @param fractionAsString String representation of the fraction.
     * @return Fraction object.
     */
    static Fraction stringToFraction(const std::string& fractionAsString) {
        int num, denom;
        char slash;
        std::istringstream iss(fractionAsString);
        iss >> num >> slash >> denom;
        return Fraction(num, denom);
    }

    /**
     * @brief Output stream operator for printing the fraction.
     * @param os Output stream.
     * @param fraction Fraction to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Fraction& fraction) {
        os << fraction.toString();
        return os;
    }

    /**
     * @brief Input stream operator for reading a fraction from input.
     * @param is Input stream.
     * @param fraction Fraction to read into.
     * @return Reference to the input stream.
     */
    friend std::istream& operator>>(std::istream& is, Fraction& fraction) {
        char slash;
        is >> fraction._numerador >> slash >> fraction._denominador;
        fraction.simplify();
        return is;
    }
};

/// @endcond