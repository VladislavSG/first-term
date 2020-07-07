#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <memory>
#include <vector>
#include <iosfwd>

struct big_integer
{
    big_integer();
    big_integer(int a);
    big_integer(uint32_t a);
    explicit big_integer(std::string const& str);

    big_integer& operator=(big_integer const& other) = default;

    big_integer& operator+=(big_integer const& rhs);
    big_integer& operator-=(big_integer const& rhs);
    big_integer& operator*=(big_integer const& rhs);
    big_integer& operator/=(big_integer const& rhs);
    big_integer& operator%=(big_integer const& rhs);

    big_integer& operator&=(big_integer const& rhs);
    big_integer& operator|=(big_integer const& rhs);
    big_integer& operator^=(big_integer const& rhs);

    big_integer& operator<<=(unsigned int rhs);
    big_integer& operator>>=(unsigned int rhs);

    big_integer operator+() const;
    big_integer operator-() const;
    big_integer operator~() const;

    big_integer& operator++();
    big_integer operator++(int);

    big_integer& operator--();
    big_integer operator--(int);

    uint32_t bitCount() const;
    big_integer& absInPlace();
    big_integer abs() const;
    bool isPositive() const;
    big_integer &negate();
    big_integer &inverse();

    big_integer& shifted_sub_ip(big_integer const &rhs, size_t pos);
    big_integer& shifted_sub_abs_ip(big_integer const &rhs, size_t pos);

    friend bool operator==(big_integer const& a, big_integer const& b);
    friend bool operator!=(big_integer const& a, big_integer const& b);
    friend bool operator<(big_integer const& a, big_integer const& b);
    friend bool operator>(big_integer const& a, big_integer const& b);
    friend bool operator<=(big_integer const& a, big_integer const& b);
    friend bool operator>=(big_integer const& a, big_integer const& b);

    friend std::string to_string(big_integer const& a);

private:
    std::vector<uint32_t> digits_{}; //храним в little endian

    big_integer& trim();
    void reserve(size_t);
    uint32_t getDigit(size_t) const;
    static uint32_t bitCount(uint32_t);
    static bool isPositive(uint32_t);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, unsigned int b);
big_integer operator>>(big_integer a, unsigned int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);

#endif // BIG_INTEGER_H
