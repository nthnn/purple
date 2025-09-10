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

#ifndef NETLET_STATE_TRANSITION_HPP
#define NETLET_STATE_TRANSITION_HPP

#include <netlet/state/callbacks.hpp>
#include <netlet/state/event.hpp>
#include <netlet/state/state.hpp>

#include <memory>

namespace Netlet::State {

template <typename StateEnum, typename EventEnum> class Transition {
private:
  std::shared_ptr<State<StateEnum>> from_state;
  std::shared_ptr<State<StateEnum>> to_state;
  std::shared_ptr<StateEvent<EventEnum>> event;

  TransitionAction<StateEnum, EventEnum> action;
  GuardCondition<StateEnum, EventEnum> guard;

public:
  Transition(std::shared_ptr<State<StateEnum>> from_state,
             std::shared_ptr<State<StateEnum>> to_state,
             std::shared_ptr<StateEvent<EventEnum>> event)
      : from_state(std::move(from_state)), to_state(std::move(to_state)),
        event(std::move(event)), action(), guard() {
    if (!this->from_state || !this->to_state || !this->event)
      throw StateConfigurationException(
          "Transition cannot be created with null states or event");
  }

  std::shared_ptr<State<StateEnum>> get_from_state() const {
    return this->from_state;
  }

  std::shared_ptr<State<StateEnum>> get_to_state() const {
    return this->to_state;
  }

  std::shared_ptr<StateEvent<EventEnum>> get_event() const {
    return this->event;
  }

  void set_action(TransitionAction<StateEnum, EventEnum> action) {
    this->action = std::move(action);
  }

  void set_guard_condition(GuardCondition<StateEnum, EventEnum> guard) {
    this->guard = std::move(guard);
  }

  void on_transition_with_current_state(
      const State<StateEnum> &actual_from_state) const {
    if (this->action)
      this->action(actual_from_state, *this->to_state, *this->event);
  }

  bool check_guard_condition_with_current_state(
      const State<StateEnum> &actual_from_state) const {
    return this->guard
               ? this->guard(actual_from_state, *this->to_state, *this->event)
               : true;
  }
};

} // namespace Netlet::State

#endif
