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

#include <purple/helper/uuid.hpp>

namespace Purple::Helper {

UUIDGenerator::UUIDGenerator() : rng(std::random_device()()), dist{0, 15} {}

std::string UUIDGenerator::generate() {
  std::stringstream ss;
  ss << std::hex;

  for (int i = 0; i < 8; ++i)
    ss << this->dist(this->rng);
  ss << "-";

  for (int i = 0; i < 4; ++i)
    ss << this->dist(this->rng);
  ss << "-";

  ss << "4";
  for (int i = 0; i < 3; ++i)
    ss << this->dist(this->rng);
  ss << "-";

  int c = (this->dist(this->rng) & 0x3) | 0x8;
  ss << std::hex << c;

  for (int i = 0; i < 3; ++i)
    ss << this->dist(this->rng);
  ss << "-";

  for (int i = 0; i < 12; ++i)
    ss << this->dist(this->rng);

  return ss.str();
}

} // namespace Purple::Helper
