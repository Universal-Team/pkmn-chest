/*
 *   This file is part of PKSM
 *   Copyright (C) 2016-2020 Bernardo Giordano, Admiral Fish, piepie62
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
// #include "Archive.hpp"
// #include "Configuration.hpp"
#include "flashcard.hpp"
#include "json.hpp"

// Public on purpose: banks being converted need to set their size
nlohmann::json g_banks;

namespace
{
    bool createJson()
    {
        g_banks           = nlohmann::json::object();
        g_banks["pksm_1"] = BANK_DEFAULT_SIZE;
        return Banks::saveJson();
    }

    bool read()
    {
        std::string path = mainDrive() + ":/_nds/pkmn-chest/banks.json";
        FILE *in          = fopen(path.c_str(), "rb");
        if (in)
        {
            fseek(in, 0, SEEK_END);
            size_t size = ftell(in);
            fseek(in, 0, SEEK_SET);
            char data[size + 1];
            fread(data, 1, size, in);
            data[size] = '\0';
            fclose(in);
            g_banks = nlohmann::json::parse(data, nullptr, false);
        }
        else
        {
            return createJson();
        }
        return true;
    }
}

bool Banks::saveJson()
{
    std::string jsonData = g_banks.dump(2);
    std::string path     = mainDrive() + ":/_nds/pkmn-chest/banks.json";
    // (Configuration::getInstance().useExtData() ? Archive::data() : Archive::sd()).deleteFile(path);
    // (Configuration::getInstance().useExtData() ? Archive::data() : Archive::sd()).createFile(path, 0, jsonData.size());
    FILE *out = fopen(path.c_str(), "wb");
    if (out)
    {
        fwrite(jsonData.data(), 1, jsonData.size() + 1, out);
        fclose(out);
        return true;
    }
    else
    {
        fclose(out);
        return false;
    }
}

int Banks::init()
{
    int res;
    if ((res = !read()))
        return res;

    if (g_banks.is_discarded())
        return -1;

    auto i = g_banks.find("pksm_1");
    if (i == g_banks.end())
    {
        i = g_banks.begin();
    }
    loadBank(i.key(), i.value());
    if (bank)
    {
        return 0;
    }
    else
    {
        return -2;
    }
}

bool Banks::loadBank(const std::string& name, const std::optional<int>& maxBoxes)
{
    if (!bank || bank->name() != name)
    {
        auto found = g_banks.find(name);
        if (found == g_banks.end())
        {
            g_banks[name] = maxBoxes.value_or(BANK_DEFAULT_SIZE);
            saveJson();
            found = g_banks.find(name);
        }
        bank = std::make_shared<Bank>(found.key(), found.value().get<int>());
        return true;
    }
    return false;
}

void Banks::removeBank(const std::string& name)
{
    if (g_banks.size() == 1)
    {
        return;
    }
    if (g_banks.contains(name))
    {
        if (bank && bank->name() == name)
        {
            bank   = nullptr;
            auto i = g_banks.begin();
            if (i.key() == name)
            {
                i++;
            }
            loadBank(i.key(), i.value());
        }
        remove((mainDrive() + ":/_nds/pkmn-chest/banks/" + name + ".bnk").c_str());
        remove((mainDrive() + ":/_nds/pkmn-chest/banks/" + name + ".json").c_str());
        // Archive::data().deleteFile("/banks/" + name + ".bnk");
        // Archive::data().deleteFile("/banks/" + name + ".json");
        for (auto i = g_banks.begin(); i != g_banks.end(); i++)
        {
            if (i.key() == name)
            {
                g_banks.erase(i);
                saveJson();
                break;
            }
        }
    }
}

std::vector<std::pair<std::string, int>> Banks::bankNames()
{
    std::vector<std::pair<std::string, int>> ret(g_banks.size());
    for (auto i = g_banks.begin(); i != g_banks.end(); i++)
    {
        ret[std::distance(g_banks.begin(), i)] = {i.key(), i.value().get<int>()};
    }
    return ret;
}

void Banks::renameBank(const std::string& oldName, const std::string& newName)
{
    if (oldName != newName && g_banks.contains(oldName))
    {
        if (bank->name() == oldName)
        {
            if (!bank->setName(newName))
            {
                return;
            }
        }
        else
        {
            rename((mainDrive() + ":/_nds/pkmn-chest/banks/" + oldName + ".bnk").c_str(), (mainDrive() + ":/_nds/pkmn-chest/banks/" + newName + ".bnk").c_str());
            rename((mainDrive() + ":/_nds/pkmn-chest/banks/" + oldName + ".json").c_str(), (mainDrive() + ":/_nds/pkmn-chest/banks/" + newName + ".json").c_str());
        }
        g_banks[newName] = g_banks[oldName];
        g_banks.erase(oldName);
        saveJson();
    }
}

void Banks::setBankSize(const std::string& name, int size)
{
    if (g_banks.count(name))
    {
        g_banks[name] = size;
        if (bank && bank->name() == name && size != bank->boxes())
        {
            bank->resize(size);
        }
        saveJson();
    }
}

// Result Banks::swapSD(bool toSD)
// {
//     Result res = 0;
//     if (toSD)
//     {
//         if (R_FAILED(res = Archive::moveDir(Archive::data(), "/banks", Archive::sd(), "/3ds/PKSM/banks")))
//             return res;
//         if (R_FAILED(res = Archive::moveFile(Archive::data(), "/banks.json", Archive::sd(), "/3ds/PKSM/banks.json")))
//             return res;
//     }
//     else
//     {
//         if (R_FAILED(res = Archive::moveDir(Archive::sd(), "/3ds/PKSM/banks", Archive::data(), "/banks")))
//             return res;
//         if (R_FAILED(res = Archive::moveFile(Archive::sd(), "/3ds/PKSM/banks.json", Archive::data(), "/banks.json")))
//             return res;
//     }
//     return res;
// }
