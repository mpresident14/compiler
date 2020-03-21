#include "regex.hpp"

using namespace std;
// TODO: Implement derivative optimization (use enum and getType())
//   (see https://github.com/hmc-cs132-spring2020/hw1-derivs-mpresident14/blob/develop/Deriv.hs)

bool EmptySet::isNullable() const { return false; }
RgxPtr EmptySet::getDeriv(char) const { return make_shared<EmptySet>(); }
void EmptySet::toStream(ostream &out) const { out << "EMPTYSET"; }

bool Epsilon::isNullable() const { return true; }
RgxPtr Epsilon::getDeriv(char) const { return make_shared<EmptySet>(); }
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

void Character::toStream(ostream &out) const { out << "CHAR " << c_; }


/***************
 * RegexVector *
 ***************/
RegexVector::RegexVector(Regex *r1, Regex *r2) : rgxs_{ RgxPtr(r1), RgxPtr(r2) } {}

RegexVector::RegexVector(RegexVector *rVec, Regex *r) : rgxs_{ move(rVec->rgxs_) } {
  rgxs_.push_back(RgxPtr(r));
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
void Concat::toStream(ostream &out) const { out << "CONCAT " << rVec_->rgxs_; }


/********
 * Star *
 ********/
Star::Star(Regex* rgx) : rgx_(RgxPtr(rgx)) {}
Star::Star(RgxPtr rgx) : rgx_(rgx) {}

bool Star::isNullable() const { return true; }
RgxPtr Star::getDeriv(char c) const {
  return make_shared<Concat>(new RegexVector(
      vector<RgxPtr>{rgx_->getDeriv(c), make_shared<Star>(rgx_)}));
}
void Star::toStream(ostream& out) const { out << "STAR (" << rgx_ << ')'; }


/*******
 * Not *
 *******/
Not::Not(Regex* rgx) : rgx_(RgxPtr(rgx)) {}
Not::Not(RgxPtr rgx) : rgx_(rgx) {}
bool Not::isNullable() const { return !rgx_->isNullable(); }
RgxPtr Not::getDeriv(char c) const { return make_shared<Not>(rgx_->getDeriv(c)); }
void Not::toStream(ostream& out) const { out << "NOT (" << rgx_ << ')'; }


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
void Range::toStream(std::ostream& out) const {
  out << "RANGE (" << start_ << "-" << end_ << ')';
}
