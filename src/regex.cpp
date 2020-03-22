#include "regex.hpp"

#include <algorithm>
#include <numeric>
#include <functional>

using namespace std;


/***************************
 * Derivative Optimization *
 ***************************/

RgxPtr makeConcat(RgxPtr r1, RgxPtr r2) {
  RgxType r1Type = r1->getType();
  RgxType r2Type = r2->getType();
  // ∅ r2 = ∅
  // r1 ∅ = ∅
  if (r1Type == RgxType::EMPTYSET || r2Type == RgxType::EMPTYSET) {
    return make_shared<EmptySet>();
  }
  // ε r2 = r2
  if (r1Type == RgxType::EPSILON) {
    return r2;
  }
  // r1 ε = r1
  if (r2Type == RgxType::EPSILON) {
    return r1;
  }

  if (r1Type == RgxType::CONCAT) {
    vector<RgxPtr> &r1Vec = static_cast<Concat *>(r1.get())->rVec_.rgxs_;
    vector<RgxPtr> newVec(r1Vec.cbegin(), r1Vec.cend());
    // [r1s] [r2s] = [r1s + r2s]
    if (r2Type == RgxType::CONCAT) {
      vector<RgxPtr> &r2Vec = static_cast<Concat *>(r2.get())->rVec_.rgxs_;
      copy(r2Vec.cbegin(), r2Vec.cend(), back_inserter(newVec));
      return make_shared<Concat>(RegexVector(move(newVec)));
    }
    // [r1s] r2 = [r1s + r2]
    newVec.push_back(r2);
    return make_shared<Concat>(RegexVector(move(newVec)));
  }
  // r1 [r2s] = [r1 + r2s]
  if (r2Type == RgxType::CONCAT) {
    vector<RgxPtr> newVec = { r1 };
    vector<RgxPtr> &r2Vec = static_cast<Concat *>(r2.get())->rVec_.rgxs_;
    copy(r2Vec.cbegin(), r2Vec.cend(), back_inserter(newVec));
    return make_shared<Concat>(RegexVector(move(newVec)));
  }

  return make_shared<Concat>(RegexVector({ r1, r2 }));
}


RgxPtr makeAlt(RgxPtr r1, RgxPtr r2) {
  RgxType r1Type = r1->getType();
  RgxType r2Type = r2->getType();
  // ^∅ | r2 = ^∅
  // r1 | ^∅ = ^∅
  if ((r1Type == RgxType::NOT &&
          static_cast<Not *>(r1.get())->rgx_->getType() == RgxType::EMPTYSET) ||
      (r2Type == RgxType::NOT &&
          static_cast<Not *>(r2.get())->rgx_->getType() == RgxType::EMPTYSET)) {
    return make_shared<Not>(new EmptySet);
  }
  // ∅ | r2 = r2
  if (r1Type == RgxType::EMPTYSET) {
    return r2;
  }
  // r1 | ∅ = r1
  if (r2Type == RgxType::EMPTYSET) {
    return r1;
  }

  if (r1Type == RgxType::ALT) {
    vector<RgxPtr> &r1Vec = static_cast<Alt *>(r1.get())->rVec_.rgxs_;
    vector<RgxPtr> newVec(r1Vec.cbegin(), r1Vec.cend());
    // Alt [r1s] | Alt [r2s] = Alt [r1s + r2s]
    if (r2Type == RgxType::ALT) {
      vector<RgxPtr> &r2Vec = static_cast<Alt *>(r2.get())->rVec_.rgxs_;
      copy(r2Vec.cbegin(), r2Vec.cend(), back_inserter(newVec));
      return make_shared<Alt>(RegexVector(move(newVec)));
    }
    // Alt [r1s] | r2 = Alt [r1s + r2]
    newVec.push_back(r2);
    return make_shared<Alt>(RegexVector(move(newVec)));
  }
  // r1 | Alt [r2s] = Alt [r1 + r2s]
  if (r2Type == RgxType::ALT) {
    vector<RgxPtr> newVec = { r1 };
    vector<RgxPtr> &r2Vec = static_cast<Alt *>(r2.get())->rVec_.rgxs_;
    copy(r2Vec.cbegin(), r2Vec.cend(), back_inserter(newVec));
    return make_shared<Alt>(RegexVector(move(newVec)));
  }

  // r | r = r
  if (r1 == r2) {
    return r1;
  }

  return make_shared<Alt>(RegexVector({ r1, r2 }));
}


