#include "moko3/sections.hpp"

#include <algorithm>
#include <format>

namespace moko3 {

void section_info::reuse() {
  was_entered = false;
  current_run_section = nullptr;
  generation_index = size_t(-1);
  generation_index_bound = size_t(-1);
  reuse_inners();
}

void section_info::register_section(section_info* s) noexcept {
  assert(s);
  if (s->registered)
    return;
  // ignore exceptions, it must be failure on exception in test system
  if (std::find(registered_sections.begin(), registered_sections.end(), s) == registered_sections.end()) {
    s->registered = true;
    registered_sections.push_back(s);
  }
}

bool section_info::enter_section(section_info* s) noexcept {
  assert(s);
  if (current_run_section)
    return s == current_run_section;  // allow to sections in loop
  if (!s->need_run())
    return false;
  current_run_section = s;
  s->was_entered = true;
  s->prepare_to_run();
  return true;
}

void section_info::leave_section(section_info* s) noexcept {
  assert(s && s == current_run_section);
}

void section_info::prepare_to_run() noexcept {
  current_run_section = nullptr;
}

void section_info::mark_toplevel() noexcept {
  was_entered = true;  // mark entered, since no one will mark top level section entered
  registered = true;
}

bool section_info::need_run() noexcept {
  if (!was_entered)
    return true;  // not runned yet
  // false when registered sections empty
  bool b = std::any_of(registered_sections.begin(), registered_sections.end(),
                       [](section_info* i) { return i->need_run(); });
  if (b || generation_index == size_t(-1))
    return b;
  if (generation_index == generation_index_bound - 1)
    return false;
  // all sections ran with this generation value, get next
  ++generation_index;
  reuse_inners();
  return true;
}

// returns name -> section -> section etc of last runned case
std::string section_info::runned_case_name() const {
  assert(registered && was_entered);
  std::string res = name;
  if (current_run_section)
    res = std::move(res) + "::" + current_run_section->runned_case_name();

  if (generation_index != size_t(-1))
    res += std::format("::G{}", generation_index);
  return res;
}

}  // namespace moko3
