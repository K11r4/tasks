#include <cmath>
#include <complex>
#include <iostream>
#include <string>
#include <vector>

typedef std::complex<double> comp;

class BigInteger {
 private:
  std::vector<long long> digits;
  bool sign;  // 1 = '+', 0 = '-'
  long long base = 100;

  bool isZero() const;
  void removeLeadingZeros();
  void addOne();
  void subtractOne();
  void addPositive(const BigInteger& a);
  void subtractPositive(const BigInteger& a);
  int reverseBits(int idx, int lg);
  int lgBase();
  void fft(std::vector<comp>& a, bool invert);
  void multOnBigInt(const BigInteger& arg);
  void multOnInt(long long a);
  void devide(const BigInteger& right, bool mode);

 public:
  BigInteger() : digits({0}), sign(1){};

  BigInteger(long long n);

  std::string toString() const;

  bool less(const BigInteger& right) const;
  bool equal(const BigInteger& right) const;

  BigInteger abs() const;

  BigInteger operator-();
  BigInteger& operator++();
  BigInteger operator++(int);
  BigInteger& operator--();
  BigInteger operator--(int);

  BigInteger& operator+=(const BigInteger& right);
  BigInteger& operator-=(const BigInteger& right);
  BigInteger& operator*=(const BigInteger& right);
  BigInteger& operator*=(long long right);
  BigInteger& operator/=(const BigInteger& right);
  BigInteger& operator%=(const BigInteger& right);

  explicit operator bool() const;

  friend std::istream& operator>>(std::istream& is, BigInteger& s);
};

BigInteger::BigInteger(long long n) {
  sign = 1;

  if (n < 0) {
    sign = 0;
    n *= -1;
  }

  if (n == 0) digits.push_back(0);

  while (n) {
    digits.push_back(n % base);
    n /= base;
  }
}

bool operator==(const BigInteger& left, const BigInteger& right) {
  return left.equal(right);
}

bool operator!=(const BigInteger& left, const BigInteger& right) {
  return !left.equal(right);
}

bool operator<(const BigInteger& left, const BigInteger& right) {
  return left.less(right);
}

bool operator>(const BigInteger& left, const BigInteger& right) {
  return right.less(left);
}

bool operator<=(const BigInteger& left, const BigInteger& right) {
  return left.less(right) || left.equal(right);
}

bool operator>=(const BigInteger& left, const BigInteger& right) {
  return right.less(left) or right.equal(left);
}

BigInteger operator+(const BigInteger& left, const BigInteger& right) {
  BigInteger res = left;
  res += right;
  return res;
}

BigInteger operator-(const BigInteger& left, const BigInteger& right) {
  BigInteger res = left;
  res -= right;
  return res;
}

BigInteger operator*(const BigInteger& left, const BigInteger& right) {
  BigInteger res = left;
  res *= right;
  return res;
}

BigInteger operator/(const BigInteger& left, const BigInteger& right) {
  BigInteger res = left;
  res /= right;
  return res;
}

BigInteger operator%(const BigInteger& left, const BigInteger& right) {
  BigInteger res = left;
  res %= right;
  return res;
}

std::ostream& operator<<(std::ostream& os, const BigInteger& a) {
  os << a.toString();
  return os;
}

std::istream& operator>>(std::istream& is, BigInteger& a) {
  std::string num;
  is >> num;
  a.digits.clear();
  a.sign = 1;

  int beg = 0;
  if (num[0] == '-') beg = 1;

  int cur = 0;
  int deg = 1;
  for (int i = num.size() - 1; i >= beg; --i) {
    cur += (num[i] - '0') * deg;
    deg *= 10;
    if ((num.size() - i) % 2 == 0) {  // base!!!!!
      a.digits.push_back(cur);
      cur = 0;
      deg = 1;
    }
  }

  a.digits.push_back(cur);

  while (a.digits.size() > 1 && !a.digits.back()) {
    a.digits.pop_back();
  }

  if (!a.isZero() and beg) a.sign = 0;

  return is;
}

bool BigInteger::isZero() const { return digits.size() == 1 and !digits[0]; }

void BigInteger::removeLeadingZeros() {
  while (digits.size() > 1 && !digits.back()) {
    digits.pop_back();
  }
}

void BigInteger::addOne() {
  digits[0] += 1;
  for (size_t i = 0; i < digits.size(); ++i) {
    if (digits[i] < base) return;
    if (i + 1 == digits.size()) {
      digits.push_back(digits[i] / base);
    } else {
      digits[i + 1] += digits[i] / base;
    }
    digits[i] %= base;
  }
}

