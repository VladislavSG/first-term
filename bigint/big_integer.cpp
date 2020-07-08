#include "big_integer.h"
#include <sstream>

big_integer::big_integer() : big_integer(0) {}

big_integer::big_integer(int x) {
    digits_.push_back(x);
}

big_integer::big_integer(uint32_t x) {
    digits_.push_back(x);
    if (!isPositive(x)) {
        digits_.push_back(0);
    }
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
    if (!isPositive) {
        negateIp();
    }
}

big_integer& big_integer::shiftedAbstIp(big_integer const& rhs, size_t pos, uint32_t start,
                                        std::function<uint32_t(uint32_t)> const& operation) {
    size_t new_size = (digits_.size() > rhs.digits_.size() ? digits_.size() : rhs.digits_.size());
    reserve(new_size + 1);

    uint64_t carry_bit = start;
    for (size_t i = pos; i < new_size + 1; ++i) {
        carry_bit += static_cast<uint64_t>(digits_[i]) + operation(rhs.getDigit(i - pos));
        digits_[i] = carry_bit;
        carry_bit >>= 32u;
    }
    return trim();
}

big_integer& big_integer::shiftedAddIp(big_integer const& rhs, size_t pos) {
    return shiftedAbstIp(rhs, pos, 0, [](uint32_t a) { return a; });
}

big_integer& big_integer::shiftedSubIp(big_integer const& rhs, size_t pos) {
    return shiftedAbstIp(rhs, pos, 1, [](uint32_t a) { return ~a; });
}

big_integer& big_integer::shiftedSubVectorIp(big_integer const& rhs, size_t pos) {
    big_integer temp(rhs);
    if (!rhs.isPositive())
        temp.digits_.push_back(0u);         // it isn't negate!
    return shiftedSubIp(temp, pos);
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
    return shiftedAddIp(rhs, 0);
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
    return shiftedSubIp(rhs, 0);
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
    big_integer result;
    result.digits_.resize(digits_.size() + rhs.digits_.size() + 1);
    for (size_t i = 0; i < digits_.size(); ++i) {
        uint64_t carry_digit = 0;
        for (size_t j = 0; j < rhs.digits_.size(); ++j) {
            carry_digit += static_cast<uint64_t>(digits_[i]) * rhs.digits_[j] + result.digits_[i+j];
            result.digits_[i+j] = carry_digit;
            carry_digit = (carry_digit >> 32u);
        }
        result.digits_[i + rhs.digits_.size()] += carry_digit;
    }
    if (!rhs.isPositive()) {            // не забываем внести ПОПРАВКИ
        result.shiftedSubVectorIp(*this, rhs.digits_.size());
    }
    if (!isPositive()) {
        result.shiftedSubVectorIp(rhs, digits_.size());
    }
    if (!rhs.isPositive() && !isPositive()) {
        result.shiftedAddIp(big_integer(1),digits_.size() + rhs.digits_.size());
    }
    result.trim();
    std::swap(*this, result);
    return *this;
}

big_integer& big_integer::divAbsLongDigitIp(uint32_t x) {
    absInPlace();
    uint64_t carry = 0;
    for (size_t i = digits_.size(); i > 0; --i) {
        uint64_t cur_val = digits_[i - 1] + (carry << 32u);
        digits_[i - 1] = static_cast<uint32_t>(cur_val / x);
        carry = cur_val % x;
    }
    return trim();
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    bool resultPositive = (isPositive() == rhs.isPositive());
    absInPlace();

    if (rhs.digits_.size() == 1 || (rhs.digits_.size() == 2 && rhs.digits_.back() == 0)) {
        divAbsLongDigitIp(rhs.abs().digits_[0]);
        if (!resultPositive) {
            negateIp();
        }
    } else {
        big_integer divisor(rhs.abs());
        big_integer result;
        if (*this >= divisor) {
            uint32_t divisorBack;
            size_t divisorSize = divisor.digits_.size();
            if (divisor.digits_.back()) {
                divisorBack = divisor.digits_.back();
            } else {
                divisorBack = divisor.digits_[divisor.digits_.size() - 2];
                --divisorSize;
            }
            size_t shift = BIT_IN_DIGIT - bitCount(divisorBack);
            divisor <<= shift;
            *this <<= shift;
            divisorBack = divisor.digits_[divisor.digits_.size() - 2];
            result.reserve(digits_.size() - divisorSize + 1);
            for (size_t k = digits_.size() - divisorSize + 1; k > 0; --k) {
                uint32_t q = ((static_cast<uint64_t>(getDigit(k + divisorSize - 1)) << 32u) +
                              getDigit(k + divisorSize - 2)) / divisorBack;
                shiftedSubIp(divisor * q, k - 1);
                while (!isPositive()) {     // works no more than 2 times
                    shiftedAddIp(divisor, k - 1);
                    --q;
                }
                result.digits_[k - 1] = q;
            }
            if (!resultPositive) {
                result.negateIp();
            }
        }
        std::swap(*this, result);
    }
    return trim();
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
    return *this -= big_integer(*this) / rhs * rhs;
}

