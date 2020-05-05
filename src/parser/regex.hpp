#ifndef REGEX_HPP
#define REGEX_HPP

#include <memory>
#include <ostream>
#include <vector>

#include <prez/print_stuff.hpp>

// TODO: add AND operator

enum class RgxType {
  EMPTYSET,
  EPSILON,
  DOT,
  CHARACTER,
  ALT,
  CONCAT,
  STAR,
  RANGE,
  NOT
};

std::ostream& operator<<(std::ostream& out, RgxType type);

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
  // TODO: Fix hash function
  virtual size_t hashFn() const = 0;

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

namespace std {
  template <>
  struct hash<RgxPtr> {
    size_t operator()(const RgxPtr& rgx) const noexcept {
      return static_cast<int>(rgx->getType()) ^ (rgx->hashFn() << 1);
    }
  };
}  // namespace std

inline bool operator==(const RgxPtr& r1, const RgxPtr& r2) {
  return *r1 == *r2;
}

class EmptySet : public Regex {
public:
  bool isNullable() const override;
  RgxPtr getDeriv(char) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;
  size_t hashFn() const override;
};

class Dot : public Regex {
public:
  bool isNullable() const override;
  RgxPtr getDeriv(char) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;
  size_t hashFn() const override;
};

class Epsilon : public Regex {
public:
  bool isNullable() const override;
  RgxPtr getDeriv(char) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;
  size_t hashFn() const override;
};

class Character : public Regex {
public:
  Character(char c);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;
  size_t hashFn() const override;

private:
  char c_;
};

struct RegexVector {
public:
  RegexVector(Regex* r1, Regex* r2);
  RegexVector(RegexVector&& rVec, Regex* r);
  RegexVector(std::vector<RgxPtr>&& vec);
  bool operator==(const RegexVector& other) const;
  size_t hashFn() const;

  std::vector<RgxPtr> rgxs_;
};

class Alt : public Regex {
public:
  Alt(RegexVector&& rVec);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  size_t hashFn() const override;
  void toStream(std::ostream& out) const override;

  friend RgxPtr makeAlt(RgxPtr r1, RgxPtr r2);

private:
  RegexVector rVec_;
};

class Concat : public Regex {
public:
  Concat(RegexVector&& rVec);
  bool isNullable() const override;
  // TODO: Make this better
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  size_t hashFn() const override;
  void toStream(std::ostream& out) const override;

  friend RgxPtr makeConcat(RgxPtr r1, RgxPtr r2);

private:
  RegexVector rVec_;
};

class Star : public Regex {
public:
  Star(Regex* rgx);
  Star(RgxPtr rgx);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  size_t hashFn() const override;
  void toStream(std::ostream& out) const override;

private:
  RgxPtr rgx_;
};


class Range : public Regex {
public:
  Range(char start, char end);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  size_t hashFn() const override;
  void toStream(std::ostream& out) const override;

private:
  char start_;
  char end_;
};

/* *
 * Match a single character NOT present in bracketed list
 * (e.g. [^ab...z] or [^a-z])
 * */
class Not : public Regex {
public:
  Not(Regex* rgx);
  Not(RgxPtr rgx);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  size_t hashFn() const override;
  void toStream(std::ostream& out) const override;

  friend RgxPtr makeAlt(RgxPtr r1, RgxPtr r2);

private:
  /* This must be a Character, Range, or Concat (enforced by lexer and parser) */
  RgxPtr rgx_;
};

#endif