void BigInteger::subtractOne() {
  if (isZero()) {
    sign = 0;
    digits[0] = 1;
  } else {
    digits[0] -= 1;
    for (size_t i = 0; i < digits.size(); ++i) {
      if (digits[i] >= 0) break;
      digits[i] += base;
      digits[i + 1] -= 1;
    }
  }

  while (digits.size() > 1 and digits.back() == 0) digits.pop_back();

  if (isZero()) sign = 1;
}

void BigInteger::addPositive(const BigInteger& right) {
  for (size_t i = 0; i < right.digits.size(); ++i) {
    if (i < digits.size()) {
      digits[i] += right.digits[i];
    } else {
      digits.push_back(right.digits[i]);
    }
  }

  for (size_t i = 0; i < digits.size(); ++i) {
    if (digits[i] >= base) {
      if (i + 1 == digits.size()) {
        digits.push_back(digits[i] / base);
      } else {
        digits[i + 1] += digits[i] / base;
      }

      digits[i] %= base;
    }
  }
}

void BigInteger::subtractPositive(const BigInteger& right) {
  for (size_t i = 0; i < right.digits.size(); ++i) {
    if (i < digits.size()) {
      digits[i] -= right.digits[i];
    } else {
      digits.push_back(-right.digits[i]);
    }
  }

  long long resultSign = 1;
  for (size_t i = 0; i < digits.size(); ++i) {
    if (digits[i] < 0) resultSign = -1;
    if (digits[i] > 0) resultSign = 1;
  }

  if (resultSign == -1) sign ^= 1;

  for (size_t i = 0; i < digits.size(); ++i) {
    digits[i] *= resultSign;
  }

  for (size_t i = 0; i + 1 < digits.size(); ++i) {
    if (digits[i] < 0) {
      digits[i + 1]--;
      digits[i] += base;
    }
  }

  while (digits.size() > 1 and digits.back() == 0) digits.pop_back();

  if (isZero()) sign = 1;
}

int BigInteger::reverseBits(int idx, int lg) {
  int res = 0;
  for (int i = 0; i < lg; ++i) {
    if (idx & (1 << i)) res |= (1 << (lg - i - 1));
  }
  return res;
}

int BigInteger::lgBase() {
  int ans = 1;
  while (ans < base) {
    ans *= 10;
  }
  return ans;
}

void BigInteger::fft(std::vector<comp>& target, bool invert) {
  double PI = acos(-1.0);

  int tSize = target.size();
  int log2 = 1;
  while ((1 << log2) < tSize) log2++;

  for (int i = 0; i < tSize; ++i) {
    int j = reverseBits(i, log2);
    if (i < j) {
      comp swap = target[i];
      target[i] = target[j];
      target[j] = swap;
    }
  }

  for (int segmentLen = 2; segmentLen <= tSize; segmentLen *= 2) {
    double ang = PI * 2 / segmentLen * (invert ? 1 : -1);
    comp wlen(cos(ang), sin(ang));
    for (int st = 0; st < tSize; st += segmentLen) {
      comp w(1);
      for (int j = st; j < st + segmentLen / 2; ++j) {
        comp u = target[j], v = target[j + segmentLen / 2] * w;
        target[j] = u + v;
        target[j + segmentLen / 2] = u - v;
        w *= wlen;
      }
    }
  }

  if (invert) {
    for (int i = 0; i < tSize; ++i) {
      target[i] /= tSize;
    }
  }
}

void BigInteger::multOnBigInt(const BigInteger& right) {
  if (isZero()) return;

  if (right.isZero()) {
    digits.clear();
    digits.push_back(0);
    sign = 1;
  }

  sign = (sign == right.sign);

  std::vector<comp> fftLeft(digits.begin(), digits.end());
  std::vector<comp> fftRight(right.digits.begin(), right.digits.end());

  int fftSize = 1;
  while (fftSize < static_cast<int>(fftLeft.size()) or
         fftSize < static_cast<int>(fftRight.size())) {
    fftSize *= 2;
  }
  fftSize *= 2;
  fftLeft.resize(fftSize);
  fftRight.resize(fftSize);

  fft(fftLeft, false);
  fft(fftRight, false);
  for (int i = 0; i < fftSize; ++i) fftLeft[i] *= fftRight[i];
  fft(fftLeft, true);

  digits.resize(fftSize);
  for (int i = 0; i < fftSize; ++i)
    digits[i] = static_cast<long long>(fftLeft[i].real() + 0.5);

  long long carry = 0;
  for (size_t i = 0; i < digits.size(); ++i) {
    digits[i] += carry;
    carry = digits[i] / base;
    digits[i] %= base;
  }

  removeLeadingZeros();
}

