#include "moko3/common.hpp"

#include <iostream>

namespace moko3 {

test_exception::~test_exception() {
  if (*activated) {
    std::cerr << "MOKO3: invalid usage. Test failure exception was catched";
    std::terminate();
  }
}

}  // namespace moko3
