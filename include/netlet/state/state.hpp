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

#ifndef NETLET_STATE_HPP
#define NETLET_STATE_HPP

#include <netlet/state/callbacks.hpp>
#include <netlet/state/transition.hpp>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Netlet::State {

template <typename StateEnum> class State {
private:
  StateEnum id;
  std::string name;

  StateAction<StateEnum> entry_action;
  StateAction<StateEnum> exit_action;

  std::map<StateEnum, std::shared_ptr<State<StateEnum>>> child_state;

public:
  explicit State(StateEnum id, const std::string &name = "")
      : id(id),
        name(name.empty() ? std::to_string(static_cast<int>(id)) : name),
        entry_action(), exit_action(), child_state() {}

  State(const State &) = delete;
  State &operator=(const State &) = delete;

  StateEnum get_id() const { return this->id; }

  const std::string &get_name() const { return this->name; }

  void set_entry_action(StateAction<StateEnum> action) {
    this->entry_action = std::move(action);
  }

  void set_exit_action(StateAction<StateEnum> action) {
    this->exit_action = std::move(action);
  }

  void on_entry() const {
    if (this->entry_action)
      this->entry_action(*this);
  }

  void on_exit() const {
    if (this->exit_action)
      this->exit_action(*this);
  }

  void add_child_state(std::shared_ptr<State<StateEnum>> child_state) {
    if (this->child_state.find(child_state->get_id()) ==
        this->child_state.end())
      this->child_state[child_state->get_id()] = std::move(child_state);
  }

  std::shared_ptr<State<StateEnum>> get_child_state(StateEnum child_id) const {
    auto it = this->child_state.find(child_id);
    return it != this->child_state.end() ? it->second : nullptr;
  }

  bool contains(const State<StateEnum> &other_state) const {
    return this->child_state.count(other_state.get_id()) > 0;
  }
};

