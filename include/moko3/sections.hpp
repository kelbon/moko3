#pragma once

#include <vector>
#include <string>
#include <cassert>

#include <iostream>

namespace moko3 {

struct section_info {
  std::string name;
  // inner section used in this run
  section_info* current_run_section = nullptr;
  std::vector<section_info*> registered_sections;
  bool was_entered = false;
  bool registered = false;                     // cache `register_section`
  size_t generation_index = size_t(-1);        // for GENERATE
  size_t generation_index_bound = size_t(-1);  //

  // allowed only on top level section
  void next_generated_value(auto&&...) = delete;

  // used when run for next generated value required
  // resets *this and inner sections to be ready to run again
  // but dont touch `registered` etc
  // makes sense only for not top-level section
  void reuse();

  void reuse_inners() {
    for (section_info* s : registered_sections)
      s->reuse();
  }

  void register_section(section_info* s) noexcept;

  // returns false if not entered
  [[nodiscard]] bool enter_section(section_info* s) noexcept;
  // should be called only after successful `enter_section`
  void leave_section(section_info* s) noexcept;

  // called before each execution
  void prepare_to_run() noexcept;
  // must be used only for top-level section before start
  void mark_toplevel() noexcept;
  // must be called only after first run
  // returns true if its required to run test with this section again
  bool need_run() noexcept;

  // returns name -> section -> section etc of last runned case
  [[nodiscard]] std::string runned_case_name() const;
};

// separate type for detecting compile errors, used in implementation
struct top_lvl_section : section_info {
  bool generate_called = false;
  // used by GENERATE macro on each invocation (even first)
  template <typename R>
  [[nodiscard]] std::ranges::range_value_t<R> next_generated_value(R values) noexcept {
    static_assert(std::ranges::random_access_range<R> && std::ranges::sized_range<R>);
    // ignore exceptions here
    if (generate_called) {
      std::cerr << "!!!GENERATE MUST BE CALLED ONLY ONCE!!! terminate called in test " << name << std::endl;
      std::abort();
    }
    assert(std::ranges::size(values) > 0);
    if (generation_index == size_t(-1)) {
      generation_index = 0;
      generation_index_bound = std::ranges::size(values);
    }
    return std::move(values[generation_index]);
  }
};

}  // namespace moko3
