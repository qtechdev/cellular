#ifndef __ERROR_HPP__
#define __ERROR_HPP__

template <typename E>
constexpr auto to_underlying(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

enum class error_code_t {
  not_enough_args = 1,
  too_many_args = 2,
  window_failed = 16,
  glad_failed = 17,

};

#endif // __ERROR_HPP__
