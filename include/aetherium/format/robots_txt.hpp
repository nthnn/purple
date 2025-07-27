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

#ifndef AETHERIUM_FORMAT_ROBOTS_TXT_HPP
#define AETHERIUM_FORMAT_ROBOTS_TXT_HPP

#include <set>
#include <string>
#include <vector>

namespace Aetherium::Format {

enum class DirectiveType {
    UserAgent,
    Allow,
    Disallow,
    CrawlDelay,
    Sitemap,
    Host,
    Unknown
};

struct RobotsTxtRule {
    DirectiveType type;
    std::string path;

    bool operator==(
        const RobotsTxtRule& other
    ) const {
        return this->type == other.type &&
            this->path == other.path;
    }
};

struct UserAgentBlock {
    std::set<std::string> user_agents;
    std::vector<RobotsTxtRule> rules;
    std::string crawl_delay;
    std::string host;

    UserAgentBlock();
    bool operator==(
        const UserAgentBlock& other
    ) const;
};

class RobotsTxt {
private:
    std::vector<UserAgentBlock> user_agent_blocks;
    std::set<std::string> sitemaps;

public:
    RobotsTxt();

    std::set<std::string> get_sitemaps();
    std::vector<UserAgentBlock> get_user_agent_blocks();

    void set_sitemaps(std::set<std::string> map);
    void set_user_agent_blocks(
        std::vector<UserAgentBlock> blocks
    );

    bool operator==(const RobotsTxt& other) const;

    static RobotsTxt parse(
        const std::string& content
    );

    std::string build();
    bool is_path_allowed(
        const std::string& user_agent,
        const std::string& path
    );
};

}

#endif
