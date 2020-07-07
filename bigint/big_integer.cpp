#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>

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
    size_t i = 0;
    bool isPositive = true;
    if (str[0] == '-') {
        isPositive = false;
        i = 1;
    }
    for (;i < str.size(); ++i) {
        (*this *= 10) += (str[i] - '0');
    }
    if (!isPositive)
        negate();
}

bool isAddOverflow(uint32_t a, uint32_t b) {
    return (UINT32_MAX - a < b);
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
    size_t new_size = (digits_.size() > rhs.digits_.size() ? digits_.size() : rhs.digits_.size());
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

big_integer &big_integer::shifted_sub_ip(big_integer const& rhs, size_t pos) {
    size_t new_size = (digits_.size() > rhs.digits_.size() ? digits_.size() : rhs.digits_.size());
    reserve(new_size + 1);

    uint32_t carry_bit = 1u;
    for (size_t i = pos; i < new_size + 1; i++) {
        uint32_t new_carry_bit;
        if (isAddOverflow(digits_[i], ~rhs.getDigit(i - pos)) ||
            isAddOverflow(digits_[i] + ~rhs.getDigit(i - pos), carry_bit))
            new_carry_bit = 1u;
        else
            new_carry_bit = 0u;
        digits_[i] += ~rhs.getDigit(i - pos) + carry_bit;
        carry_bit = new_carry_bit;
    }
    trim();
    return *this;
}

big_integer &big_integer::shifted_sub_abs_ip(big_integer const& rhs, size_t pos) {
    big_integer temp(rhs);
    temp.digits_.push_back(0);
    return shifted_sub_ip(temp, pos);
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
    big_integer result;
    result.digits_.resize(digits_.size() + rhs.digits_.size() + 1);
    for (size_t i = 0; i < digits_.size(); i++) {
        uint32_t carry_digit = 0;
        for (size_t j = 0; j < rhs.digits_.size(); j++) {
            uint64_t mul2 = static_cast<uint64_t>(digits_[i])*rhs.digits_[j] + carry_digit;
            uint32_t carry_add_bit = (isAddOverflow(result.digits_[i+j], mul2) ? 1 : 0);
            result.digits_[i+j] += mul2;
            carry_digit = (mul2 >> 32u) + carry_add_bit;
        }
        result.digits_[i + rhs.digits_.size()] += carry_digit;
    }
    if (!rhs.isPositive()) {
        result.shifted_sub_abs_ip(*this, rhs.digits_.size());
    }
    if (!isPositive()) {
        result.shifted_sub_abs_ip(rhs, digits_.size());
        //big_integer temp = rhs << (digits_.size() * sizeof(uint32_t) * 8);
        //temp.digits_.push_back(0);
        //result -= temp;
    }
    if (!rhs.isPositive() && !isPositive())
        result += big_integer(1) <<= ((digits_.size() + rhs.digits_.size()) * sizeof(uint32_t) * 8);
    result.trim();
    std::swap(*this, result);
    return *this;
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    bool result_positive = (isPositive() == rhs.isPositive());
    absInPlace();
    big_integer divisor(rhs.abs());
    big_integer result;
    if (*this >= divisor) {
        uint32_t divisor_back;
        size_t divisor_size = divisor.digits_.size();
        if (divisor.digits_.back()) {
            divisor_back = divisor.digits_.back();
        } else {
            divisor_back = divisor.digits_[divisor.digits_.size() - 2];
            --divisor_size;
        }
        size_t shift = sizeof(uint32_t) * 8 - bitCount(divisor_back);
        divisor <<= shift;
        *this <<= shift;
        divisor_back = divisor.digits_[divisor.digits_.size() - 2];
        result.reserve(digits_.size() - divisor_size + 1);
        divisor <<= ((digits_.size() - divisor_size) * 8 * sizeof(uint32_t));
        for (size_t k = digits_.size() - divisor_size + 1; k > 0; --k) {
            uint32_t q = ((static_cast<uint64_t>(getDigit(k + divisor_size - 1)) << 32u) +
                          getDigit(k + divisor_size - 2)) / divisor_back;
            *this -= divisor * q;
            while (!isPositive()) {
                *this += divisor;
                --q;
            }
            divisor >>= 8 * sizeof(uint32_t);
            result.digits_[k - 1] = q;
        }
        result.trim();
        if (!result_positive)
            result.negate();
    }
    std::swap(*this, result);
    return *this;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
    big_integer r(*this);
    r /= rhs;
    *this -= r * rhs;
    return *this;
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
    size_t max_size = (rhs.digits_.size() > digits_.size() ? rhs.digits_.size() : digits_.size());
    reserve(rhs.digits_.size());
    for (size_t i = 0; i < max_size; ++i) {
        digits_[i] &= rhs.getDigit(i);
    }
    trim();
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
    size_t max_size = (rhs.digits_.size() > digits_.size() ? rhs.digits_.size() : digits_.size());
    reserve(rhs.digits_.size());
    for (size_t i = 0; i < max_size; ++i) {
        digits_[i] |= rhs.getDigit(i);
    }
    trim();
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs)
{
    size_t max_size = (rhs.digits_.size() > digits_.size() ? rhs.digits_.size() : digits_.size());
    reserve(rhs.digits_.size());
    for (size_t i = 0; i < max_size; ++i) {
        digits_[i] ^= rhs.getDigit(i);
    }
    trim();
    return *this;
}

big_integer& big_integer::operator<<=(unsigned int rhs) {
    if (rhs == 0)
        return *this;
    unsigned int digit_count = rhs / (8 * sizeof(uint32_t));
    unsigned int bit_count_l = rhs % (8 * sizeof(uint32_t));
    unsigned int bit_count_r =  sizeof(uint32_t) * 8u - bit_count_l;
    size_t new_size = digits_.size() + digit_count + (bit_count_l ? 1 : 0);
    reserve(new_size);
    if (bit_count_l) {
        for (size_t i = new_size; i > digit_count + 1; --i) {
            digits_[i - 1] = (getDigit(i - digit_count - 1) << bit_count_l) +
                         (getDigit(i - digit_count - 2) >> bit_count_r);
        }
        digits_[digit_count] = (getDigit(0) << bit_count_l);
    } else {
        for (size_t i = new_size; i > digit_count; --i) {
            digits_[i - 1] = digits_[i - digit_count - 1];
        }
    }
    for (size_t i = 0; i < digit_count; ++i) {
        digits_[i] = 0;
    }
    trim();
    return *this;
}

big_integer& big_integer::operator>>=(unsigned int rhs) {
    if (rhs == 0)
        return *this;
    size_t digit_size = digits_.size();
    unsigned int digit_count = rhs / (8 * sizeof(uint32_t));
    unsigned int bit_count_r = rhs % (8 * sizeof(uint32_t));
    unsigned int bit_count_l = (8 * sizeof(uint32_t)) - bit_count_r;
    size_t i = 0;
    if (bit_count_r) {
        for (; i < digit_size - digit_count; ++i) {
            digits_[i] = (getDigit(i + digit_count) >> bit_count_r) +
                         (getDigit(i + digit_count + 1) << bit_count_l);
        }
    } else {
        for (; i < digit_size - digit_count; ++i) {
            digits_[i] = getDigit(i + digit_count);
        }
    }

    for (; i < digit_size; ++i)
         digits_.pop_back();
    trim();
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer r(*this);
    return r.negate();
}

big_integer big_integer::operator~() const {
    big_integer r(*this);
    return r.inverse();
}

big_integer& big_integer::operator++() {
    *this += 1;
    return *this;
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer& big_integer::operator--() {
    *this -= 1;
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
    for (size_t i = a.digits_.size(); i != 0; --i) {
        if (a.digits_[i-1] > b.digits_[i-1])
            return true;
        else if (a.digits_[i-1] < b.digits_[i-1])
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
    int const divisor = 1000000000;
    std::vector<uint32_t> buffer;
    std::stringstream s;
    big_integer r = a.abs();
    while (r > 0) {
        buffer.push_back((r % divisor).digits_[0]);
        r /= divisor;
    }

    if (!a.isPositive())
        s << "-";
    s << buffer.back();
    for (size_t i = buffer.size() - 1; i > 0; --i) {
        char nine[10];
        sprintf(nine, "%09d", buffer[i-1]);
        s << nine;
    }
    return s.str();
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



big_integer& big_integer::absInPlace() {
    if (isPositive())
        return *this;
    else
        return this->negate();
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

big_integer &big_integer::negate() {
    reserve(digits_.size() + 1);
    ++inverse();
    return trim();
}

big_integer &big_integer::inverse() {
    for (auto &i : digits_)
        i = ~i;
    return this->trim();
}

big_integer big_integer::abs() const {
    big_integer r(*this);
    return r.absInPlace();
}
