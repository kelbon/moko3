#include "moko3/testbox.hpp"

#include <regex>
#include "zal/zal.hpp"

namespace moko3 {

testbox::testbox() : listener(std::make_unique<test_listener_i>()) {
}

void testbox::register_test(std::string testname, test_fn_t* test) {
  // does not check for unique
  tests.push_back(testinfo{std::move(testname), test});
}

std::chrono::steady_clock::duration testbox::test_timeout(std::string_view testname) {
  return config.timeout * std::chrono::seconds(1);
}

void testbox::register_listener(std::unique_ptr<test_listener_i> l) {
  if (!l)
    listener = std::unique_ptr<test_listener_i>();
  else
    listener = std::move(l);
}

void testbox::parse_config(int argc, char* argv[]) {
  config = moko3::cli::parse_or_exit(argc, argv);
}

int testbox::run_tests() {
  std::regex r(std::string(config.tests_regex));
  int failed = 0;
  listener->on_start();
  on_scope_exit {
    listener->on_end();
  };
  for (const testinfo& i : tests) {
    if (!std::regex_search(i.name, r))
      continue;
    if (config.dry_run)
      continue;
    listener->on_test_start(i);
    top_lvl_section toplevel_section;
    toplevel_section.name = i.name;
    cur_running_test = &toplevel_section;
    toplevel_section.mark_toplevel();
    do {
      toplevel_section.prepare_to_run();
      test_run_info runinfo;
      listener->on_test_case_start(i);
      try {
        runinfo = i.func(&toplevel_section);
      } catch (test_failed& e) {
        e.deactivate();
        runinfo.failreason = "test_failed exception thrown";
        runinfo.failed = true;
      } catch (std::exception& e) {
        runinfo.failreason = e.what();
        runinfo.failed = true;
      } catch (...) {
        runinfo.failreason = "unknown exception";
        runinfo.failed = true;
      }
      runinfo.testname = toplevel_section.runned_case_name();
      listener->on_test_case_end(runinfo);
      if (runinfo.failed) {
        ++failed;
        break;
      }
    } while (toplevel_section.need_run());
    listener->on_test_end(i);
  }
  return failed;
}

testbox& get_testbox() {
  // inner constant to avoid depending on ordering of global constant initialization (tests registering)
  static testbox _box;
  return _box;
}

}  // namespace moko3
