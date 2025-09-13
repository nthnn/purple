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

#include <purple/format/robots_txt.hpp>

#include <algorithm>
#include <sstream>

namespace Purple::Format {

std::string robots_str_trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t\n\r");
  if (std::string::npos == first)
    return str;

  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, (last - first + 1));
}

UserAgentBlock::UserAgentBlock()
    : user_agents({}), rules(), crawl_delay(""), host("") {}

bool UserAgentBlock::operator==(const UserAgentBlock &other) const {
  if (this->user_agents != other.user_agents ||
      this->crawl_delay != other.crawl_delay || this->host != other.host ||
      this->rules.size() != other.rules.size())
    return false;

  for (size_t i = 0; i < this->rules.size(); ++i)
    if (!(this->rules[i] == other.rules[i]))
      return false;

  return true;
}

RobotsTxt::RobotsTxt() : user_agent_blocks(), sitemaps({}) {}

std::vector<UserAgentBlock> RobotsTxt::get_user_agent_blocks() {
  return this->user_agent_blocks;
}

std::set<std::string> RobotsTxt::get_sitemaps() { return this->sitemaps; }

void RobotsTxt::set_user_agent_blocks(std::vector<UserAgentBlock> blocks) {
  this->user_agent_blocks = blocks;
}

void RobotsTxt::set_sitemaps(std::set<std::string> map) {
  this->sitemaps = map;
}

bool RobotsTxt::operator==(const RobotsTxt &other) const {
  if (this->user_agent_blocks.size() != other.user_agent_blocks.size() ||
      this->sitemaps != other.sitemaps)
    return false;

  for (size_t i = 0; i < this->user_agent_blocks.size(); ++i)
    if (!(this->user_agent_blocks[i] == other.user_agent_blocks[i]))
      return false;

  return true;
}

RobotsTxt RobotsTxt::parse(const std::string &content) {
  RobotsTxt robots;
  std::stringstream ss(content);
  std::string line;

  UserAgentBlock current_block;
  bool in_user_agent_block = false;

  while (std::getline(ss, line)) {
    line = robots_str_trim(line);
    if (line.empty() || line[0] == '#')
      continue;

    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos)
      continue;

    std::string directive = robots_str_trim(line.substr(0, colonPos));
    std::string value = robots_str_trim(line.substr(colonPos + 1));

    std::transform(directive.begin(), directive.end(), directive.begin(),
                   ::tolower);

    if (directive == "user-agent") {
      if (in_user_agent_block) {
        robots.user_agent_blocks.push_back(current_block);
        current_block = UserAgentBlock();
      }

      current_block.user_agents.insert(value);
      in_user_agent_block = true;
    } else if (directive == "allow") {
      if (in_user_agent_block)
        current_block.rules.push_back({DirectiveType::Allow, value});
    } else if (directive == "disallow") {
      if (in_user_agent_block)
        current_block.rules.push_back({DirectiveType::Disallow, value});
    } else if (directive == "crawl-delay") {
      if (in_user_agent_block)
        current_block.crawl_delay = value;
    } else if (directive == "sitemap")
      robots.sitemaps.insert(value);
    else if (directive == "host") {
      if (in_user_agent_block)
        current_block.host = value;
    }
  }

  if (in_user_agent_block)
    robots.user_agent_blocks.push_back(current_block);
  return robots;
}

std::string RobotsTxt::build() {
  std::stringstream ss;
  for (const auto &block : this->user_agent_blocks) {
    for (const auto &ua : block.user_agents)
      ss << "User-agent: " << ua << "\n";

    for (const auto &rule : block.rules) {
      if (rule.type == DirectiveType::Allow)
        ss << "Allow: " << rule.path << "\n";
      else if (rule.type == DirectiveType::Disallow)
        ss << "Disallow: " << rule.path << "\n";
    }

    if (!block.crawl_delay.empty())
      ss << "Crawl-delay: " << block.crawl_delay << "\n";

    if (!block.host.empty())
      ss << "Host: " << block.host << "\n";
    ss << "\n";
  }

  for (const auto &sitemap : this->sitemaps)
    ss << "Sitemap: " << sitemap << "\n";

  return ss.str();
}

bool RobotsTxt::is_path_allowed(const std::string &user_agent,
                                const std::string &path) {
  const UserAgentBlock *best_match = nullptr;
  int best_match_score = -1;

  for (const auto &block : this->user_agent_blocks) {
    for (const auto &ua : block.user_agents)
      if (ua == "*" && best_match_score < 0) {
        best_match = &block;
        best_match_score = 0;
      } else if (ua == user_agent) {
        best_match = &block;
        best_match_score = 1;
        break;
      }

    if (best_match_score == 1)
      break;
  }

  if (!best_match)
    return true;

  bool allowed = true;
  std::vector<RobotsTxtRule> sorted_rules = best_match->rules;
  std::sort(sorted_rules.begin(), sorted_rules.end(),
            [](const RobotsTxtRule &a, const RobotsTxtRule &b) {
              return a.path.length() > b.path.length();
            });

  for (const auto &rule : sorted_rules)
    if (path.rfind(rule.path, 0) == 0) {
      bool rule_ends_with_path = rule.path.back() == '$';
      std::string effective_rule_path =
          rule_ends_with_path ? rule.path.substr(0, rule.path.length() - 1)
                              : rule.path;

      if (path.rfind(effective_rule_path, 0) == 0) {
        bool path_matches_rule = false;

        if (rule_ends_with_path)
          path_matches_rule = (path == effective_rule_path);
        else
          path_matches_rule = path.rfind(effective_rule_path, 0) == 0;

        if (path_matches_rule) {
          if (rule.type == DirectiveType::Disallow)
            return false;
          else if (rule.type == DirectiveType::Allow)
            return true;
        }
      }
    }

  return allowed;
}

} // namespace Purple::Format
