/*
 * Copyright (c) 2025 - Nathanne Isip
 * This file is part of Netlet.
 *
 * Netlet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Netlet is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Netlet. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NETLET_STATE_EVENT_HPP
#define NETLET_STATE_EVENT_HPP

#include <string>

namespace Netlet::State {

template <typename EventEnum> class StateEvent {
private:
  EventEnum id;
  std::string name;

public:
  explicit StateEvent(EventEnum id, const std::string &name = "")
      : id(id),
        name(name.empty() ? std::to_string(static_cast<int>(id)) : name) {}

  EventEnum get_id() const { return this->id; }

  const std::string &get_name() const { return this->name; }
};

} // namespace Netlet::State

#endif