void BigInteger::multOnInt(long long right) {
  if (right < 0) {
    right *= -1;
    sign ^= 1;
  }

  for (size_t i = 0; i < digits.size(); ++i) digits[i] *= right;

  for (size_t i = 0; i < digits.size(); ++i) {
    if (digits[i] >= base) {
      if (i == digits.size() - 1) {
        digits.push_back(digits[i] / base);
      } else {
        digits[i + 1] += digits[i] / base;
      }
      digits[i] %= base;
    }
  }
}

void BigInteger::devide(const BigInteger& right,
                        bool mode) {  // 0 is quotient, 1 is remainder
  BigInteger cur, res;
  res.digits.resize(digits.size());
  BigInteger b = right;
  b.sign = 1;
  BigInteger precalc[base];
  precalc[0] = 0;
  for (int i = 1; i < base; ++i) {
    precalc[i] = precalc[i - 1] + b;
  }

  for (int i = digits.size() - 1; i >= 0; --i) {
    cur *= base;
    cur.digits[0] = digits[i];
    cur.removeLeadingZeros();
    long long l = 0, r = base;
    while (r - l > 1) {
      long long m = (r + l) / 2;
      if (precalc[m] <= cur) {
        l = m;
      } else {
        r = m;
      }
    }
    cur -= precalc[l];
    res.digits[i] = l;
  }

  res.removeLeadingZeros();
  if (!res.isZero()) res.sign = (sign == right.sign);

  if (!cur.isZero()) cur.sign = sign;

  if (mode)
    *this = res;
  else
    *this = cur;
}

std::string BigInteger::toString() const {
  std::string result = "";
  for (size_t i = 0; i < digits.size(); ++i) {
    long long curDigit = digits[i];
    long long mod = 1;
    while (mod < base) {
      result += (curDigit % 10 + '0');
      curDigit /= 10;
      mod *= 10;
    }
  }

  while (result.size() > 1 and result.back() == '0') result.pop_back();

  if (!sign) {
    result += '-';
  }

  for (size_t i = 0; i < result.size() / 2; ++i) {
    char swap = result[i];
    result[i] = result[result.size() - i - 1];
    result[result.size() - i - 1] = swap;
  }

  return result;
}

bool BigInteger::less(const BigInteger& right) const {
  if (sign == right.sign) {
    if (digits.size() == right.digits.size()) {
      for (int i = digits.size() - 1; i >= 0; --i) {
        if (digits[i] != right.digits[i])
          return (digits[i] < right.digits[i]) ^ (!sign);
      }
      return false;
    } else {
      return (digits.size() < right.digits.size()) ^ (!sign);
    }
  } else {
    return sign == 0;
  }
}

bool BigInteger::equal(const BigInteger& right) const {
  if (sign != right.sign or digits.size() != right.digits.size()) return false;
  for (size_t i = 0; i < digits.size(); ++i)
    if (digits[i] != right.digits[i]) return false;
  return true;
}

BigInteger BigInteger::abs() const {
  BigInteger result = (*this);
  result.sign = 1;
  return result;
}

BigInteger BigInteger::operator-() {
  BigInteger res = (*this);
  if (!isZero()) res.sign ^= 1;
  return res;
}

BigInteger& BigInteger::operator++() {
  if (sign) {
    addOne();
  } else {
    subtractOne();
  }
  return *this;
}

BigInteger BigInteger::operator++(int) {
  BigInteger res = *this;
  ++(*this);
  return res;
}

BigInteger& BigInteger::operator--() {
  if (sign) {
    subtractOne();
  } else {
    addOne();
  }
  return *this;
}

BigInteger BigInteger::operator--(int) {
  BigInteger res = *this;
  --(*this);
  return res;
}

BigInteger& BigInteger::operator+=(const BigInteger& right) {
  if (sign) {
    if (right.sign)
      addPositive(right);
    else
      subtractPositive(right);
  } else {
    if (right.sign)
      subtractPositive(right);
    else
      addPositive(right);
  }
  return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& right) {
  if (sign) {
    if (right.sign)
      subtractPositive(right);
    else
      addPositive(right);
  } else {
    if (right.sign)
      addPositive(right);
    else
      subtractPositive(right);
  }
  return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& right) {
  multOnBigInt(right);
  return *this;
}

