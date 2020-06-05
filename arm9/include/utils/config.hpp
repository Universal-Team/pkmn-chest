#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include "enums/Language.hpp"

namespace Config {
	void load();
	void save();

	bool getBool(const std::string &key);
	void setBool(const std::string &key, bool v);

	int getInt(const std::string &key);
	void setInt(const std::string &key, int v);

	std::string getString(const std::string &key);
	void setString(const std::string &key, const std::string &v);

	pksm::Language getLang(const std::string &key);
	void setLang(const std::string &key, pksm::Language lang);
}

#endif
