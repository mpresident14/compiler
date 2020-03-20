#ifndef REGEX_HPP
#define REGEX_HPP

#include <algorithm>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

#include <prez/print_stuff.hpp>

// TODO: Implement derivative optimization (use enum and getType())
//   (see https://github.com/hmc-cs132-spring2020/hw1-derivs-mpresident14/blob/develop/Deriv.hs)

class Regex;
using RgxPtr = std::shared_ptr<Regex>;

class Regex {
public:
  virtual ~Regex(){};
  virtual void toStream(std::ostream& out) const = 0;
  virtual RgxPtr getDeriv(char) const = 0;
  virtual bool isNullable() const = 0;

  friend std::ostream& operator<<(std::ostream& out, Regex* rgx) {
    rgx->toStream(out);
    return out;
  }

  friend std::ostream& operator<<(std::ostream& out, RgxPtr rgx) {
    rgx->toStream(out);
    return out;
  }
};

class EmptySet : public Regex {
public:
  bool isNullable() const override;
  RgxPtr getDeriv(char) const override;
  void toStream(std::ostream& out) const override;
};

class Epsilon : public Regex {
public:
  bool isNullable() const override;
  RgxPtr getDeriv(char) const override;
  void toStream(std::ostream& out) const override;
};

class Character : public Regex {
public:
  Character(char c);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  void toStream(std::ostream& out) const override;

private:
  char c_;
};

struct RegexVector {
public:
  RegexVector(Regex* r1, Regex* r2);
  RegexVector(RegexVector* rVec, Regex* r);
  RegexVector(std::vector<RgxPtr>&& vec);
  std::vector<RgxPtr> rgxs_;
};

class Alt : public Regex {
public:
  Alt(RegexVector* rVec);
  ~Alt();
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  void toStream(std::ostream& out) const override;

private:
 RegexVector* rVec_;
};

class Concat : public Regex {
public:
  Concat(RegexVector* rVec);
  ~Concat();
  bool isNullable() const override;
  // TODO: Make this better
  RgxPtr getDeriv(char c) const override;
  void toStream(std::ostream& out) const override;

private:
  RegexVector* rVec_;
};

class Star : public Regex {
public:
  Star(Regex* rgx);
  Star(RgxPtr rgx);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  void toStream(std::ostream& out) const override;

private:
  RgxPtr rgx_;
};

// class Not : public Regex {
// public:
//   Not(Regex* rgx);
//   // ~Not() { delete rgx_; }
//   bool isNullable() const override;
//   Regex* getDeriv(char c) const override;
//   void toStream(std::ostream& out) const override;

// private:
//   Regex* rgx_;
// };

#endif