template <typename StateEnum, typename EventEnum> class StateMachine {
private:
  std::string name;

  std::map<StateEnum, std::shared_ptr<State<StateEnum>>> states;
  std::map<EventEnum, std::shared_ptr<StateEvent<EventEnum>>> events;

  std::map<
      StateEnum,
      std::map<EventEnum, std::shared_ptr<Transition<StateEnum, EventEnum>>>>
      transitions;

  std::map<EventEnum, std::shared_ptr<Transition<StateEnum, EventEnum>>>
      global_transitions;

  std::shared_ptr<State<StateEnum>> current_state;
  std::shared_ptr<State<StateEnum>> initial_state;

  mutable std::mutex mtx;

public:
  explicit StateMachine(const std::string &name = "<anon-state-mach>")
      : name(name), states(), events(), transitions(), global_transitions(),
        current_state(nullptr), initial_state(nullptr), mtx() {}

  StateMachine(const StateMachine &) = delete;
  StateMachine &operator=(const StateMachine &) = delete;

  std::shared_ptr<State<StateEnum>> add_state(StateEnum id,
                                              const std::string &name = "") {
    std::lock_guard<std::mutex> lock(this->mtx);
    if (this->states.count(id))
      throw StateConfigurationException("State with ID " +
                                        std::to_string(static_cast<int>(id)) +
                                        " already exists");

    auto new_state = std::make_shared<State<StateEnum>>(id, name);
    this->states[id] = new_state;

    return new_state;
  }

  std::shared_ptr<State<StateEnum>> get_state(StateEnum id) const {
    auto it = this->states.find(id);
    if (it != this->states.end())
      return it->second;

    throw UnknownStateException(
        "State " + std::to_string(static_cast<int>(id)) + " not found");
  }

  std::shared_ptr<StateEvent<EventEnum>>
  add_event(EventEnum id, const std::string &name = "") {
    std::lock_guard<std::mutex> lock(this->mtx);

    if (this->events.count(id))
      throw StateConfigurationException("Event with ID " +
                                        std::to_string(static_cast<int>(id)) +
                                        " already exists");

    auto new_event = std::make_shared<StateEvent<EventEnum>>(id, name);

    this->events[id] = new_event;
    return new_event;
  }

  std::shared_ptr<StateEvent<EventEnum>> get_event(EventEnum id) const {
    auto it = this->events.find(id);
    if (it != this->events.end())
      return it->second;

    throw UnknownEventException(
        "Event " + std::to_string(static_cast<int>(id)) + " not found");
  }

  std::shared_ptr<Transition<StateEnum, EventEnum>>
  add_transition(StateEnum from_state_id, StateEnum to_state_id,
                 EventEnum event_id) {
    std::lock_guard<std::mutex> lock(this->mtx);

    auto from_state = this->get_state(from_state_id);
    auto to_state = this->get_state(to_state_id);
    auto event = this->get_event(event_id);

    if (this->transitions.count(from_state_id) &&
        this->transitions[from_state_id].count(event_id))
      throw StateConfigurationException("Transition from State " +
                                        from_state->get_name() + " on event " +
                                        event->get_name() + " already exists");

    auto new_transition = std::make_shared<Transition<StateEnum, EventEnum>>(
        from_state, to_state, event);

    this->transitions[from_state_id][event_id] = new_transition;
    return new_transition;
  }

  void set_initial_state(StateEnum initial_state_id) {
    std::lock_guard<std::mutex> lock(this->mtx);

    if (this->current_state != nullptr)
      throw StateConfigurationException(
          "Cannot set initial state after state machine has started");

    this->initial_state = this->get_state(initial_state_id);
  }

  void start() {
    std::lock_guard<std::mutex> lock(this->mtx);

    if (this->current_state != nullptr)
      throw std::logic_error("State machine is already running");

    if (this->initial_state == nullptr)
      throw StateConfigurationException("Initial state not set");

    this->current_state = this->initial_state;
    this->current_state->on_entry();
  }

  void stop() {
    std::lock_guard<std::mutex> lock(this->mtx);
    if (this->current_state != nullptr) {
      this->current_state->on_exit();
      this->current_state = nullptr;
    }
  }

  bool process_event(EventEnum event_id) {
    std::lock_guard<std::mutex> lock(this->mtx);
    if (this->current_state == nullptr)
      throw StateConfigurationException("State machine is not started");

    auto event = this->get_event(event_id);
    std::shared_ptr<Transition<StateEnum, EventEnum>> chosen_transition =
        nullptr;

    if (this->transitions.count(this->current_state->get_id())) {
      const auto &state_transitions =
          this->transitions[this->current_state->get_id()];

      if (state_transitions.count(event->get_id()))
        chosen_transition = state_transitions.at(event->get_id());
    }

    if (!chosen_transition && this->global_transitions.count(event->get_id()))
      chosen_transition = this->global_transitions.at(event->get_id());

    if (chosen_transition) {
      if (chosen_transition->check_guard_condition_with_current_state(
              *this->current_state)) {
        this->current_state->on_exit();
        chosen_transition->on_transition_with_current_state(
            *this->current_state);

        this->current_state = chosen_transition->get_to_state();
        this->current_state->on_entry();

        return true;
      }
    }

    return false;
  }

  std::shared_ptr<State<StateEnum>> get_current_state() const {
    std::lock_guard<std::mutex> lock(this->mtx);
    return this->current_state;
  }

  bool is_running() const {
    std::lock_guard<std::mutex> lock(this->mtx);
    return this->current_state != nullptr;
  }

  const std::string &get_name() const { return this->name; }

  std::shared_ptr<Transition<StateEnum, EventEnum>>
  add_global_transition(StateEnum to_state_id, EventEnum event_id) {
    std::lock_guard<std::mutex> lock(this->mtx);

    auto to_state = get_state(to_state_id);
    auto event = get_event(event_id);

    if (this->global_transitions.count(event_id))
      throw StateConfigurationException("Global transition on event " +
                                        event->get_name() + " already exists");

    auto dummy_from_state = this->states.begin()->second;
    auto new_global_transition =
        std::make_shared<Transition<StateEnum, EventEnum>>(dummy_from_state,
                                                           to_state, event);

    this->global_transitions[event_id] = new_global_transition;
    return new_global_transition;
  }

  std::map<StateEnum, std::shared_ptr<State<StateEnum>>>
  get_all_states() const {
    std::lock_guard<std::mutex> lock(this->mtx);
    return this->states;
  }

  std::map<EventEnum, std::shared_ptr<StateEvent<EventEnum>>>
  get_all_events() const {
    std::lock_guard<std::mutex> lock(this->mtx);
    return this->events;
  }

  std::map<EventEnum, std::shared_ptr<Transition<StateEnum, EventEnum>>>
  get_transitions_from_state(StateEnum from_state_id) const {
    std::lock_guard<std::mutex> lock(this->mtx);
    if (!this->states.count(from_state_id))
      throw UnknownStateException(
          "State " + std::to_string(static_cast<int>(from_state_id)) +
          " not found");

    auto it = this->transitions.find(from_state_id);
    return it != this->transitions.end() ? it->second : ({});
  }

  void clear() {
    std::lock_guard<std::mutex> lock(this->mtx);
    if (this->current_state != nullptr)
      throw std::logic_error("Cannot clear state machine while it is running");

    this->states.clear();
    this->events.clear();

    this->transitions.clear();
    this->global_transitions.clear();

    this->current_state = nullptr;
    this->initial_state = nullptr;
  }
};

} // namespace Netlet::State

#endif
