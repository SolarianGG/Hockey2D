#pragma once

#include <algorithm>
#include <concepts>
#include <iterator>

#include "game_data.hpp"

namespace mp {

template <typename FwdIt>
requires(std::forward_iterator<FwdIt>)
FwdIt FindRequiredPlayer(FwdIt begin, FwdIt end, const std::uint32_t id) {
  return std::find_if(begin, end,
                      [id](const mp::Player& mp) { return mp.id == id; });
}

}  // namespace mp

namespace std {

constexpr mp::Vector2 clamp(const mp::Vector2 val, const mp::Vector2 min,
                            const mp::Vector2 max) {
  const float x = std::clamp(val.x, min.x, max.x);
  const float y = std::clamp(val.y, min.y, max.y);
  return {x, y};
}
}  // namespace std
