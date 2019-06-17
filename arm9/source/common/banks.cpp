/*
 *   This file is part of PKSM
 *   Copyright (C) 2016-2019 Bernardo Giordano, Admiral Fish, piepie62
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
 *       * Requiring preservation of specified reasonable legal notices or
 *         author attributions in that material or in the Appropriate Legal
 *         Notices displayed by works containing it.
 *       * Prohibiting misrepresentation of the origin of that material,
 *         or requiring that modified versions of such material be marked in
 *         reasonable ways as different from the original version.
 */

#include "banks.hpp"
#include <fstream>
#include "json.hpp"
#include "../graphics/graphics.h"

std::shared_ptr<Bank> Banks::bank = nullptr;

nlohmann::json g_banks;

static bool saveJson() {
    std::string jsonData = g_banks.dump(2);
    remove("/_nds/pkmn-chest/banks.json");
    std::ofstream out("/_nds/pkmn-chest/banks.json");
    if(out.good()) {
        if(out.write(jsonData.data(), jsonData.size() + 1)) {
            out.close();
            return 1;
        } else {
            out.close();
            return 0;
        }
    } else {
        out.close();
        return 0;
    }
}

static bool createJson() {
    g_banks           = nlohmann::json::object();
    g_banks["pksm_1"] = BANK_DEFAULT_SIZE;
    return saveJson();
}

static bool read() {
    std::string path = "/_nds/pkmn-chest/banks.json";
    std::ifstream in(path);
    if(in.good()) {
        in.seekg(0, in.end);
        size_t size = in.tellg();
        in.seekg(0, in.beg);
        char data[size + 1];
        in.read(data, size);
        data[size] = '\0';
        in.close();
        g_banks = nlohmann::json::parse(data, nullptr, false);
        return 1;
    } else {
        in.close();
        return createJson();
    }
}

int Banks::init() {
    if(!read())
        return -1;

    if(g_banks.is_discarded())
        return -2;

    auto i = g_banks.find("pksm_1");
    if(i == g_banks.end()) {
        i = g_banks.begin();
    }

    return loadBank(i.key(), i.value());
}

bool Banks::loadBank(const std::string& name, int maxBoxes) {
    if(!bank || bank->name() != name) {
        auto found = g_banks.find(name);
        if(found == g_banks.end()) {
            g_banks[name] = (maxBoxes == 0 ? BANK_DEFAULT_SIZE : maxBoxes);
            saveJson();
            found = g_banks.find(name);
        }
        bank = std::make_shared<Bank>(found.key(), found.value().get<int>());
        return true;
    }
    return false;
}

void Banks::removeBank(const std::string& name) {
    if(g_banks.size() == 1) {
        return;
    }
    if(g_banks.contains(name)) {
        if(bank && bank->name() == name) {
            bank   = nullptr;
            auto i = g_banks.begin();
            if(i.key() == name) {
                i++;
            }
            loadBank(i.key(), i.value());
        }
        remove(("/_nds/pkmn-chest/banks/" + name + ".bnk").c_str());
        remove(("/_nds/pkmn-chest/banks/" + name + ".json").c_str());
        for(auto i = g_banks.begin(); i != g_banks.end(); i++) {
            if(i.key() == name) {
                g_banks.erase(i);
                saveJson();
                break;
            }
        }
    }
}

std::vector<std::pair<std::string, int>> Banks::bankNames() {
    std::vector<std::pair<std::string, int>> ret(g_banks.size());
    for(auto i = g_banks.begin(); i != g_banks.end(); i++) {
        ret[std::distance(g_banks.begin(), i)] = {i.key(), i.value().get<int>()};
    }
    return ret;
}

void Banks::renameBank(const std::string& oldName, const std::string& newName) {
    if(oldName != newName && g_banks.contains(oldName)) {
        if(bank->name() == oldName) {
            if(!bank->setName(newName)) {
                return;
            }
        } else {
            rename(("/_nds/pkmn-chest/banks/" + oldName + ".bnk").c_str(), ("/_nds/pkmn-chest/banks/" + newName + ".bnk").c_str());
            rename(("/_nds/pkmn-chest/banks/" + oldName + ".json").c_str(), ("/_nds/pkmn-chest/banks/" + newName + ".json").c_str());
        }
        g_banks[newName] = g_banks[oldName];
        g_banks.erase(oldName);
        saveJson();
    }
}

void Banks::setBankSize(const std::string& name, int size) {
    if(g_banks.count(name)) {
        g_banks[name] = size;
        if(bank && bank->name() == name && size != bank->boxes()) {
            bank->resize(size);
        }
        saveJson();
    }
}

// bool Banks::swapSD(bool toSD)
// {
//     bool res = 0;
//     if(toSD)
//     {
//         if(R_FAILED(res = Archive::moveDir(Archive::data(), "/banks", Archive::sd(), "/_nds/pkmn-chest/banks")))
//             return res;
//         if(R_FAILED(res = Archive::moveFile(Archive::data(), "/banks.json", Archive::sd(), "/_nds/pkmn-chest/banks.json")))
//             return res;
//     }
//     else
//     {
//         if(R_FAILED(res = Archive::moveDir(Archive::sd(), "/_nds/pkmn-chest/banks", Archive::data(), "/banks")))
//             return res;
//         if(R_FAILED(res = Archive::moveFile(Archive::sd(), "/_nds/pkmn-chest/banks.json", Archive::data(), "/banks.json")))
//             return res;
//     }
//     return res;
// }
