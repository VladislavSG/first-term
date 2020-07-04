#include "big_integer.h"

#include <cstring>
#include <stdexcept>

big_integer::big_integer() : big_integer(0) {}

big_integer::big_integer(int x) {
    digits_.push_back(x);
}

big_integer::big_integer(uint32_t x) {
    digits_.push_back(x);
    if (!isPositive())
        digits_.push_back(0);
}

big_integer::big_integer(std::string const& str) : big_integer(0) {
    for (auto const& c : str) {
        *this = *this * 10 + (c - '0');
    }
}

bool isAddOverflow(uint32_t a, uint32_t b) {
    return (UINT32_MAX - a < b);
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
    size_t new_size = (digits_.size() >= rhs.digits_.size() ? digits_.size() : rhs.digits_.size());
    reserve(new_size + 1);

    uint32_t carry_bit = 0u;
    for (size_t i = 0u; i < new_size + 1; i++) {
        uint32_t new_carry_bit;
        if (isAddOverflow(digits_[i], rhs.getDigit(i)) ||
            isAddOverflow(digits_[i] + rhs.getDigit(i), carry_bit))
                new_carry_bit = 1u;
        else
                new_carry_bit = 0u;
        digits_[i] += rhs.getDigit(i) + carry_bit;
        carry_bit = new_carry_bit;
    }
    trim();
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
    *this += -rhs;
    return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
    big_integer result;
    result.digits_.resize(digits_.size() + rhs.digits_.size());
    for (size_t i = 0; i < digits_.size(); i++) {
        uint32_t carry_digit = 0;
        for (size_t j = 0; j < rhs.digits_.size(); j++) {
            uint64_t mul2 = static_cast<uint64_t>(digits_[i])*rhs.digits_[j] + carry_digit;
            uint32_t carry_add_bit = (isAddOverflow(result.digits_[i+j], mul2) ? 1 : 0);
            result.digits_[i+j] += mul2;
            carry_digit = (mul2 >> 32u) + carry_add_bit;
        }
        if (i + rhs.digits_.size() < result.digits_.size())
            result.digits_[i + rhs.digits_.size()] += carry_digit;
        else
            result.digits_.push_back(carry_digit);
    }
    result.trim();
    *this = result;
    return *this;
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    bool result_positive = (isPositive() == rhs.isPositive());
    *this = abs();
    big_integer divisor(rhs.abs());
    big_integer result;
    if (*this >= divisor) {
        uint32_t divisor_back = (divisor.digits_.back() ? divisor.digits_.back() : divisor.digits_[divisor.digits_.size() - 2]);
        size_t shift = sizeof(uint32_t) * 8 - bitCount(divisor_back);
        divisor <<= shift;
        *this <<= shift;
        divisor_back = divisor.digits_[divisor.digits_.size() - 2];
        size_t divisor_size = divisor.digits_.size() - 1;
        result.reserve(digits_.size() - divisor_size + 1);
        for (size_t k = digits_.size() - divisor_size + 1; k > 0; --k) {
            uint32_t q = ((static_cast<uint64_t>(getDigit(k + divisor_size - 1)) << 32u) +
                           getDigit(k + divisor_size - 2)) / divisor_back;
            *this -= ((divisor * q) << ((k - 1) * 8 * sizeof(uint32_t)));
            while (!isPositive()) {
                *this += divisor;
                --q;
            }
            result.digits_[k-1] = q;
        }
        result.trim();
    }
    if (!result_positive)
        result = -result;
    *this = result;
    return *this;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
    big_integer r(*this);
    r /= rhs;
    *this -= r * rhs;
    return *this;
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
    reserve(rhs.digits_.size());
    for (size_t i = 0; i < rhs.digits_.size(); ++i) {
        digits_[i] &= rhs.digits_[i];
    }
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
    reserve(rhs.digits_.size());
    for (size_t i = 0; i < rhs.digits_.size(); ++i) {
        digits_[i] |= rhs.digits_[i];
    }
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs)
{
    reserve(rhs.digits_.size());
    for (size_t i = 0; i < rhs.digits_.size(); ++i) {
        digits_[i] ^= rhs.digits_[i];
    }
    return *this;
}

big_integer& big_integer::operator<<=(unsigned int rhs) {
    if (rhs == 0)
        return *this;
    unsigned int digit_count = rhs / (8 * sizeof(uint32_t));
    unsigned int bit_count_l = rhs % (8 * sizeof(uint32_t));
    unsigned int bit_count_r =  sizeof(uint32_t) * 8u - bit_count_l;
    size_t new_size = digits_.size() + digit_count + (bit_count_l ? 1 : 0);
    digits_.resize(new_size);
    if (bit_count_l) {
        for (size_t i = new_size - 1; i > digit_count; --i) {
            digits_[i] = (digits_[i - digit_count] << bit_count_l) +
                         (digits_[i - digit_count - 1] >> bit_count_r);
        }
    } else {
        for (size_t i = new_size - 1; i > digit_count; --i) {
            digits_[i] = digits_[i - digit_count];
        }
    }
    digits_[digit_count] = (digits_.front() << bit_count_l);
    for (size_t i = 0; i < digit_count; ++i) {
        digits_[i] = 0;
    }
    return *this;
}

