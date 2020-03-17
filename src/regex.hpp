#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include <prez/print_stuff.hpp>

class Regex;
using RgxPtr = Regex*;

class Regex {
public:
  virtual ~Regex(){};
  virtual void toStream(std::ostream& out) = 0;
  virtual RgxPtr getDeriv() = 0;
  virtual bool isNullable() = 0;

  friend std::ostream& operator<<(std::ostream& out, RgxPtr rgx) {
    rgx->toStream(out);
    return out;
  }
};

class EmptySet : public Regex {
public:
  EmptySet() {}
  void toStream(std::ostream& out) override { out << "EMPTYSET"; }
};

class Epsilon : public Regex {
public:
  Epsilon() {}
  void toStream(std::ostream& out) override { out << "EPSILON"; }
};

class Character : public Regex {
public:
  Character(char c) : c_{c} {}
  void toStream(std::ostream& out) override { out << "CHAR " << c_; }

private:
  char c_;
};

class Concat : public Regex {
public:
  Concat(std::vector<RgxPtr>&& rgxs) : rgxs_(std::move(rgxs)) {}
  ~Concat() {
    std::for_each(rgxs_.cbegin(), rgxs_.cend(), [](const RgxPtr rPtr) { delete rPtr; });
  }
  void toStream(std::ostream& out) override { out << "CONCAT " << rgxs_; }

private:
  std::vector<RgxPtr> rgxs_;
};

class Alt : public Regex {
public:
  Alt(std::vector<RgxPtr>&& rgxs) : rgxs_(std::move(rgxs)) {}
  ~Alt() {
    std::for_each(rgxs_.cbegin(), rgxs_.cend(), [](const RgxPtr rPtr) { delete rPtr; });
  }
  void toStream(std::ostream& out) override { out << "ALT " << rgxs_; }

private:
  std::vector<RgxPtr> rgxs_;
};

class Star : public Regex {
public:
  Star(RgxPtr rgx) : rgx_{std::move(rgx)} {}
  ~Star() { delete rgx_; }
  void toStream(std::ostream& out) override { out << "STAR (" << rgx_ << ")"; }

private:
  RgxPtr rgx_;
};
