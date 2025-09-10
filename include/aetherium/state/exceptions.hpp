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

#ifndef AETHERIUM_STATE_EXCEPTIONS_HPP
#define AETHERIUM_STATE_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace Aetherium::State {

class UnknownStateException : public std::runtime_error {
public:
  explicit UnknownStateException(const std::string &message)
      : std::runtime_error("Unknown State: " + message) {}
};

class UnknownEventException : public std::runtime_error {
public:
  explicit UnknownEventException(const std::string &message)
      : std::runtime_error("Unknown Event: " + message) {}
};

class InvalidTransitionException : public std::runtime_error {
public:
  explicit InvalidTransitionException(const std::string &message)
      : std::runtime_error("Invalid Transition: " + message) {}
};

class StateConfigurationException : public std::runtime_error {
public:
  explicit StateConfigurationException(const std::string &message)
      : std::runtime_error("State Configuration Error: " + message) {}
};

} // namespace Aetherium::State

#endif
