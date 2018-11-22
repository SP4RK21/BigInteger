#include "BigInteger.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

const int IOBase = 1000000000;

template<typename T, size_t Base>
BigInteger<T, Base>::BigInteger(long long value) {
    if (value < 0) {
        positive = false;
        value = -value;
    }
    size_t i = 0;
    do {
        setDigit(i, value % Base);
        value /= Base;
        i++;
    } while (value);
}

template<typename T, size_t Base>
template<typename NewT, size_t NewBase>
BigInteger<T, Base>::operator BigInteger<NewT, NewBase>() const
{
    vector<NewT> newNumber;
    BigInteger<T, Base> temp(*this);
    while (!(temp.size() == 1 && temp[0] == 0)) {
        size_t holder = 0;
        vector<size_t> divided;
        for (int i = int(temp.size() - 1); i >= 0; --i) {
            holder = holder * Base + temp[i];
            if (holder >= NewBase) {
                divided.push_back(holder / NewBase);
                holder %= NewBase;
            } else if (!divided.empty()){
                divided.push_back(0);
            }
        }
        if (divided.empty()) {
            divided.push_back(0);
        }
        std::reverse(divided.begin(), divided.end());
        temp = BigInteger<T, Base>(divided, positive);
        newNumber.push_back((NewT)holder);
    }
    BigInteger<NewT, NewBase> result(newNumber, positive);
    return result;
}

template<typename T, size_t Base>
BigInteger<T, Base>::BigInteger(string s) {
    if (!s.empty() && (s[0] == '-' || s[0] == '+')) {
        positive = (s[0] == '+');
        s = s.substr(1);
    } else if (s.empty()) {
        s = '0';
    }
    std::vector<size_t> initial;
    s.erase(0, s.find_first_not_of('0'));
    for (int i = (int)s.size() - 1; i >= 0; i -= 9) {
        size_t x = std::stoi(s.substr(std::max(0, (int) s.size() - 9), std::min(9, (int) s.size())));
        if (s.size() < 9) {
            s = "";
        } else {
            s = s.substr(0, (int) s.size() - 9);
        }
        initial.push_back(x);
    }
    
    BigInteger<uint32_t, IOBase> temp(initial, positive);
    *this = static_cast<BigInteger<T, Base>>(temp);
}

template<typename T, size_t Base>
template<typename digitType>
BigInteger<T, Base>::BigInteger(vector<digitType> number, bool pos) {
    size_t ind = 0;
    for (auto i : number) {
        setDigit(ind, i % Base);
        ind++;
    }
    positive = pos;
}

template<typename T, size_t Base>
BigInteger<T, Base>::BigInteger(const BigInteger<T, Base> & bigint) {
    positive = bigint.positive;
    v = bigint.v;
    numberSize = bigint.numberSize;
}

template<typename T, size_t Base>
BigInteger<T, Base>::BigInteger(BigInteger<T, Base> && bigint) {
    positive = bigint.positive;
    v = std::move(bigint.v);
    numberSize = bigint.numberSize;
}

template<typename T, size_t Base>
BigInteger<T, Base>& BigInteger<T, Base>::operator =(const BigInteger<T, Base> & other) {
    positive = other.positive;
    v = other.v;
    numberSize = other.numberSize;
    return *this;
}

template<typename T, size_t Base>
BigInteger<T, Base> & BigInteger<T, Base>::operator +=(const BigInteger<T, Base> &second)
{
    if (positive == second.positive) {
        int carry = 0;
        for (size_t i = 0; i < std::max(numberSize, second.size()) || carry; i++) {
            if (i >= numberSize && i >= v.size() * blocksInType) {
                v.push_back(0);
            }
            T digit;
            if (getDigit(i) >= Base - carry - (i < second.size()? second.getDigit(i) : 0)) {
                digit = (getDigit(i) - (Base - (i < second.size()? second.getDigit(i) : 0) - carry));
                carry = 1;
            } else {
                digit = getDigit(i) + carry + (i < second.size()? second.getDigit(i) : 0);
                carry = 0;
            }
            setDigit(i, digit);
        }
    } else if (!positive && second.positive) {
        *this = (second) + (*this);
    } else {
        *this -= -(second);
    }
    return *this;
}

template<typename T, size_t Base>
std::pair<T, T> BigInteger<T, Base>::safe_add(T first, T second) {
    if (first >= Base - second)
        return std::make_pair(first - (Base - second), 1);
    return std::make_pair(first + second, 0);
}

