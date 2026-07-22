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
  bool registered = false;  // cache `register_section`
  bool toplevel = false;
  size_t generate_index = 0;
  size_t max_generation_index = size_t(-1);  //
  int generate_line = -1;                    // line where GENERATE called

  [[nodiscard]] bool has_generate() const noexcept {
    return generate_line != -1;
  }

  void mark_not_entered() noexcept {
    assert(was_entered);
    was_entered = false;
    if (has_generate()) {
      assert(!has_more_generate_values());
      generate_index = 0;
    }
    for (section_info* i : registered_sections)
      i->mark_not_entered();
  }

  [[nodiscard]] bool has_more_generate_values() const noexcept {
    return has_generate() && generate_index < max_generation_index;
  }
  // used by GENERATE macro on each invocation (even first)
  template <typename R>
  [[nodiscard]] std::ranges::range_value_t<R> next_generated_value(R values, int line) noexcept {
    static_assert(std::ranges::random_access_range<R> && std::ranges::sized_range<R>);
    // ignore exceptions here
    if (has_generate() && line != generate_line) {
      std::cerr << "!!!GENERATE MUST BE CALLED ONLY ONCE!!! abort called in test " << name << std::endl;
      std::abort();
    }
    if (!has_generate()) {
      generate_line = line;
      assert(std::ranges::size(values) > 0);
      max_generation_index = std::ranges::size(values) - 1;
    }
    return std::move(values[generate_index]);
  }

  // resets *this and inner sections to be ready to run again
  // but dont touch `registered` etc
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

}  // namespace moko3
