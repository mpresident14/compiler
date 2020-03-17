#include "regex.hpp"

using namespace std;
// TODO: Implement derivative optimization (use enum and getType())
//   (see https://github.com/hmc-cs132-spring2020/hw1-derivs-mpresident14/blob/develop/Deriv.hs)

bool EmptySet::isNullable() const { return false; }
RgxPtr EmptySet::getDeriv(char) const { return std::make_shared<EmptySet>(EmptySet()); }
void EmptySet::toStream(std::ostream& out) const { out << "EMPTYSET"; }


bool Epsilon::isNullable() const { return true; }
RgxPtr Epsilon::getDeriv(char) const { return std::make_shared<EmptySet>(EmptySet()); }
void Epsilon::toStream(std::ostream& out) const { out << "EPSILON"; }



Character::Character(char c) : c_{c} {}
bool Character::isNullable() const { return false; }
RgxPtr Character::getDeriv(char c) const {
  if (c == c_) {
    return std::make_shared<Epsilon>(Epsilon());
  }
  return std::make_shared<EmptySet>(EmptySet());
}
void Character::toStream(std::ostream& out) const { out << "CHAR " << c_; }



Alt::Alt(std::vector<RgxPtr>&& rgxs) : rgxs_(std::move(rgxs)) {}
// ~Alt() {
//   std::for_each(rgxs_.cbegin(), rgxs_.cend(), [](const RgxPtr rPtr) { delete rPtr; });
// }
bool Alt::isNullable() const {
  return std::any_of(
      rgxs_.cbegin(), rgxs_.cend(), [](const RgxPtr rPtr) { return rPtr->isNullable(); });
}
RgxPtr Alt::getDeriv(char c) const {
  std::vector<RgxPtr> derivs;
  std::transform(
      rgxs_.cbegin(), rgxs_.cend(), std::back_inserter(derivs), [c](const RgxPtr rPtr) {
        return rPtr->getDeriv(c);
      });
  return std::make_shared<Alt>(Alt(std::move(derivs)));
}
void Alt::toStream(std::ostream& out) const { out << "ALT " << rgxs_; }


Concat::Concat(std::vector<RgxPtr>&& rgxs) : rgxs_(std::move(rgxs)) {}
bool Concat::isNullable() const {
  return std::all_of(
      rgxs_.cbegin(), rgxs_.cend(), [](const RgxPtr rPtr) { return rPtr->isNullable(); });
}
// TODO: Make this better
RgxPtr Concat::getDeriv(char c) const {
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
void Concat::toStream(std::ostream& out) const { out << "CONCAT " << rgxs_; }


Star::Star(RgxPtr rgx) : rgx_{std::move(rgx)} {}
// ~Star() { delete rgx_; }
bool Star::isNullable() const { return true; }
// TODO: Sharing rgx_ pointer will cause problems
RgxPtr Star::getDeriv(char c) const {
  return std::make_shared<Concat>(
      Concat(std::vector<RgxPtr>{rgx_->getDeriv(c), std::make_shared<Star>(Star(rgx_))}));
}
void Star::toStream(std::ostream& out) const { out << "STAR (" << rgx_ << ")"; }


Not::Not(RgxPtr rgx) : rgx_{std::move(rgx)} {}
// ~Not() { delete rgx_; }
bool Not::isNullable() const { return !rgx_->isNullable(); }
RgxPtr Not::getDeriv(char c) const {
  return std::make_shared<Not>(Not(rgx_->getDeriv(c)));
}
void Not::toStream(std::ostream& out) const { out << "NOT (" << rgx_ << ")"; }
