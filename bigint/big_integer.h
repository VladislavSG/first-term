#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <vector>
#include <functional>

struct big_integer
{
    big_integer();
    big_integer(int);
    big_integer(uint32_t);
    explicit big_integer(std::string const&);
    big_integer(big_integer const&) = default;
    big_integer& operator=(big_integer const&) = default;

    big_integer& operator+=(big_integer const&);
    big_integer& operator-=(big_integer const&);
    big_integer& operator*=(big_integer const&);
    big_integer& operator/=(big_integer const&);
    big_integer& operator%=(big_integer const&);

    big_integer& bit_operation(big_integer const&,
            std::function<uint32_t(uint32_t, uint32_t)> const&);
    big_integer& operator&=(big_integer const&);
    big_integer& operator|=(big_integer const&);
    big_integer& operator^=(big_integer const&);

    big_integer& operator<<=(unsigned int);
    big_integer& operator>>=(unsigned int);

    big_integer operator+() const;
    big_integer operator-() const;
    big_integer operator~() const;

    big_integer& operator++();
    big_integer operator++(int);

    big_integer& operator--();
    big_integer operator--(int);

    big_integer abs() const;
    bool isPositive() const;
    big_integer& negateIp();
    big_integer& inverseIp();
    big_integer& absInPlace();

    big_integer& shiftedSubIp(big_integer const&, size_t);
    big_integer& shiftedAddIp(big_integer const&, size_t);
    big_integer& divAbsLongDigitIp(uint32_t x);

    friend bool operator==(big_integer const&, big_integer const&);
    friend bool operator!=(big_integer const&, big_integer const&);
    friend bool operator<(big_integer const&, big_integer const&);
    friend bool operator>(big_integer const&, big_integer const&);
    friend bool operator<=(big_integer const&, big_integer const&);
    friend bool operator>=(big_integer const&, big_integer const&);

    friend std::string to_string(big_integer const&);

private:
    std::vector<uint32_t> digits_{}; //храним в little endian в дополнительном коде
    const size_t BIT_IN_DIGIT = 8 * sizeof(uint32_t);

    big_integer& shiftedAbstIp(big_integer const &, size_t, uint32_t,
            std::function<uint32_t(uint32_t)> const&);
    big_integer& shiftedSubVectorIp(big_integer const&, size_t);
    big_integer& trim();
    void reserve(size_t);
    uint32_t getDigit(size_t) const;

    static uint32_t bitCount(uint32_t);
    static bool isPositive(uint32_t);
    static bool vectorAbsSmaller(big_integer const&, big_integer const&);
};

big_integer operator+(big_integer, big_integer const&);
big_integer operator-(big_integer, big_integer const&);
big_integer operator*(big_integer, big_integer const&);
big_integer operator/(big_integer, big_integer const&);
big_integer operator%(big_integer, big_integer const&);

big_integer operator&(big_integer, big_integer const&);
big_integer operator|(big_integer, big_integer const&);
big_integer operator^(big_integer, big_integer const&);

big_integer operator<<(big_integer, unsigned int);
big_integer operator>>(big_integer, unsigned int);

bool operator==(big_integer const&, big_integer const&);
bool operator!=(big_integer const&, big_integer const&);
bool operator<(big_integer const&, big_integer const&);
bool operator>(big_integer const&, big_integer const&);
bool operator<=(big_integer const&, big_integer const&);
bool operator>=(big_integer const&, big_integer const&);

std::string to_string(big_integer const&);
std::ostream& operator<<(std::ostream& s, big_integer const&);

#endif // BIG_INTEGER_H