RgxPtr makeConcats(vector<RgxPtr> &&rs) {
  return accumulate(rs.cbegin() + 1, rs.cend(), rs[0], makeConcat);
}

RgxPtr makeAlts(vector<RgxPtr> &&rs) {
  return accumulate(rs.cbegin() + 1, rs.cend(), rs[0], makeAlt);
}

RgxPtr makeStar(RgxPtr r) {
  switch (r->getType()) {
    case RgxType::STAR:
      return r;
    case RgxType::EPSILON:
      return r;
    case RgxType::EMPTYSET:
      return make_shared<Epsilon>();
    default:
      return make_shared<Star>(r);
  }
}

RgxPtr makeNot(RgxPtr r) {
  if (r->getType() == RgxType::NOT) {
    return static_cast<Not *>(r.get())->rgx_;
  }
  return make_shared<Not>(r);
}


bool EmptySet::isNullable() const { return false; }
RgxPtr EmptySet::getDeriv(char) const { return make_shared<EmptySet>(); }
bool EmptySet::operator==(const Regex &other) const { return other.getType() == RgxType::EMPTYSET; }
RgxType EmptySet::getType() const { return RgxType::EMPTYSET; }
size_t EmptySet::hashFn() const { return 0; }
void EmptySet::toStream(ostream &out) const { out << "EMPTYSET"; }

bool Epsilon::isNullable() const { return true; }
RgxPtr Epsilon::getDeriv(char) const { return make_shared<EmptySet>(); }
bool Epsilon::operator==(const Regex &other) const { return other.getType() == RgxType::EPSILON; }
RgxType Epsilon::getType() const { return RgxType::EPSILON; }
size_t Epsilon::hashFn() const { return 0; }
void Epsilon::toStream(ostream &out) const { out << "EPSILON"; }


/*************
 * Character *
 *************/
Character::Character(char c) : c_{ c } {}

bool Character::isNullable() const { return false; }

RgxPtr Character::getDeriv(char c) const {
  if (c_ == '.' || c == c_) {
    return make_shared<Epsilon>();
  }
  return make_shared<EmptySet>();
}

RgxType Character::getType() const { return RgxType::CHARACTER; }

bool Character::operator==(const Regex &other) const {
  return other.getType() == RgxType::CHARACTER && static_cast<const Character &>(other).c_ == c_;
}

size_t Character::hashFn() const { return hash<char>()(c_); }

void Character::toStream(ostream &out) const { out << "CHAR " << c_; }


/***************
 * RegexVector *
 ***************/
RegexVector::RegexVector(Regex *r1, Regex *r2) : rgxs_{ RgxPtr(r1), RgxPtr(r2) } {}

RegexVector::RegexVector(RegexVector &&rVec, Regex *r) : rgxs_{ move(rVec.rgxs_) } {
  rgxs_.push_back(RgxPtr(r));
}

bool RegexVector::operator==(const RegexVector &other) const {
  return equal(rgxs_.cbegin(),
      rgxs_.cend(),
      other.rgxs_.cbegin(),
      [](const RgxPtr &rPtr1, const RgxPtr &rPtr2) { return *rPtr1 == *rPtr2; });
}

RegexVector::RegexVector(vector<RgxPtr> &&vec) : rgxs_{ move(vec) } {}

size_t RegexVector::hashFn() const {
  hash<RgxPtr> hasher;
  return accumulate(rgxs_.cbegin() + 1,
      rgxs_.cend(),
      hasher(rgxs_[0]),
      [&hasher](size_t hashSum, const RgxPtr &r2) { return hashSum ^ (hasher(r2) << 1); });
}

/*******
 * Alt *
 *******/
Alt::Alt(RegexVector &&rVec) : rVec_(move(rVec)) {}

bool Alt::isNullable() const {
  return any_of(rVec_.rgxs_.cbegin(), rVec_.rgxs_.cend(), [](const RgxPtr rPtr) {
    return rPtr->isNullable();
  });
}

RgxPtr Alt::getDeriv(char c) const {
  vector<RgxPtr> derivs;
  transform(
      rVec_.rgxs_.cbegin(), rVec_.rgxs_.cend(), back_inserter(derivs), [c](const RgxPtr rPtr) {
        return rPtr->getDeriv(c);
      });
  return makeAlts(move(derivs));
}

