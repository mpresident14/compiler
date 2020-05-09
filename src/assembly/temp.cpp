#include "temp.hpp"


virtual TempType Register::getType() const noexcept override { return TempType::REGISTER; }
virtual TempType Variable::getType() const noexcept override { return TempType::VARIABLE; }