template<typename T, size_t Base>
std::pair<T, T> BigInteger<T, Base>::safe_multiply(T first, T second) {
    T res = 0, carry = 0, left = 0;
    while (second > 0) {
        if (second & 1) {
            std::pair<T, T> added = safe_add (res, first);
            res = added.first;
            left += carry + added.second;
        }
        std::pair<T, T> added = safe_add (first, first);
        first = added.first;
        carry = (carry << 1) + added.second;
        second >>= 1;
    }
    return std::make_pair(res, left);
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator *(const BigInteger<T, Base> &second) {
    BigInteger<T, Base> result;
    for (size_t i = 0; i < size(); ++i) {
        size_t carry = 0;
        for (size_t j = 0; j < second.size() || carry; ++j) {
            std::pair<T, T> curRes = safe_multiply(getDigit(i), (j < second.size() ? second[j] : 0));
            T newDigit = result[i + j];
            std::pair<T, T> tempRes = safe_add(newDigit, curRes.first);
            curRes.second += tempRes.second;
            newDigit = tempRes.first;
            tempRes = safe_add(newDigit, carry);
            curRes.second += tempRes.second;
            newDigit = tempRes.first;
            result.setDigit(i + j, newDigit);
            carry = curRes.second;
        }
    }
    return result;
}

template<typename T, size_t Base>
BigInteger<T, Base> & BigInteger<T, Base>::operator *=(const BigInteger<T, Base> &second) {
    *this = *this * second;
    return *this;
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator +(const BigInteger<T, Base> &second) const {
    return BigInteger<T, Base>(*this) += second;
}

template<typename T, size_t Base>
BigInteger<T, Base> & BigInteger<T, Base>::operator -=(const BigInteger<T, Base> &second)
{
    if (positive && second.positive) {
        if (abs(*this) < abs(second)) {
            *this = -(second - (*this));
        } else {
            bool carry = 0;
            for (size_t i = 0; i < second.size() || carry; ++i) {
                T a = getDigit(i);
                T b = i < second.size() ? second[i] : 0;
                if (carry > a) {
                    a += Base - (carry ? 1 : 0);
                    carry = 1;
                } else {
                    a -= (carry ? 1 : 0);
                    carry = 0;
                }
                if (a >= b) {
                    a-= b;
                } else {
                    a += (Base - b);
                    carry = 1;
                }
                setDigit(i, a);
            }
            trim();
            
            size_t blocksUsed = std::ceil(double(size()) / blocksInType);
            if (blocksUsed < v.size()) {
                v.resize(blocksUsed);
            }
        }
    } else {
        *this += -(second);
    }
    return (*this);
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator -(const BigInteger<T, Base> &other) const
{
    return BigInteger(*this) -= other;
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::abs(const BigInteger<T, Base> &number) const {
    BigInteger res = number;
    res.positive = true;
    return res;
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator-() const {
    BigInteger res = *this;
    res.positive = !res.positive;
    return res;
}

template<typename T, size_t Base>
size_t BigInteger<T, Base>::size() const
{
    return numberSize;
}

template<typename T, size_t Base>
T BigInteger<T, Base>::getDigit(size_t ind) const {
    if (ind >= numberSize) {
        return 0;
    }
    size_t indexInBlock = ind % blocksInType;
    T block = v[ind / blocksInType];
    block >>= indexInBlock * bitsForDigit;
    if (blocksInType == 1) {
        return block % Base;
    } else {
        return block & ((1 << bitsForDigit) - 1);
    }
}


template<typename T, size_t Base>
T BigInteger<T, Base>::operator[](size_t i) const{
    return getDigit(i);
}

template<typename T, size_t Base>
void BigInteger<T, Base>::setDigit(size_t ind, T x) {
    fixLength(ind);
    size_t j = ind / blocksInType;
    if (blocksInType == 1) {
        v[j] = x % Base;
    } else {
        size_t indexInBlock = ind % blocksInType;
        T mask = (1 << bitsForDigit) - 1;
        x &= mask;
        mask = ~(mask << indexInBlock  * bitsForDigit);
        v[j] &= mask;
        v[j] |= x << indexInBlock * bitsForDigit;
    }
}

template<typename T, size_t Base>
void BigInteger<T, Base>::fixLength(size_t ind){
    if (ind >= size()) {
        for (size_t i = size(); i <= ind; ++i) {
            ++numberSize;
            if (i % blocksInType == 0) {
                v.push_back(0);
            }
        }
    }
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator>(const BigInteger<T, Base> &second) const {
    if (positive != second.positive) {
        return positive;
    }
    if (size() != second.size()) {
        return positive ? size() > second.size() : size() < second.size();
    }
    for (int i = (int)(size() - 1); i >= 0; i--) {
        if (getDigit(i) != second[i]) {
            return positive ? getDigit(i) > second[i] : getDigit(i) < second[i];
        }
    }
    return false;
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator<(const BigInteger<T, Base> &second) const {
    return second > *this;
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator>=(const BigInteger<T, Base> &second) const {
    return !(*this < second);
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator<=(const BigInteger<T, Base> &second) const {
    return !(*this > second);
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator==(const BigInteger<T, Base> &second) const {
    return v == second.v && positive == second.positive;
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator!=(const BigInteger<T, Base> &second) const {
    return !(*this == second);
}

template<typename T, size_t Base>
void BigInteger<T, Base>::trim()  {
    for (size_t i = size() - 1; !(size() == 1 && getDigit(0) == 0) && i >= 1 && getDigit(i) == 0; --i) {
        --numberSize;
    }
}

template<typename T, size_t Base>
ostream & operator << (ostream &stream, const BigInteger<T, Base> &bigint) {
    if (!bigint.positive) {
        stream << '-';
    }
    BigInteger<uint32_t, IOBase> temp = static_cast<BigInteger<uint32_t, IOBase>>(bigint);
    if (temp.v.size() == 0) {
        stream << 0;
    } else {
        stream << temp[int(temp.size()) - 1];
        for (int i = int(temp.size()) - 2; i >= 0; --i) {
            stream << std::setfill('0') << std::setw(9) << temp[i];
        }
    }
    return stream;
}

template<typename T, size_t Base>
istream & operator >>(istream &stream, BigInteger<T, Base> &bigint) {
    std::string temp;
    stream >> temp;
    bigint = BigInteger<T, Base>(temp);
    return stream;
}