RgxType Alt::getType() const { return RgxType::ALT; }

bool Alt::operator==(const Regex &other) const {
  return other.getType() == RgxType::ALT && static_cast<const Alt &>(other).rVec_ == rVec_;
}

size_t Alt::hashFn() const { return rVec_.hashFn(); }

void Alt::toStream(ostream &out) const { out << "ALT " << rVec_.rgxs_; }


/**********
 * Concat *
 **********/

Concat::Concat(RegexVector &&rVec) : rVec_(move(rVec)) {}

bool Concat::isNullable() const {
  return all_of(rVec_.rgxs_.cbegin(), rVec_.rgxs_.cend(), [](const RgxPtr rPtr) {
    return rPtr->isNullable();
  });
}

// TODO: Make this better
RgxPtr Concat::getDeriv(char c) const {
  const vector<RgxPtr> &rgxs = rVec_.rgxs_;
  vector<RgxPtr> derivAndRest = { rgxs[0]->getDeriv(c) };
  copy(rgxs.cbegin() + 1, rgxs.cend(), back_inserter(derivAndRest));

  if (rgxs[0]->isNullable()) {
    vector<RgxPtr> rest(rgxs.cbegin() + 1, rgxs.cend());
    return makeAlt(Concat(RegexVector(move(rest))).getDeriv(c), makeConcats(move(derivAndRest)));
  }

  return makeConcats(move(derivAndRest));
}

RgxType Concat::getType() const { return RgxType::CONCAT; }

bool Concat::operator==(const Regex &other) const {
  return other.getType() == RgxType::CONCAT && static_cast<const Concat &>(other).rVec_ == rVec_;
}

size_t Concat::hashFn() const { return rVec_.hashFn(); }

void Concat::toStream(ostream &out) const { out << "CONCAT " << rVec_.rgxs_; }


/********
 * Star *
 ********/
Star::Star(Regex *rgx) : rgx_(RgxPtr(rgx)) {}
Star::Star(RgxPtr rgx) : rgx_(rgx) {}

bool Star::isNullable() const { return true; }
RgxPtr Star::getDeriv(char c) const { return makeConcat(rgx_->getDeriv(c), makeStar(rgx_)); }

RgxType Star::getType() const { return RgxType::STAR; }

bool Star::operator==(const Regex &other) const {
  return other.getType() == RgxType::STAR && *static_cast<const Star &>(other).rgx_ == *rgx_;
}

size_t Star::hashFn() const { return rgx_->hashFn(); }

void Star::toStream(ostream &out) const { out << "STAR (" << rgx_ << ')'; }


/*******
 * Not *
 *******/
Not::Not(Regex *rgx) : rgx_(RgxPtr(rgx)) {}
Not::Not(RgxPtr rgx) : rgx_(rgx) {}
bool Not::isNullable() const { return !rgx_->isNullable(); }
RgxPtr Not::getDeriv(char c) const { return makeNot(rgx_->getDeriv(c)); }
RgxType Not::getType() const { return RgxType::NOT; }

bool Not::operator==(const Regex &other) const {
  return other.getType() == RgxType::NOT && *static_cast<const Not &>(other).rgx_ == *rgx_;
}

size_t Not::hashFn() const { return rgx_->hashFn(); }

void Not::toStream(ostream &out) const { out << "NOT (" << rgx_ << ')'; }


/*********
 * Range *
 *********/
Range::Range(char start, char end) : start_(start), end_(end) {}
bool Range::isNullable() const { return false; }
RgxPtr Range::getDeriv(char c) const {
  if (start_ <= c && c <= end_) {
    return make_shared<Epsilon>();
  }
  return make_shared<EmptySet>();
}
RgxType Range::getType() const { return RgxType::RANGE; }

bool Range::operator==(const Regex &other) const {
  return other.getType() == RgxType::RANGE && static_cast<const Range &>(other).start_ == start_ &&
         static_cast<const Range &>(other).end_ == end_;
}

size_t Range::hashFn() const {
  hash<char> hasher;
  return hasher(start_) ^ (hasher(end_) << 1);
}

void Range::toStream(std::ostream &out) const { out << "RANGE (" << start_ << "-" << end_ << ')'; }
