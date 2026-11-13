#include "moko3/common.hpp"

#include "moko3/testbox.hpp"
#include "moko3/require_eq.hpp"

#include <exception>
#include <iostream>
#include <format>

namespace moko3 {

test_exception::~test_exception() {
  // if its last exception copy (in chain for example) and its not deactivated yet
  if (activated.use_count() == 1 && *activated) {
    std::cerr << "moko3: invalid usage. Test failure exception was catched";
    std::terminate();
  }
}

test_failed::test_failed(std::string m, std::source_location l) noexcept : test_exception(std::move(m)) {
  // std::terminate on exceptions here
  get_testbox().out << std::format("moko3:FAIL on {}:{}:{}\nFailure msg: \"{}\"", l.file_name(), l.line(),
                                   l.column(), this->what())
                    << std::endl;
}

void noexport::require_eq_failed(std::string_view macro, std::string_view astr, std::string_view bstr,
                                 std::string a, std::string b, std::source_location l) {
  throw test_failed(std::format("{}({}, {})\n  {} = {}\n  {} = {}", macro, astr, bstr, astr, a, bstr, b), l);
}

}  // namespace moko3
