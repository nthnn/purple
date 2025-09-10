/*
 * Copyright (c) 2025 - Nathanne Isip
 * This file is part of Aetherium.
 *
 * Aetherium is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Aetherium is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Aetherium. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef AETHERIUM_STATE_CALLBACKS_HPP
#define AETHERIUM_STATE_CALLBACKS_HPP

#include <functional>
#include <memory>

namespace Aetherium::State {

template <typename StateEnum> class State;
template <typename EventEnum> class StateEvent;

template <typename StateEnum>
using StateAction = std::function<void(const State<StateEnum> &)>;

template <typename StateEnum, typename EventEnum>
using TransitionAction =
    std::function<void(const State<StateEnum> &,     // from_state
                       const State<StateEnum> &,     // to_state
                       const StateEvent<EventEnum> & // event
                       )>;

template <typename StateEnum, typename EventEnum>
using GuardCondition =
    std::function<bool(const State<StateEnum> &,     // from_state
                       const State<StateEnum> &,     // to_state
                       const StateEvent<EventEnum> & // event
                       )>;

using StateContext = void *;

} // namespace Aetherium::State

#endif
