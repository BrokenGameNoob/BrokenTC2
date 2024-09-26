#pragma once

#include <chrono>

namespace utils {

inline double HighResMs() {
  static const auto kAppStart{std::chrono::high_resolution_clock::now()};
  const auto now{std::chrono::high_resolution_clock::now()};
  return static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(now - kAppStart).count()) / 1000.0;
}

}  // namespace utils
