#include "regex.hpp"

using namespace std;
// TODO: Implement derivative optimization (use enum and getType())
//   (see https://github.com/hmc-cs132-spring2020/hw1-derivs-mpresident14/blob/develop/Deriv.hs)

bool EmptySet::isNullable() const { return false; }
Regex* EmptySet::getDeriv(char) const { return new EmptySet(); }
void EmptySet::toStream(ostream& out) const { out << "EMPTYSET"; }

bool Epsilon::isNullable() const { return true; }
Regex* Epsilon::getDeriv(char) const { return new EmptySet(); }
void Epsilon::toStream(ostream& out) const { out << "EPSILON"; }


/*************
 * Character *
 *************/
Character::Character(char c) : c_{c} {}

bool Character::isNullable() const { return false; }

Regex* Character::getDeriv(char c) const {
  if (c == c_) {
    return new Epsilon();
  }
  return new EmptySet();
}

void Character::toStream(ostream& out) const { out << "CHAR " << c_; }


/***************
 * RegexVector *
 ***************/
RegexVector::RegexVector(Regex* r1, Regex* r2) : rgxs_{r1, r2} {}

RegexVector::RegexVector(RegexVector* rVec, Regex* r)
    : rgxs_{move(rVec->rgxs_)}
{
  rgxs_.push_back(r);
}

RegexVector::RegexVector(vector<Regex*>&& vec) : rgxs_{move(vec)} {}

RegexVector::~RegexVector() {
  for_each(rgxs_.cbegin(), rgxs_.cend(), [](const Regex* rPtr) { delete rPtr; });
}


/*******
 * Alt *
 *******/
Alt::Alt(RegexVector* rVec) : rVec_(rVec) {}

Alt::~Alt() { delete rVec_; }

bool Alt::isNullable() const {
  return any_of(
      rVec_->rgxs_.cbegin(), rVec_->rgxs_.cend(), [](const Regex* rPtr) { return rPtr->isNullable(); });
}

Regex* Alt::getDeriv(char c) const {
  vector<Regex*> derivs;
  transform(rVec_->rgxs_.cbegin(), rVec_->rgxs_.cend(), back_inserter(derivs), [c](const Regex* rPtr) {
    return rPtr->getDeriv(c);
  });
  return new Alt(new RegexVector(move(derivs)));
}

void Alt::toStream(ostream& out) const { out << "ALT " << rVec_->rgxs_; }


/**********
 * Concat *
 **********/

// Concat::Concat(RegexVector* rVec) : rVec_(rVec) {}

// Concat::~Concat() { delete rVec_; }

// bool Concat::isNullable() const {
//   return all_of(
//       rVec_->rgxs_.cbegin(), rVec_->rgxs_.cend(), [](const Regex* rPtr) { return rPtr->isNullable(); });
// }

// // TODO: Make this better
// Regex* Concat::getDeriv(char c) const {
//   vector<Regex*> derivAndRest = {rgxs_[0]->getDeriv(c)};
//   copy(rgxs_.cbegin() + 1, rgxs_.cend(), back_inserter(derivAndRest));

//   if (rgxs_[0]->isNullable()) {
//     vector<Regex*> rest1(rgxs_.cbegin() + 1, rgxs_.cend());
//     vector<Regex*> rest2 = rest1;
//     return make_shared<Alt>(Alt({Concat(move(rest1)).getDeriv(c),
//         make_shared<Concat>(Concat(move(derivAndRest)))}));
//   }

//   return make_shared<Concat>(Concat(move(derivAndRest)));
// }
// void Concat::toStream(ostream& out) const { out << "CONCAT " << rVec->rgxs_; }

// Star::Star(Regex* rgx) : rgx_{move(rgx)} {}
// // ~Star() { delete rgx_; }
// bool Star::isNullable() const { return true; }
// // TODO: Sharing rgx_ pointer will cause problems
// Regex* Star::getDeriv(char c) const {
//   return make_shared<Concat>(
//       Concat(vector<Regex*>{rgx_->getDeriv(c), make_shared<Star>(Star(rgx_))}));
// }
// void Star::toStream(ostream& out) const { out << "STAR (" << rgx_ << ")"; }

// Not::Not(Regex* rgx) : rgx_{move(rgx)} {}
// // ~Not() { delete rgx_; }
// bool Not::isNullable() const { return !rgx_->isNullable(); }
// Regex* Not::getDeriv(char c) const { return make_shared<Not>(Not(rgx_->getDeriv(c))); }
// void Not::toStream(ostream& out) const { out << "NOT (" << rgx_ << ")"; }