BigInteger& BigInteger::operator*=(long long right) {
  multOnInt(right);
  return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& right) {
  devide(right, 1);
  return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& right) {
  devide(right, 0);
  return *this;
}

BigInteger::operator bool() const { return !isZero(); }

class Rational {
 private:
  BigInteger p, q;

  void normalizeSign();

  void normalizeByGCD();

 public:
  Rational() : p(0ll), q(1ll){};
  Rational(long long a) : p(a), q(1ll){};
  Rational(const BigInteger& a) : p(a), q(1ll){};

  std::string toString() const;

  bool less(const Rational& right) const;
  bool equal(const Rational& right) const;

  std::string asDecimal(size_t precisions) const;

  Rational& operator+=(const Rational& right);
  Rational& operator-=(const Rational& right);
  Rational& operator*=(const Rational& right);
  Rational& operator/=(const Rational& right);
  Rational operator-();

  explicit operator double();
};

Rational operator+(const Rational& left, const Rational& right) {
  Rational res = left;
  res += right;
  return res;
}

Rational operator-(const Rational& left, const Rational& right) {
  Rational res = left;
  res -= right;
  return res;
}

Rational operator*(const Rational& left, const Rational& right) {
  Rational res = left;
  res *= right;
  return res;
}

Rational operator/(const Rational& left, const Rational& right) {
  Rational res = left;
  res /= right;
  return res;
}

bool operator==(const Rational& left, const Rational& right) {
  return left.equal(right);
}

bool operator<(const Rational& left, const Rational& right) {
  return left.less(right);
}

bool operator>(const Rational& left, const Rational& right) {
  return right.less(left);
  ;
}

bool operator!=(const Rational& left, const Rational& right) {
  return !left.equal(right);
}

bool operator<=(const Rational& left, const Rational& right) {
  return left.less(right) or left.equal(right);
}

bool operator>=(const Rational& left, const Rational& right) {
  return right.less(left) or left.equal(right);
}

Rational& Rational::operator+=(const Rational& right) {
  p = p * right.q + q * right.p;
  q *= right.q;
  normalizeSign();
  normalizeByGCD();

  return (*this);
}

Rational& Rational::operator-=(const Rational& right) {
  p = p * right.q - q * right.p;
  q *= right.q;
  normalizeSign();
  normalizeByGCD();

  return (*this);
}

Rational& Rational::operator*=(const Rational& right) {
  p *= right.p;
  q *= right.q;
  normalizeSign();
  normalizeByGCD();

  return (*this);
}

Rational& Rational::operator/=(const Rational& right) {
  p *= right.q;
  q *= right.p;
  normalizeSign();
  normalizeByGCD();

  return (*this);
}

Rational Rational::operator-() {
  Rational res = (*this);
  res.p *= -1;
  return res;
}

Rational::operator double() { return atof((this->asDecimal(24)).c_str()); };

std::string Rational::toString() const {
  std::string res = p.toString();
  if (q != 1) res += "/" + q.toString();
  return res;
};

void Rational::normalizeSign() {
  if (q < 0) {
    p *= -1;
    q *= -1;
  }
}

void Rational::normalizeByGCD() {
  BigInteger a = p.abs(), b = q;

  while (a != 0 and b != 0) {
    if (a > b) {
      a %= b;
    } else {
      b %= a;
    }
  }

  BigInteger d = (a + b);
  p /= d;
  q /= d;
}

bool Rational::less(const Rational& right) const {
  return p * right.q < right.p * q;
}

bool Rational::equal(const Rational& right) const {
  return p * right.q == right.p * q;
}

std::string Rational::asDecimal(size_t precision = 0) const {
  BigInteger shift = 1;
  for (size_t i = 0; i < precision; ++i) shift *= 10;

  BigInteger numberAsDecimal = (p.abs() * shift) / q;
  if ((p.abs() * shift % q) * 2 >= q) numberAsDecimal++;

  std::string digits = numberAsDecimal.toString();
  std::string result = "";
  if (precision == 0) {
    result = digits;
  } else {
    int idx = digits.size() - 1;
    while (precision--) {
      if (idx >= 0) {
        result += digits[idx];
      } else {
        result += '0';
      }
      idx--;
    }

    result += '.';

    while (idx >= 0) {
      result += digits[idx];
      idx--;
    }

    if (result.back() == '.') result += '0';

    for (size_t i = 0; i < result.size() / 2; ++i) {
      char swap = result[i];
      result[i] = result[result.size() - 1 - i];
      result[result.size() - 1 - i] = swap;
    }
  }

  return (p < 0 ? "-" + result : result);
}

