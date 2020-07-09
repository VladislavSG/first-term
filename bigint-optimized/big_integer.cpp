#include "big_integer.h"
#include <sstream>

big_integer::big_integer() : big_integer(0) {}

big_integer::big_integer(int x) {
    data_.push_back(x);
}

big_integer::big_integer(uint32_t x) {
    data_.push_back(x);
    if (!isPositive(x)) {
        data_.push_back(0);
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
        negateInPlace();
    }
}

big_integer& big_integer::shiftedAbstractInPlace(big_integer const& rhs, size_t pos, uint32_t start,
                                   std::function<uint32_t(uint32_t)> const& operation, bool sign) {
    size_t new_size = std::max(data_.size(), rhs.data_.size());
    reserve(new_size + 1);

    uint64_t carry_bit = start;
    for (size_t i = pos; i < new_size + 1; ++i) {
        carry_bit += static_cast<uint64_t>(data_[i]) + operation(rhs.getDigit(i - pos, sign));
        data_[i] = carry_bit;
        carry_bit >>= 32u;
    }
    return trim();
}

big_integer& big_integer::shiftedAddInPlace(big_integer const& rhs, size_t pos) {
    return shiftedAbstractInPlace(rhs, pos, 0, [](uint32_t a) { return a; }, rhs.isPositive());
}

big_integer& big_integer::shiftedSubInPlace(big_integer const& rhs, size_t pos) {
    return shiftedAbstractInPlace(rhs, pos, 1, [](uint32_t a) { return ~a; }, rhs.isPositive());
}

big_integer& big_integer::shiftedSubVectorInPlace(big_integer const& rhs, size_t pos) {
    return shiftedAbstractInPlace(rhs, pos, 1, [](uint32_t a) { return ~a; }, true);
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
    return shiftedAddInPlace(rhs, 0);
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
    return shiftedSubInPlace(rhs, 0);
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
    big_integer result;
    result.data_.resize(data_.size() + rhs.data_.size() + 1);
    for (size_t i = 0; i < data_.size(); ++i) {
        uint64_t carry_digit = 0;
        for (size_t j = 0; j < rhs.data_.size(); ++j) {
            carry_digit += static_cast<uint64_t>(data_[i]) * rhs.data_[j] + result.data_[i + j];
            result.data_[i + j] = carry_digit;
            carry_digit = (carry_digit >> 32u);
        }
        result.data_[i + rhs.data_.size()] += carry_digit;
    }
    if (!rhs.isPositive()) {            // не забываем внести ПОПРАВКИ
        result.shiftedSubVectorInPlace(*this, rhs.data_.size());
    }
    if (!isPositive()) {
        result.shiftedSubVectorInPlace(rhs, data_.size());
    }
    if (!rhs.isPositive() && !isPositive()) {
        result.shiftedAddInPlace(big_integer(1), data_.size() + rhs.data_.size());
    }
    result.trim();
    std::swap(*this, result);
    return *this;
}

