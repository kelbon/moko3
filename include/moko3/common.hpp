#pragma once

#include <memory>
#include <string_view>
#include <string>

namespace moko3 {

struct test_run_info {
  std::string testname = {};
  std::string failreason = {};
  bool failed = false;
};

// not std::exception for reducing changes to catch it
// must not be catched by user
struct test_exception {
 private:
  // invariant: != nullptr
  // shared ptr for case when system copies exception
  std::shared_ptr<bool> activated = std::make_shared<bool>(true);
  std::string msg;

  void deactivate() noexcept {
    *activated = false;
  }
  friend struct testbox;

 public:
  test_exception() = default;
  test_exception(std::string m) noexcept : msg(std::move(m)) {
  }

  // forbid move
  test_exception(test_exception const&) = default;
  virtual ~test_exception();
  virtual std::string_view what() const noexcept {
    return "";
  }
};

struct test_failed : test_exception {
  using test_exception::test_exception;
};

}  // namespace moko3
