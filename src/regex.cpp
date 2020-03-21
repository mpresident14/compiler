#include "regex.hpp"

using namespace std;
// TODO: Implement derivative optimization (use enum and getType())
//   (see https://github.com/hmc-cs132-spring2020/hw1-derivs-mpresident14/blob/develop/Deriv.hs)

bool EmptySet::isNullable() const { return false; }
RgxPtr EmptySet::getDeriv(char) const { return make_shared<EmptySet>(); }
bool EmptySet::operator==(const Regex &other) const { return other.getType() == RgxType::EMPTYSET; }
RgxType EmptySet::getType() const { return RgxType::EMPTYSET; }
void EmptySet::toStream(ostream &out) const { out << "EMPTYSET"; }

bool Epsilon::isNullable() const { return true; }
RgxPtr Epsilon::getDeriv(char) const { return make_shared<EmptySet>(); }
bool Epsilon::operator==(const Regex &other) const { return other.getType() == RgxType::EPSILON; }
RgxType Epsilon::getType() const { return RgxType::EPSILON; }
void Epsilon::toStream(ostream &out) const { out << "EPSILON"; }


/*************
 * Character *
 *************/
Character::Character(char c) : c_{ c } {}

bool Character::isNullable() const { return false; }

RgxPtr Character::getDeriv(char c) const {
  if (c == c_) {
    return make_shared<Epsilon>();
  }
  return make_shared<EmptySet>();
}

RgxType Character::getType() const { return RgxType::CHARACTER; }

bool Character::operator==(const Regex &other) const {
  return other.getType() == RgxType::CHARACTER && static_cast<const Character &>(other).c_ == c_;
}

void Character::toStream(ostream &out) const { out << "CHAR " << c_; }


/***************
 * RegexVector *
 ***************/
RegexVector::RegexVector(Regex *r1, Regex *r2) : rgxs_{ RgxPtr(r1), RgxPtr(r2) } {}

RegexVector::RegexVector(RegexVector *rVec, Regex *r) : rgxs_{ move(rVec->rgxs_) } {
  rgxs_.push_back(RgxPtr(r));
}

bool RegexVector::operator==(const RegexVector &other) const {
  return equal(rgxs_.cbegin(),
      rgxs_.cend(),
      other.rgxs_.cbegin(),
      [](const RgxPtr &rPtr1, const RgxPtr &rPtr2) { return *rPtr1 == *rPtr2; });
}

RegexVector::RegexVector(vector<RgxPtr> &&vec) : rgxs_{ move(vec) } {}


/*******
 * Alt *
 *******/
Alt::Alt(RegexVector *rVec) : rVec_(rVec) {}

Alt::~Alt() { delete rVec_; }

bool Alt::isNullable() const {
  return any_of(rVec_->rgxs_.cbegin(), rVec_->rgxs_.cend(), [](const RgxPtr rPtr) {
    return rPtr->isNullable();
  });
}

RgxPtr Alt::getDeriv(char c) const {
  vector<RgxPtr> derivs;
  transform(
      rVec_->rgxs_.cbegin(), rVec_->rgxs_.cend(), back_inserter(derivs), [c](const RgxPtr rPtr) {
        return rPtr->getDeriv(c);
      });
  return make_shared<Alt>(new RegexVector(move(derivs)));
}

RgxType Alt::getType() const { return RgxType::ALT; }

bool Alt::operator==(const Regex &other) const {
  return other.getType() == RgxType::ALT && *static_cast<const Alt &>(other).rVec_ == *rVec_;
}

void Alt::toStream(ostream &out) const { out << "ALT " << rVec_->rgxs_; }


/**********
 * Concat *
 **********/

Concat::Concat(RegexVector *rVec) : rVec_(rVec) {}

Concat::~Concat() { delete rVec_; }

bool Concat::isNullable() const {
  return all_of(rVec_->rgxs_.cbegin(), rVec_->rgxs_.cend(), [](const RgxPtr rPtr) {
    return rPtr->isNullable();
  });
}

// TODO: Make this better
RgxPtr Concat::getDeriv(char c) const {
  vector<RgxPtr> &rgxs = rVec_->rgxs_;
  vector<RgxPtr> derivAndRest = { rgxs[0]->getDeriv(c) };
  copy(rgxs.cbegin() + 1, rgxs.cend(), back_inserter(derivAndRest));

  if (rgxs[0]->isNullable()) {
    vector<RgxPtr> rest(rgxs.cbegin() + 1, rgxs.cend());
    return make_shared<Alt>(new RegexVector({ Concat(new RegexVector(move(rest))).getDeriv(c),
        make_shared<Concat>(new RegexVector(move(derivAndRest))) }));
  }

  return make_shared<Concat>(new RegexVector(move(derivAndRest)));
}

RgxType Concat::getType() const { return RgxType::CONCAT; }

bool Concat::operator==(const Regex &other) const {
  return other.getType() == RgxType::CONCAT && *static_cast<const Concat &>(other).rVec_ == *rVec_;
}

void Concat::toStream(ostream &out) const { out << "CONCAT " << rVec_->rgxs_; }


/********
 * Star *
 ********/
Star::Star(Regex *rgx) : rgx_(RgxPtr(rgx)) {}
Star::Star(RgxPtr rgx) : rgx_(rgx) {}

bool Star::isNullable() const { return true; }
RgxPtr Star::getDeriv(char c) const {
  return make_shared<Concat>(
      new RegexVector(vector<RgxPtr>{ rgx_->getDeriv(c), make_shared<Star>(rgx_) }));
}

RgxType Star::getType() const { return RgxType::STAR; }

bool Star::operator==(const Regex &other) const {
  return other.getType() == RgxType::STAR && *static_cast<const Star &>(other).rgx_ == *rgx_;
}

void Star::toStream(ostream &out) const { out << "STAR (" << rgx_ << ')'; }


/*******
 * Not *
 *******/
Not::Not(Regex *rgx) : rgx_(RgxPtr(rgx)) {}
Not::Not(RgxPtr rgx) : rgx_(rgx) {}
bool Not::isNullable() const { return !rgx_->isNullable(); }
RgxPtr Not::getDeriv(char c) const { return make_shared<Not>(rgx_->getDeriv(c)); }
RgxType Not::getType() const { return RgxType::NOT; }

bool Not::operator==(const Regex &other) const {
  return other.getType() == RgxType::NOT && *static_cast<const Not &>(other).rgx_ == *rgx_;
}

void Not::toStream(ostream &out) const { out << "NOT (" << rgx_ << ')'; }


/*********
 * Range *
 *********/
Range::Range(char start, char end) : start_(start), end_(end) {}
bool Range::isNullable() const { return false; }
RgxPtr Range::getDeriv(char c) const {
  if (start_ < c && c < end_) {
    return make_shared<Epsilon>();
  }
  return make_shared<EmptySet>();
}
RgxType Range::getType() const { return RgxType::RANGE; }

bool Range::operator==(const Regex &other) const {
  return other.getType() == RgxType::RANGE && static_cast<const Range &>(other).start_ == start_ &&
         static_cast<const Range &>(other).end_ == end_;
}

void Range::toStream(std::ostream &out) const { out << "RANGE (" << start_ << "-" << end_ << ')'; }
