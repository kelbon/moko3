#pragma once

#include <chrono>

#include "moko3/common.hpp"
#include "moko3/sections.hpp"

#define program_options_file "moko3/moko3_cli.def"
#define CLINOK_NAMESPACE_NAME moko3::cli
#include <clinok/cli_interface.hpp>

namespace moko3 {

using test_fn_t = test_run_info(top_lvl_section* /*top level section*/);

struct testinfo {
  std::string name;
  test_fn_t* func;
};

// interface
struct test_listener_i {
  // called once when test running starts
  virtual void on_start() noexcept {
  }
  // called once when test running ends
  virtual void on_end() noexcept {
  }
  // called when test itself started, before first case (section, generation etc) in test
  virtual void on_test_start(testinfo const&) noexcept {
  }
  virtual void on_test_end(testinfo const&) noexcept {
  }

  // called before each case, e.g. may be called many times for one
  // receives test info, but not test case info
  virtual void on_test_case_start(testinfo const&) noexcept {
  }
  virtual void on_test_case_end(const test_run_info&) noexcept {
  }

  virtual ~test_listener_i() {
  }
};

// prints in google test style
struct gtest_listener : test_listener_i {
 private:
  size_t runned_tests = 0;
  size_t runned_cases = 0;
  std::vector<std::string> failed;

 public:
  void on_start() noexcept override;
  void on_end() noexcept override;
  void on_test_start(testinfo const&) noexcept override;
  void on_test_end(testinfo const&) noexcept override;

  void on_test_case_start(testinfo const&) noexcept override;
  void on_test_case_end(const test_run_info& t) noexcept override;
};

struct testbox {
 private:
  std::vector<testinfo> tests;
  std::chrono::steady_clock::duration default_test_timeout = std::chrono::seconds(30);
  std::unique_ptr<test_listener_i> listener;
  moko3::cli::options config;
  section_info* cur_running_test = nullptr;

 public:
  std::ostream& out = std::cout;

  testbox();

  void register_test(std::string testname, test_fn_t* test);
  void register_listener(std::unique_ptr<test_listener_i>);

  std::chrono::steady_clock::duration test_timeout(std::string_view testname);

  test_listener_i& get_listener() const noexcept {
    return *listener;  // invariant: != nullptr
  }
  const moko3::cli::options get_config() const noexcept {
    return config;
  }
  void parse_config(int argc, char* argv[]);

  // returns coun of failed tests
  int run_tests();
};

testbox& get_testbox();

struct test_registrator {
  test_registrator(std::string name, test_fn_t* test) {
    get_testbox().register_test(std::move(name), test);
  }
  test_registrator(test_registrator&&) = delete;
  void operator=(test_registrator&&) = delete;
};

struct test_listener_registrator {
  test_listener_registrator(std::unique_ptr<test_listener_i> l) noexcept {
    get_testbox().register_listener(std::move(l));
  }
};

}  // namespace moko3