big_integer& big_integer::operator>>=(unsigned int rhs) {
    if (rhs == 0)
        return *this;
    unsigned int digit_count = rhs / (8 * sizeof(uint32_t));
    unsigned int bit_count_r = rhs % (8 * sizeof(uint32_t));
    unsigned int bit_count_l = (8 * sizeof(uint32_t)) - bit_count_r;
    size_t i = 0;
    if (bit_count_l) {
        for (; i < digits_.size() - digit_count - 1; i++) {
            digits_[i] = (digits_[i + digit_count] >> bit_count_r) +
                         (digits_[i + digit_count + 1] << bit_count_l);
        }
    } else {
        for (; i < digits_.size() - digit_count - 1; i++) {
            digits_[i] = digits_[i + digit_count + 1];
        }
    }
    digits_[i] = (digits_.back() >> bit_count_r);
    for (++i; i < digits_.size(); ++i)
         digits_.pop_back();
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer r(*this);
    r.reserve(r.digits_.size() + 1);
    r = ~r + 1;
    return r.trim();
}

big_integer big_integer::operator~() const {
    big_integer r(*this);
    for (auto &i : r.digits_)
        i = ~i;
    return r;
}

big_integer& big_integer::operator++() {
    *this = *this + 1;
    return *this;
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer& big_integer::operator--() {
    *this = *this - 1;
    return *this;
}

big_integer big_integer::operator--(int) {
    big_integer r = *this;
    --*this;
    return r;
}

big_integer operator+(big_integer a, big_integer const& b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, unsigned int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, unsigned int b) {
    return a >>= b;
}

bool operator==(big_integer const& a, big_integer const& b) {
    if (a.isPositive() != b.isPositive() || a.bitCount() != b.bitCount())
        return false;
    for (size_t i = 0; i < a.digits_.size(); ++i) {
        if (a.digits_[i] != b.digits_[i])
            return false;
    }
    return true;
}

bool operator!=(big_integer const& a, big_integer const& b) {
    return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b) {
    return b > a;
}

bool operator>(big_integer const& a, big_integer const& b) {
    if (a.isPositive() != b.isPositive())
        return a.isPositive();
    uint32_t a_bc = a.bitCount();
    uint32_t b_bc = b.bitCount();
    if (a_bc != b_bc) {
        return (a.isPositive() == (a_bc > b_bc));
    }
    for (size_t i = 0; i < a.digits_.size() - 1; ++i) {
        if (a.digits_[i] > b.digits_[i])
            return true;
        else if (a.digits_[i] < b.digits_[i])
            return false;
    }
    return false;
}

bool operator<=(big_integer const& a, big_integer const& b) {
    return !(a > b);
}

bool operator>=(big_integer const& a, big_integer const& b) {
    return !(b > a);
}

std::string to_string(big_integer const& a) {
    if (a == 0)
        return "0";
    big_integer r = a.abs();
    std::string s;
    while (r > 0) {
        s.push_back('0' + (r % 10).digits_[0]);
        r /= 10;
    }
    if (!a.isPositive())
        s.push_back('-');
    for (size_t i = 0; i < s.size()/2; ++i) {
        std::swap(s[i], s[s.size() - i - 1]);
    }
    return s;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
    return s << to_string(a);
}

uint32_t big_integer::bitCount() const {
    uint32_t last_digit = digits_[digits_.size() - 1];
    uint32_t count = sizeof(uint32_t) * (digits_.size() - 1) * 8u;
    return count + bitCount(last_digit);
}

bool big_integer::isPositive() const {
    return isPositive(digits_.back());
}

big_integer& big_integer::trim() {
    if (isPositive()) {
        while (digits_.size() > 1 && digits_.back() == 0 && isPositive(digits_[digits_.size() - 2]))
            digits_.pop_back();
    } else {
        while (digits_.size() > 1 && digits_.back() == UINT32_MAX && !isPositive(digits_[digits_.size() - 2]))
            digits_.pop_back();
    }
    return *this;
}

big_integer big_integer::abs() const {
    big_integer r(*this);
    if (r.isPositive())
        return r;
    else
        return -r;
}

void big_integer::reserve(size_t new_size) {
    if (digits_.size() >= new_size)
        return;
    uint32_t filler = (isPositive() ? 0 : UINT32_MAX);
    for (size_t i = digits_.size(); i < new_size; i++)
        digits_.push_back(filler);
}

uint32_t big_integer::getDigit(size_t i) const {
    if (i < digits_.size()) {
        return digits_[i];
    } else {
       if (isPositive())
           return 0;
       else
           return UINT32_MAX;
    }
}

uint32_t big_integer::bitCount(uint32_t d) {
    uint32_t count = 0;
    while (d > 0) {
        count++;
        d /= 2;
    }
    return count;
}

bool big_integer::isPositive(uint32_t x) {
    return (x >> 31u) == 0;
}
