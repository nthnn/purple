/*
 * Copyright (c) 2025 - Nathanne Isip
 * This file is part of Purple.
 *
 * Purple is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Purple is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Purple. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PURPLE_HELPER_UUID_HPP
#define PURPLE_HELPER_UUID_HPP

#include <random>
#include <sstream>

namespace Purple::Helper {

class UUIDGenerator {
private:
  std::mt19937 rng;
  std::uniform_int_distribution<int> dist;

public:
  UUIDGenerator();
  std::string generate();
};

} // namespace Purple::Helper

#endif