big_integer& big_integer::divAbsLongDigitInPlace(uint32_t x) {
    absInPlace();
    uint64_t carry = 0;
    for (size_t i = data_.size(); i > 0; --i) {
        uint64_t cur_val = data_[i - 1] + (carry << 32u);
        data_[i - 1] = static_cast<uint32_t>(cur_val / x);
        carry = cur_val % x;
    }
    return trim();
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    bool resultPositive = (isPositive() == rhs.isPositive());
    absInPlace();

    if (rhs.data_.size() == 1 || (rhs.data_.size() == 2 && rhs.data_.back() == 0)) {
        divAbsLongDigitInPlace(rhs.abs().data_[0]);
        if (!resultPositive) {
            negateInPlace();
        }
    } else {
        big_integer divisor(rhs.abs());
        big_integer result;
        if (*this >= divisor) {
            uint32_t divisorBack;
            size_t divisorSize = divisor.data_.size();
            if (divisor.data_.back()) {
                divisorBack = divisor.data_.back();
            } else {
                divisorBack = divisor.data_[divisor.data_.size() - 2];
                --divisorSize;
            }
            size_t shift = BIT_IN_DIGIT - bitCount(divisorBack);
            divisor <<= shift;
            *this <<= shift;
            divisorBack = divisor.data_[divisor.data_.size() - 2];
            result.reserve(data_.size() - divisorSize + 1);
            for (size_t k = data_.size() - divisorSize + 1; k > 0; --k) {
                uint32_t q = ((static_cast<uint64_t>(getDigit(k + divisorSize - 1)) << 32u) +
                              getDigit(k + divisorSize - 2)) / divisorBack;
                shiftedSubInPlace(divisor * q, k - 1);
                while (!isPositive()) {     // works no more than 2 times
                    shiftedAddInPlace(divisor, k - 1);
                    --q;
                }
                result.data_[k - 1] = q;
            }
            if (!resultPositive) {
                result.negateInPlace();
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
    size_t max_size = (rhs.data_.size() > data_.size() ? rhs.data_.size() : data_.size());
    reserve(max_size);
    for (size_t i = 0; i < max_size; ++i) {
        data_[i] = operation(data_[i], rhs.getDigit(i));
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
    size_t new_size = data_.size() + digit_count + (bit_count_l ? 1 : 0);
    reserve(new_size);
    if (bit_count_l) {
        for (size_t i = new_size; i > digit_count + 1; --i) {
            data_[i - 1] = (getDigit(i - digit_count - 1) << bit_count_l) +
                           (getDigit(i - digit_count - 2) >> bit_count_r);
        }
        data_[digit_count] = (getDigit(0) << bit_count_l);
    } else {
        for (size_t i = new_size; i > digit_count; --i) {
            data_[i - 1] = data_[i - digit_count - 1];
        }
    }
    for (size_t i = 0; i < digit_count; ++i) {
        data_[i] = 0;
    }
    return trim();
}

big_integer& big_integer::operator>>=(unsigned int rhs) {
    if (rhs == 0) {
        return *this;
    }
    size_t digit_size = data_.size();
    unsigned int digit_count = rhs / BIT_IN_DIGIT;
    unsigned int bit_count_r = rhs % BIT_IN_DIGIT;
    unsigned int bit_count_l = BIT_IN_DIGIT - bit_count_r;
    size_t i = 0;
    if (bit_count_r) {
        for (; i < digit_size - digit_count; ++i) {
            data_[i] = (getDigit(i + digit_count) >> bit_count_r) +
                       (getDigit(i + digit_count + 1) << bit_count_l);
        }
    } else {
        for (; i < digit_size - digit_count; ++i) {
            data_[i] = getDigit(i + digit_count);
        }
    }

    for (; i < digit_size; ++i) {
        data_.pop_back();
    }
    return trim();
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    return big_integer(*this).negateInPlace();
}

big_integer big_integer::operator~() const {
    return big_integer(*this).inverseInPlace();
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

int big_integer::vectorCmpThreeWay(big_integer const& a, big_integer const& b) {
    if (a.data_.size() != b.data_.size()) {
        return (a.data_.size() < b.data_.size() ? -1 : 1);
    } else {
        for (size_t i = a.data_.size(); i > 0; i--) {
            if (a.data_[i - 1] < b.data_[i - 1]) {
                return -1;
            } else if (a.data_[i - 1] > b.data_[i - 1]) {
                return 1;
            }
        }
        return 0;
    }
}

bool operator==(big_integer const& a, big_integer const& b) {
    if (a.isPositive() != b.isPositive()) {
        return false;
    }
    return big_integer::vectorCmpThreeWay(a, b) == 0;
}

bool operator!=(big_integer const& a, big_integer const& b) {
    return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b) {
    if (a.isPositive() != b.isPositive()) {
        return b.isPositive();
    }
    return a.isPositive() == (big_integer::vectorCmpThreeWay(a, b) < 0);
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
        buffer.push_back((r % divisor).data_[0]);
        r.divAbsLongDigitInPlace(divisor);
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
    return isPositive(data_.back());
}

big_integer& big_integer::trim() {
    while (data_.size() > 1 && (data_.back() == 0 || data_.back() == UINT32_MAX) &&
           ((data_.back() == 0) == isPositive(data_[data_.size() - 2]))) {
        data_.pop_back();
    }
    return *this;
}

big_integer& big_integer::absInPlace() {
    if (isPositive()) {
        return *this;
    } else {
        return negateInPlace();
    }
}

void big_integer::reserve(size_t new_size) {
    if (data_.size() >= new_size) {
        return;
    }
    uint32_t filler = (isPositive() ? 0 : UINT32_MAX);
    for (size_t i = data_.size(); i < new_size; ++i) {
        data_.push_back(filler);
    }
}

uint32_t big_integer::getDigit(size_t i) const {
    if (i < data_.size()) {
        return data_[i];
    } else {
       if (isPositive()) {
           return 0;
       } else {
           return UINT32_MAX;
       }
    }
}

uint32_t big_integer::getDigit(size_t i, bool sign) const {
    if (i < data_.size()) {
        return data_[i];
    } else {
        if (sign) {
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

big_integer& big_integer::negateInPlace() {
    reserve(data_.size() + 1);
    ++inverseInPlace();
    return trim();
}

big_integer& big_integer::inverseInPlace() {
    for (size_t i = 0; i < data_.size(); ++i) {
        data_[i] = ~data_[i];
    }
    return trim();
}

big_integer big_integer::abs() const {
    return big_integer(*this).absInPlace();
}
