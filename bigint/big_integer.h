#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <vector>
#include <functional>

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

    big_integer& bit_operation(big_integer const& rhs,
            std::function<uint32_t(uint32_t, uint32_t)> const& operation);
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
    big_integer abs() const;
    bool isPositive() const;
    big_integer& negateIp();
    big_integer& inverseIp();
    big_integer& absInPlace();

    big_integer& shiftedSubIp(big_integer const &rhs, size_t pos);
    big_integer& divAbsLongDigitIp(uint32_t x);

    friend bool operator==(big_integer const& a, big_integer const& b);
    friend bool operator!=(big_integer const& a, big_integer const& b);
    friend bool operator<(big_integer const& a, big_integer const& b);
    friend bool operator>(big_integer const& a, big_integer const& b);
    friend bool operator<=(big_integer const& a, big_integer const& b);
    friend bool operator>=(big_integer const& a, big_integer const& b);

    friend std::string to_string(big_integer const& a);

private:
    std::vector<uint32_t> digits_{}; //храним в little endian в дополнительном коде

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
