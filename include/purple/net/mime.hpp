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

/**
 * @file mime.hpp
 * @author Nathanne Isip <nathanneisip@gmail.com>
 * @brief Provides MIME type handling and mapping utilities.
 *
 * This file declares the Mime class, which maps file extensions to MIME
 * types and allows lookup of content types based on resource names.
 */
#ifndef PURPLE_NET_MIME_HPP
#define PURPLE_NET_MIME_HPP

#include <string>

namespace Purple::Net {

/*
 * The Mime class provides lookup facilities to translate file extensions
 * (e.g., ".html") into standard MIME types (e.g., "text/html"). It is used
 * to ensure correct Content-Type headers in web responses.
 */
std::string get_mime_type(const std::string &filename);

} // namespace Purple::Net

#endif
