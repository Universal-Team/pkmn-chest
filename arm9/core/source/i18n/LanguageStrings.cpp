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

#include "LanguageStrings.hpp"
#include "format.h"
#include "io.hpp"
#include "utils.hpp"

namespace
{
    nlohmann::json& formJson()
    {
        static nlohmann::json forms;
        static bool first = true;
        if (first)
        {
            FILE* in = fopen("romfs:/i18n/forms.json", "rt");
            if (in)
            {
                if (!ferror(in))
                {
                    forms = nlohmann::json::parse(in, nullptr, false);
                }
                fclose(in);
            }
            first = false;
        }
        return forms;
    }
}

std::string LanguageStrings::folder(Language lang)
{
    switch (lang)
    {
        case Language::EN:
            return "en";
        case Language::ES:
            return "es";
        case Language::DE:
            return "de";
        case Language::FR:
            return "fr";
        case Language::IT:
            return "it";
        case Language::JP:
            return "jp";
        case Language::KO:
            return "ko";
        case Language::NL:
            return "nl";
        case Language::PT:
            return "pt";
        case Language::ZH:
            return "zh";
        case Language::TW:
            return "tw";
        case Language::RO:
            return "ro";
        default:
            return "en";
    }

    return "en";
}

LanguageStrings::LanguageStrings(Language lang)
{
    load(lang, "/abilities.txt", abilities);
    load(lang, "/balls.txt", balls);
    load(lang, "/forms.txt", forms);
    load(lang, "/hp.txt", hps);
    load(lang, "/items.txt", items);
    load(lang, "/moves.txt", moves);
    load(lang, "/natures.txt", natures);
    load(lang, "/species.txt", speciess);
    load(lang, "/games.txt", games);
    load(lang, "/locations4.txt", locations4);
    load(lang, "/locations5.txt", locations5);
    load(lang, "/locations6.txt", locations6);
    load(lang, "/locations7.txt", locations7);
    load(lang, "/locationsLGPE.txt", locationsLGPE);
    load(lang, "/locations8.txt", locations8);
    countries = {};
    load(lang, "/countries.txt", countries);
    for (auto i = countries.begin(); i != countries.end(); i++)
    {
        subregions[i->first] = {};
        load(lang, fmt::format("/subregions/{:03d}.txt", i->first), subregions[i->first]);
    }
    load(lang, "/gui.json", gui);
}

void LanguageStrings::load(Language lang, const std::string& name, std::vector<std::string>& array)
{
    static constexpr const char* base = "romfs:/i18n/";
    std::string path                  = io::exists(base + folder(lang) + name) ? base + folder(lang) + name : base + folder(Language::EN) + name;

    std::string tmp;
    FILE* values = fopen(path.c_str(), "rt");
    if (values)
    {
        if (ferror(values))
        {
            fclose(values);
            return;
        }
        char* data  = (char*)malloc(128);
        size_t size = 0;
        while (!feof(values) && !ferror(values))
        {
            size = std::max(size, (size_t)128);
            if (__getline(&data, &size, values) >= 0)
            {
                tmp = std::string(data);
                tmp = tmp.substr(0, tmp.find('\n'));
                array.emplace_back(tmp.substr(0, tmp.find('\r')));
            }
            else
            {
                break;
            }
        }
        fclose(values);
        free(data);
    }
}

void LanguageStrings::load(Language lang, const std::string& name, nlohmann::json& json)
{
    static constexpr const char* base = "romfs:/i18n/";
    std::string path                  = io::exists(base + folder(lang) + name) ? base + folder(lang) + name : base + folder(Language::EN) + name;

    FILE* values = fopen(path.c_str(), "rt");
    if (values)
    {
        json = nlohmann::json::parse(values, nullptr, false);
        fclose(values);
    }
}

const std::string& LanguageStrings::ability(u16 v) const
{
    return v < abilities.size() ? abilities.at(v) : localize("INVALID_ABILITY");
}

const std::string& LanguageStrings::ball(u8 v) const
{
    return v < balls.size() ? balls.at(v) : localize("INVALID_BALL");
}

