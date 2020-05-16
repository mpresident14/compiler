#include "src/language/language.hpp"

using namespace std;

namespace language {



im::ExprPtr ConstInt::toImExpr() const {
  return make_unique<im::Const>(n_);
}


im::ExprPtr ConstBool::toImExpr() const {
  return make_unique<im::Const>(b_);
}


}
