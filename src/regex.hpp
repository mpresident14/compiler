#include <algorithm>
#include <ostream>
#include <memory>
#include <utility>
#include <vector>

#include <prez/print_stuff.hpp>

// TODO: Move impl to .cpp file
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
  EmptySet() {}
  bool isNullable() const override { return false; }
  RgxPtr getDeriv(char) const override { return std::make_shared<EmptySet>(EmptySet()); }
  void toStream(std::ostream& out) const override { out << "EMPTYSET"; }
};

class Epsilon : public Regex {
public:
  Epsilon() {}
  bool isNullable() const override { return true; }
  RgxPtr getDeriv(char) const override { return std::make_shared<EmptySet>(EmptySet()); }
  void toStream(std::ostream& out) const override { out << "EPSILON"; }
};

class Character : public Regex {
public:
  Character(char c) : c_{c} {}
  bool isNullable() const override { return false; }
  RgxPtr getDeriv(char c) const override {
    if (c == c_) {
      return std::make_shared<Epsilon>(Epsilon());
    }
    return std::make_shared<EmptySet>(EmptySet());
  }
  void toStream(std::ostream& out) const override { out << "CHAR " << c_; }

private:
  char c_;
};


class Alt : public Regex {
public:
  Alt(std::vector<RgxPtr>&& rgxs) : rgxs_(std::move(rgxs)) {}
  // ~Alt() {
  //   std::for_each(rgxs_.cbegin(), rgxs_.cend(), [](const RgxPtr rPtr) { delete rPtr; });
  // }
  bool isNullable() const override {
    return std::any_of(
        rgxs_.cbegin(), rgxs_.cend(), [](const RgxPtr rPtr) { return rPtr->isNullable(); });
  }
  RgxPtr getDeriv(char c) const override {
    std::vector<RgxPtr> derivs;
    std::transform(
        rgxs_.cbegin(), rgxs_.cend(), std::back_inserter(derivs), [c](const RgxPtr rPtr) {
          return rPtr->getDeriv(c);
        });
    return std::make_shared<Alt>(Alt(std::move(derivs)));
  }
  void toStream(std::ostream& out) const override { out << "ALT " << rgxs_; }

private:
  std::vector<RgxPtr> rgxs_;
};

class Concat : public Regex {
public:
  Concat(std::vector<RgxPtr>&& rgxs) : rgxs_(std::move(rgxs)) {}
  // ~Concat() {
  //   std::for_each(rgxs_.cbegin(), rgxs_.cend(), [](const RgxPtr rPtr) { delete rPtr; });
  // }
  bool isNullable() const override {
    return std::all_of(
        rgxs_.cbegin(), rgxs_.cend(), [](const RgxPtr rPtr) { return rPtr->isNullable(); });
  }
  // TODO: Make this better
  RgxPtr getDeriv(char c) const override {
    std::vector<RgxPtr> derivAndRest = { rgxs_[0]->getDeriv(c) };
    std::copy(rgxs_.cbegin() + 1, rgxs_.cend(), std::back_inserter(derivAndRest));

    if (rgxs_[0]->isNullable()) {
      std::vector<RgxPtr> rest1(rgxs_.cbegin() + 1, rgxs_.cend());
      std::vector<RgxPtr> rest2 = rest1;
      return std::make_shared<Alt>(Alt(
          {
            Concat(std::move(rest1)).getDeriv(c),
            std::make_shared<Concat>(Concat(move(derivAndRest)))
          }));
    }

    return std::make_shared<Concat>(Concat(move(derivAndRest)));

  }
  void toStream(std::ostream& out) const override { out << "CONCAT " << rgxs_; }

private:
  std::vector<RgxPtr> rgxs_;
};

class Star : public Regex {
public:
  Star(RgxPtr rgx) : rgx_{std::move(rgx)} {}
  // ~Star() { delete rgx_; }
  bool isNullable() const override { return true; }
  // TODO: Sharing rgx_ pointer will cause problems
  RgxPtr getDeriv(char c) const override {
    return std::make_shared<Concat>(
        Concat(std::vector<RgxPtr>{rgx_->getDeriv(c), std::make_shared<Star>(Star(rgx_))}));
  }
  void toStream(std::ostream& out) const override { out << "STAR (" << rgx_ << ")"; }

private:
  RgxPtr rgx_;
};

class Not : public Regex {
public:
  Not(RgxPtr rgx) : rgx_{std::move(rgx)} {}
  // ~Not() { delete rgx_; }
  bool isNullable() const override { return !rgx_->isNullable(); }
  RgxPtr getDeriv(char c) const override {
    return std::make_shared<Not>(Not(rgx_->getDeriv(c)));
  }
  void toStream(std::ostream& out) const override { out << "NOT (" << rgx_ << ")"; }

private:
  RgxPtr rgx_;
};
