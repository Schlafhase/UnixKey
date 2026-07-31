#include "config.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

void from_json(const nlohmann::json &j, unixKeyConfigJson &c) {
  j.at("case_sensitive").get_to(c.caseSensitive);
  j.at("case_insensitive").get_to(c.caseInsensitive);
}

void to_json(nlohmann::json &j, const unixKeyConfigJson &c) {
  j = nlohmann::json{{"case_sensitive", c.caseSensitive},
                     {"case_insensitive", c.caseInsensitive}};
}

unixKeyConfig::unixKeyConfig(std::string file) {
  std::ifstream in(file);
  nlohmann::json j = nlohmann::json::parse(in);

  unixKeyConfigJson jsonConfig = j.get<unixKeyConfigJson>();

  // now convert the raw json objects to actual config

  replacements = {};
  for (const auto &[key, value] : jsonConfig.caseSensitive) {
    std::string keyLower = key;
    replacements.push_back({keyLower, value, true});
  }
  for (const auto &[key, value] : jsonConfig.caseInsensitive) {
    std::string keyLower = key;
    // case insensitive stuff can just be lowercased because i don't care about the case
    std::transform(keyLower.begin(), keyLower.end(), keyLower.begin(),
                   ::tolower);
    replacements.push_back({keyLower, value, false});
  }

  std::sort(replacements.begin(), replacements.end(),
            [](const replacement &a, const replacement &b) {
              return a.from.size() > b.from.size();
            });
}
