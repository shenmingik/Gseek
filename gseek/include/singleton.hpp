#pragma once

template <typename Class>
class Singleton {
 public:
  static Class& get_instance() {
      static Class instance;
      return instance;
  }

  Singleton(const Class &) = delete;
  Singleton(const Class &&) = delete;
  Singleton operator=(const Class &) = delete;

  protected:
  Singleton() = default;
};
