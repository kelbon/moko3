#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <cassert>
#include <memory>

#include <iostream>

namespace moko3 {

// идентифицирует SECTION в исходнике, один статический объект на каждое использование макроса
struct section_key {
  std::string_view name;
};

struct section_info {
  std::string name;
  // SECTION в исходнике, которую представляет эта секция (nullptr для верхнего уровня)
  const section_key* key = nullptr;
  // inner section used in this run
  section_info* current_run_section = nullptr;
  // вложенные секции, создаются в `child`. Одна и та же SECTION в исходнике под разными родителями
  // (например функция с секциями, вызываемая из нескольких секций) - разные секции, поэтому состояние
  // хранится здесь, а не в статическом `section_key`
  std::vector<std::unique_ptr<section_info>> registered_sections;
  bool was_entered = false;
  bool registered = false;
  bool toplevel = false;
  size_t generate_index = 0;
  size_t max_generation_index = size_t(-1);
  // false после SECTION/GENERATE
  bool can_generate = true;

  [[nodiscard]] bool has_generate() const noexcept {
    return max_generation_index != size_t(-1);
  }

  void mark_not_entered() noexcept {
    assert(was_entered);
    was_entered = false;
    if (has_generate()) {
      assert(!has_more_generate_values());
      generate_index = 0;
      // GENERATE может зависеть от родительской секции, количество значений запоминается заново
      max_generation_index = size_t(-1);
    }
    for (auto& i : registered_sections)
      i->mark_not_entered();
  }

  [[nodiscard]] bool has_more_generate_values() const noexcept {
    return has_generate() && generate_index < max_generation_index;
  }
  // used by GENERATE macros on each invocation (even first)
  // returns index of value for this run
  [[nodiscard]] size_t next_generated_index(size_t count, int line) noexcept {
    if (!can_generate) {
      std::cerr << "!!!GENERATE MUST BE CALLED ONLY ONCE AND BEFORE ANY SECTION!!! abort called in test "
                << name << ", line " << line << std::endl;
      std::abort();
    }
    can_generate = false;
    if (!has_generate()) {
      assert(count > 0);
      max_generation_index = count - 1;
    }
    assert(max_generation_index == count - 1);
    return generate_index;
  }

  template <typename R>
  [[nodiscard]] std::ranges::range_value_t<R> next_generated_value(R values, int line) noexcept {
    static_assert(std::ranges::random_access_range<R> && std::ranges::sized_range<R>);
    // ignore exceptions here
    return std::move(values[next_generated_index(std::ranges::size(values), line)]);
  }

  // resets *this and inner sections to be ready to run again
  // but dont touch `registered` etc
  void reuse();

  void reuse_inners() {
    for (auto& s : registered_sections)
      s->reuse();
  }

  // возвращает вложенную секцию для `key`, при первом вызове создаёт её
  [[nodiscard]] section_info* child(const section_key* key);

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