const std::string& LanguageStrings::form(u16 species, u16 form, Generation generation) const
{
    std::string sSpecies = std::to_string((int)species);
    if (formJson().contains(sSpecies))
    {
        std::vector<int> formIndices;
        if (formJson()[sSpecies].is_object())
        {
            if (formJson()[sSpecies].find(genToString(generation)) != formJson()[sSpecies].end())
            {
                formIndices = formJson()[sSpecies][genToString(generation)].get<std::vector<int>>();
            }
        }
        else
        {
            formIndices = formJson()[sSpecies].get<std::vector<int>>();
        }

        if (form < formIndices.size())
        {
            size_t formNameIndex = formIndices[form];
            if (formNameIndex < forms.size())
            {
                return forms.at(formNameIndex);
            }
        }
    }
    return localize("INVALID_FORM");
}

const std::string& LanguageStrings::hp(u8 v) const
{
    return v < hps.size() ? hps.at(v) : localize("INVALID_HP");
}

const std::string& LanguageStrings::item(u16 v) const
{
    return v < items.size() ? items.at(v) : localize("INVALID_ITEM");
}

const std::string& LanguageStrings::move(u16 v) const
{
    return v < moves.size() ? moves.at(v) : localize("INVALID_MOVE");
}

const std::string& LanguageStrings::nature(u8 v) const
{
    return v < natures.size() ? natures.at(v) : localize("INVALID_NATURE");
}

const std::string& LanguageStrings::species(u16 v) const
{
    return v < speciess.size() ? speciess.at(v) : localize("INVALID_SPECIES");
}

const std::string& LanguageStrings::localize(const std::string& v) const
{
    if (!gui.contains(v))
    {
        const_cast<nlohmann::json&>(gui)[v] = "MISSING: " + v;
    }
    return gui.at(v).get_ref<const std::string&>();
}

const std::vector<std::string>& LanguageStrings::rawItems() const
{
    return items;
}

const std::vector<std::string>& LanguageStrings::rawMoves() const
{
    return moves;
}

const std::string& LanguageStrings::subregion(u8 country, u8 v) const
{
    auto i = subregions.find(country);
    if (i != subregions.end())
    {
        auto j = i->second.find(v);
        if (j != i->second.end())
        {
            return j->second;
        }
        return localize("INVALID_SUBREGION");
    }
    return localize("INVALID_COUNTRY");
}

const std::string& LanguageStrings::country(u8 v) const
{
    auto i = countries.find(v);
    if (i != countries.end())
    {
        return i->second;
    }
    return localize("INVALID_COUNTRY");
}

const std::string& LanguageStrings::location(u16 v, Generation generation) const
{
    std::map<u16, std::string>::const_iterator i;
    switch (generation)
    {
        case Generation::FOUR:
            if ((i = locations4.find(v)) != locations4.end())
            {
                return i->second;
            }
            break;
        case Generation::FIVE:
            if ((i = locations5.find(v)) != locations5.end())
            {
                return i->second;
            }
            break;
        case Generation::SIX:
            if ((i = locations6.find(v)) != locations6.end())
            {
                return i->second;
            }
            break;
        case Generation::SEVEN:
            if ((i = locations7.find(v)) != locations7.end())
            {
                return i->second;
            }
            break;
        case Generation::LGPE:
            if ((i = locationsLGPE.find(v)) != locationsLGPE.end())
            {
                return i->second;
            }
            break;
        case Generation::EIGHT:
            if ((i = locations8.find(v)) != locations8.end())
            {
                return i->second;
            }
            break;
        case Generation::UNUSED:
            break;
    }
    return localize("INVALID_LOCATION");
}

const std::string& LanguageStrings::game(u8 v) const
{
    if (v < games.size() && games.at(v) != "")
    {
        return games.at(v);
    }
    return localize("INVALID_GAME");
}

const std::map<u16, std::string>& LanguageStrings::locations(Generation g) const
{
    static std::map<u16, std::string> emptyMap;
    switch (g)
    {
        case Generation::FOUR:
            return locations4;
        case Generation::FIVE:
            return locations5;
        case Generation::SIX:
            return locations6;
        case Generation::SEVEN:
            return locations7;
        case Generation::LGPE:
            return locationsLGPE;
        case Generation::EIGHT:
            return locations8;
        case Generation::UNUSED:
            return emptyMap;
    }
    return emptyMap;
}

size_t LanguageStrings::numGameStrings() const
{
    return games.size();
}

const std::map<u8, std::string>& LanguageStrings::rawCountries() const
{
    return countries;
}

const std::map<u8, std::string>& LanguageStrings::rawSubregions(u8 country) const
{
    static std::map<u8, std::string> emptyMap;
    auto i = subregions.find(country);
    if (i == subregions.end())
    {
        return emptyMap;
    }
    return i->second;
}
