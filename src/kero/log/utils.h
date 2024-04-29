#ifndef KERO_LOG_UTILS_H
#define KERO_LOG_UTILS_H

#include <ostream>
#include <thread>
#include <vector>

#include "kero/core/common.h"

namespace kero {

template <typename>
inline constexpr bool always_false_v = false;

class NullStream : public std::ostream {
 public:
  NullStream() noexcept;
  ~NullStream() noexcept = default;
  CLASS_KIND_PINNABLE(NullStream);

 private:
  class NullBuffer : public std::streambuf {
   public:
    NullBuffer() noexcept = default;
    ~NullBuffer() noexcept = default;
    CLASS_KIND_PINNABLE(NullBuffer);

    virtual auto
    overflow(int c) noexcept -> int override;
  };

  NullBuffer null_buffer{};
};

auto
ThreadIdToString(const std::thread::id& thread_id) -> std::string;

auto
TimePointToIso8601(const std::chrono::system_clock::time_point& time_point)
    -> std::string;

template <typename T>
auto
operator<<(std::ostream& os, const std::vector<T>& vec) -> std::ostream& {
  os << "[";
  for (auto it = vec.begin(); it != vec.end(); ++it) {
    os << *it;
    if (std::next(it) != vec.end()) {
      os << ", ";
    }
  }
  os << "]";
  return os;
}

}  // namespace kero

#endif  // KERO_LOG_UTILS_H