big_integer& big_integer::bit_operation(big_integer const& rhs, const
std::function<uint32_t(uint32_t, uint32_t)>& operation) {
    size_t max_size = (rhs.digits_.size() > digits_.size() ? rhs.digits_.size() : digits_.size());
    reserve(max_size);
    for (size_t i = 0; i < max_size; ++i) {
        digits_[i] = operation(digits_[i], rhs.getDigit(i));
    }
    return trim();
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
    return bit_operation(rhs, [](uint32_t a, uint32_t b) { return a & b; });
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
    return bit_operation(rhs, [](uint32_t a, uint32_t b) { return a | b; });
}

big_integer& big_integer::operator^=(big_integer const& rhs)
{
    return bit_operation(rhs, [](uint32_t a, uint32_t b) { return a ^ b; });
}

big_integer& big_integer::operator<<=(unsigned int rhs) {
    if (rhs == 0) {
        return *this;
    }
    unsigned int digit_count = rhs / BIT_IN_DIGIT;
    unsigned int bit_count_l = rhs % BIT_IN_DIGIT;
    unsigned int bit_count_r =  BIT_IN_DIGIT - bit_count_l;
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
    return trim();
}

big_integer& big_integer::operator>>=(unsigned int rhs) {
    if (rhs == 0) {
        return *this;
    }
    size_t digit_size = digits_.size();
    unsigned int digit_count = rhs / BIT_IN_DIGIT;
    unsigned int bit_count_r = rhs % BIT_IN_DIGIT;
    unsigned int bit_count_l = BIT_IN_DIGIT - bit_count_r;
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

    for (; i < digit_size; ++i) {
        digits_.pop_back();
    }
    return trim();
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    return big_integer(*this).negateIp();
}

big_integer big_integer::operator~() const {
    return big_integer(*this).inverseIp();
}

big_integer& big_integer::operator++() {
    return *this += 1;
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer& big_integer::operator--() {
    return *this += -1;
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

bool big_integer::vectorAbsSmaller(big_integer const& a, big_integer const& b) {
    if (a.digits_.size() != b.digits_.size()) {
        return a.digits_.size() < b.digits_.size();
    } else {
        return std::lexicographical_compare(a.digits_.rbegin(), a.digits_.rend(),
                                            b.digits_.rbegin(), b.digits_.rend());
    }
}

bool operator==(big_integer const& a, big_integer const& b) {
    if (a.isPositive() != b.isPositive() || a.digits_.size() != b.digits_.size()) {
        return false;
    }
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
    if (a.isPositive() != b.isPositive()) {
        return b.isPositive();
    }
    if (a.digits_.size() != b.digits_.size()) {
        return a.isPositive() == (a.digits_.size() < b.digits_.size());
    }
    return big_integer::vectorAbsSmaller(a, b);
}

bool operator>(big_integer const& a, big_integer const& b) {
    return b < a;
}

bool operator<=(big_integer const& a, big_integer const& b) {
    return !(b < a);
}

bool operator>=(big_integer const& a, big_integer const& b) {
    return !(a < b);
}

std::string to_string(big_integer const& a) {
    if (a == 0) {
        return "0";
    }
    int const divisor = 1000000000;
    std::vector<uint32_t> buffer;
    std::stringstream s;
    big_integer r = a.abs();
    while (r > 0) {
        buffer.push_back((r % divisor).digits_[0]);
        r.divAbsLongDigitIp(divisor);
    }

    if (!a.isPositive()) {
        s << "-";
    }
    s << buffer.back();
    for (size_t i = buffer.size() - 1; i > 0; --i) {
        char nine_chars[10];
        sprintf(nine_chars, "%09d", buffer[i - 1]);
        s << nine_chars;
    }
    return s.str();
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
    return s << to_string(a);
}

bool big_integer::isPositive() const {
    return isPositive(digits_.back());
}

big_integer& big_integer::trim() {
    // test digits_.back() == 0 or UINT32_MAX
    while (digits_.size() > 1 && (digits_.back() + 1 < 2) &&
    ((digits_.back() == 0) == isPositive(digits_[digits_.size() - 2]))) {
        digits_.pop_back();
    }
    return *this;
}

big_integer& big_integer::absInPlace() {
    if (isPositive()) {
        return *this;
    } else {
        return negateIp();
    }
}

void big_integer::reserve(size_t new_size) {
    if (digits_.size() >= new_size) {
        return;
    }
    uint32_t filler = (isPositive() ? 0 : UINT32_MAX);
    for (size_t i = digits_.size(); i < new_size; ++i) {
        digits_.push_back(filler);
    }
}

uint32_t big_integer::getDigit(size_t i) const {
    if (i < digits_.size()) {
        return digits_[i];
    } else {
       if (isPositive()) {
           return 0;
       } else {
           return UINT32_MAX;
       }
    }
}

uint32_t big_integer::bitCount(uint32_t d) {
    uint32_t count = 0;
    while (d > 0) {
        ++count;
        d /= 2;
    }
    return count;
}

bool big_integer::isPositive(uint32_t x) {
    return (x >> 31u) == 0;
}

big_integer& big_integer::negateIp() {
    reserve(digits_.size() + 1);
    ++inverseIp();
    return trim();
}

big_integer& big_integer::inverseIp() {
    for (auto &i : digits_) {
        i = ~i;
    }
    return trim();
}

big_integer big_integer::abs() const {
    return big_integer(*this).absInPlace();
}
