#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <vector>
#include <functional>
#include "my_opt_vector.h"

using storage_t = my_opt_vector<uint32_t>;

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
    big_integer& negateInPlace();
    big_integer& inverseInPlace();
    big_integer& absInPlace();

    big_integer& shiftedSubInPlace(big_integer const&, size_t);
    big_integer& shiftedAddInPlace(big_integer const&, size_t);
    big_integer& divAbsLongDigitInPlace(uint32_t x);

    friend bool operator==(big_integer const&, big_integer const&);
    friend bool operator!=(big_integer const&, big_integer const&);
    friend bool operator<(big_integer const&, big_integer const&);
    friend bool operator>(big_integer const&, big_integer const&);
    friend bool operator<=(big_integer const&, big_integer const&);
    friend bool operator>=(big_integer const&, big_integer const&);

    friend std::string to_string(big_integer const&);
    friend void swap (big_integer &, big_integer &);

private:
    storage_t data_; //храним в little endian в дополнительном коде
    static const size_t BIT_IN_DIGIT = 8 * sizeof(uint32_t);

    big_integer& shiftedAbstractInPlace(big_integer const &, size_t, uint32_t,
                            std::function<uint32_t(uint32_t)> const&, bool);
    big_integer& shiftedSubVectorInPlace(big_integer const&, size_t);
    big_integer& trim();
    void reserve(size_t);
    uint32_t getDigit(size_t) const;
    uint32_t getDigit(size_t, bool) const;

    static uint32_t bitCount(uint32_t);
    static bool isPositive(uint32_t);
    static int vectorCmpThreeWay(big_integer const &a, big_integer const &b);
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

void swap (big_integer &, big_integer &);

#endif // BIG_INTEGER_H
