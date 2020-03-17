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
// TODO: Switch to Regex* and define copy and move ctors
using RgxPtr = std::shared_ptr<Regex>;

class Regex {
public:
  virtual ~Regex(){};
  virtual void toStream(std::ostream& out) const = 0;
  virtual RgxPtr getDeriv(char) const = 0;
  virtual bool isNullable() const = 0;

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

class Alt : public Regex {
public:
  Alt(std::vector<RgxPtr>&& rgxs);
  // ~Alt() {
  //   std::for_each(rgxs_.cbegin(), rgxs_.cend(), [](const RgxPtr rPtr) { delete rPtr; });
  // }
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  void toStream(std::ostream& out) const override;

private:
  std::vector<RgxPtr> rgxs_;
};

class Concat : public Regex {
public:
  Concat(std::vector<RgxPtr>&& rgxs);
  // ~Concat() {
  //   std::for_each(rgxs_.cbegin(), rgxs_.cend(), [](const RgxPtr rPtr) { delete rPtr; });
  // }
  bool isNullable() const override;
  // TODO: Make this better
  RgxPtr getDeriv(char c) const override;
  void toStream(std::ostream& out) const override;

private:
  std::vector<RgxPtr> rgxs_;
};

class Star : public Regex {
public:
  Star(RgxPtr rgx);
  // ~Star() { delete rgx_; }
  bool isNullable() const override;
  // TODO: Sharing rgx_ pointer will cause problems
  RgxPtr getDeriv(char c) const override;
  void toStream(std::ostream& out) const override;

private:
  RgxPtr rgx_;
};

class Not : public Regex {
public:
  Not(RgxPtr rgx);
  // ~Not() { delete rgx_; }
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  void toStream(std::ostream& out) const override;

private:
  RgxPtr rgx_;
};

#endif
