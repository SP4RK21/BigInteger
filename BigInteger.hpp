#ifndef BIGINTEGER_HPP
#define BIGINTEGER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include "math.h"
using std::istream;
using std::ostream;
using std::string;
using std::vector;

template<typename T, size_t Base = std::numeric_limits<T>::max()>
class BigInteger;

template<typename T, size_t Base>
ostream & operator <<(ostream &, const BigInteger<T, Base> &);
template<typename T, size_t Base>
istream & operator >>(istream &, BigInteger<T, Base> &);

template<typename T, size_t Base>
class BigInteger {
    friend ostream & operator << <>(ostream &, const BigInteger<T, Base> &);
    friend istream & operator >> <>(istream &, BigInteger<T, Base> &);
    template<typename T1, size_t Base1> friend class BigInteger;
    public:
    explicit BigInteger(long long value = 0);
    explicit BigInteger(string value);
    template<typename NewT, size_t NewBase>
    explicit operator BigInteger<NewT, NewBase>() const;
    
    BigInteger(const BigInteger &);
    BigInteger(BigInteger &&);
    BigInteger& operator =(const BigInteger &);
    
    BigInteger & operator +=(const BigInteger &);
    
    
    std::pair<T, T> safe_add(T, T);
    std::pair<T, T> safe_multiply(T,T);
    BigInteger & operator *=(const BigInteger &);
    BigInteger operator *(const BigInteger &);
    BigInteger operator +(const BigInteger &) const;
    BigInteger & operator -=(const BigInteger &);
    BigInteger operator -(const BigInteger &) const;
    BigInteger operator -() const;
    
    bool operator >(const BigInteger &) const;
    bool operator <(const BigInteger &) const;
    bool operator >=(const BigInteger &) const;
    bool operator <=(const BigInteger &) const;
    bool operator ==(const BigInteger &) const;
    bool operator !=(const BigInteger &) const;
    BigInteger abs(const BigInteger &) const;
    void trim();
    
    size_t size() const;
    T getDigit(size_t) const;
    T operator[](size_t) const;
    void fixLength(size_t i);
    
    private:
    template<typename digitT>
    explicit BigInteger(vector<digitT>, bool);
    void setDigit(size_t, T);
    const size_t bitsForDigit = ceil(log2(Base));
    const size_t blocksInType = (sizeof(T) * 8) / bitsForDigit;
    vector<T> v;
    size_t numberSize = 0;
    bool positive = true;
};

#endif // BIGINTEGER_H


