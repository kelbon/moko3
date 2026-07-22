#include "moko3/sections.hpp"

#include <algorithm>
#include <format>

namespace moko3 {

void section_info::reuse() {
  if (!toplevel && !need_run())
    return;
  was_entered = toplevel;
  current_run_section = nullptr;
  reuse_inners();
  auto is_need_run = [](section_info* i) { return i->need_run(); };
  if (std::ranges::none_of(registered_sections, is_need_run) && has_more_generate_values()) {
    ++generate_index;
    for (section_info* i : registered_sections)
      i->mark_not_entered();
  }
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
  toplevel = true;
}

bool section_info::need_run() noexcept {
  auto is_need_run = [](section_info* i) { return i->need_run(); };
  return !was_entered || has_more_generate_values() || std::ranges::any_of(registered_sections, is_need_run);
}

std::string section_info::runned_case_name() const {
  assert(registered && was_entered);
  std::string res = name;
  if (has_generate())
    res += std::format("::G{}", generate_index);
  if (current_run_section)
    res = std::move(res) + "::" + current_run_section->runned_case_name();

  return res;
}

}  // namespace moko3
