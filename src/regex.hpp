#ifndef REGEX_HPP
#define REGEX_HPP

#include <algorithm>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

#include <prez/print_stuff.hpp>

// TODO: add AND operator

enum class RgxType { EMPTYSET, EPSILON, CHARACTER, ALT, CONCAT, STAR, NOT, RANGE };

class Regex;
using RgxPtr = std::shared_ptr<Regex>;

class Regex {
public:
  virtual ~Regex(){};
  virtual void toStream(std::ostream& out) const = 0;
  virtual RgxPtr getDeriv(char) const = 0;
  virtual bool isNullable() const = 0;
  virtual RgxType getType() const = 0;
  virtual bool operator==(const Regex& other) const = 0;

  friend std::ostream& operator<<(std::ostream& out, const Regex& rgx) {
    rgx.toStream(out);
    return out;
  }

  friend std::ostream& operator<<(std::ostream& out, Regex* rgx) {
    return out << *rgx;
  }

  friend std::ostream& operator<<(std::ostream& out, RgxPtr rgx) {
    return out << *rgx;
  }
};

class EmptySet : public Regex {
public:
  bool isNullable() const override;
  RgxPtr getDeriv(char) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;
};

class Epsilon : public Regex {
public:
  bool isNullable() const override;
  RgxPtr getDeriv(char) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;
};

class Character : public Regex {
public:
  Character(char c);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;


  char c_;
};

struct RegexVector {
public:
  RegexVector(Regex* r1, Regex* r2);
  RegexVector(RegexVector* rVec, Regex* r);
  RegexVector(std::vector<RgxPtr>&& vec);
  bool operator==(const RegexVector& other) const;
  std::vector<RgxPtr> rgxs_;
};

class Alt : public Regex {
public:
  Alt(RegexVector* rVec);
  ~Alt();
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;


  RegexVector* rVec_;
};

class Concat : public Regex {
public:
  Concat(RegexVector* rVec);
  ~Concat();
  bool isNullable() const override;
  // TODO: Make this better
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;


  RegexVector* rVec_;
};

class Star : public Regex {
public:
  Star(Regex* rgx);
  Star(RgxPtr rgx);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;


  RgxPtr rgx_;
};

class Not : public Regex {
public:
  Not(Regex* rgx);
  Not(RgxPtr rgx);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;


  RgxPtr rgx_;
};

class Range : public Regex {
public:
  Range(char start, char end);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;


  char start_;
  char end_;
};

#endif
