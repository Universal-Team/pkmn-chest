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

#include "Bank.hpp"
#include <unistd.h>

#include "flashcard.hpp"
#include "langStrings.hpp"
#include "PK4.hpp"
#include "../fileBrowse.hpp"
#include "../graphics/graphics.hpp"

#define BANK(paths) paths.first
#define JSON(paths) paths.second
#define ARCHIVE Configuration::getInstance().useExtData() ? Archive::data() : Archive::sd()
#define OTHERARCHIVE Configuration::getInstance().useExtData() ? Archive::sd() : Archive::data()

std::string Bank::BANK_MAGIC = "PKSMBANK";

// TODO actually do stuff with the name
Bank::Bank(const std::string& name, int maxBoxes) : bankName(name) {
	load(maxBoxes);
	if(boxes() != maxBoxes) {
		resize(maxBoxes);
	}
}

void Bank::load(int maxBoxes) {
	if(data) {
		delete[] data;
		data = nullptr;
	}
	needsCheck = false;
	auto paths    = this->paths();
	bool needSave = false;
	FILE* in = fopen(BANK(paths).c_str(), "rb");
	if(in) {
		// Gui::waitFrame(i18n::localize("BANK_LOAD"));
		BankHeader h{"BAD_MGC", 0, 0};
		fseek(in, 0, SEEK_END);
		size = ftell(in);
		fseek(in, 0, SEEK_SET);
		fread(&h, 1, sizeof(BankHeader) - sizeof(int), in);
		if(memcmp(&h, BANK_MAGIC.data(), 8)) {
			// Gui::warn(i18n::localize("BANK_CORRUPT"));
			fclose(in);
			createBank(maxBoxes);
			needSave = true;
		} else {
			// NOTE: THIS IS THE CONVERSION SECTION. WILL NEED TO BE MODIFIED WHEN THE FORMAT IS CHANGED
			if(h.version == 1) {
				h.boxes  = (size - (sizeof(BankHeader) - sizeof(int))) / sizeof(BankEntry) / 30;
				maxBoxes = h.boxes;
				extern nlohmann::json g_banks;
				g_banks[bankName] = maxBoxes;
				data              = new u8[size = size + sizeof(int)];
				h.version         = BANK_VERSION;
				needSave          = true;
			}
			else {
				data = new u8[size];
				fread(&h.boxes, 1, sizeof(int), in);
			}
			std::copy((char*)&h, (char*)(&h + 1), data);
			fread(data + sizeof(BankHeader), 1, size - sizeof(BankHeader), in);
			fclose(in);
		}
	} else {
		// Gui::waitFrame(i18n::localize("BANK_CREATE"));
		fclose(in);
		createBank(maxBoxes);
		needSave = true;
	}

	in = fopen(JSON(paths).c_str(), "rb");
	if(in) {
		fseek(in, 0, SEEK_END);
		size_t jsonSize = ftell(in);
		fseek(in, 0, SEEK_SET);
		char jsonData[jsonSize + 1];
		fread(jsonData, 1, jsonSize, in);
		fclose(in);
		jsonData[jsonSize] = '\0';
		boxNames = nlohmann::json::parse(jsonData, nullptr, false);
		if(boxNames.is_discarded()) {
			createJSON();
			needSave = true;
		} else {
			for(int i = boxNames.size(); i < boxes(); i++) {
				// boxNames[i] = i18n::localize("STORAGE") + " " + std::to_string(i + 1);
				boxNames[i] = Lang::chest + " " + std::to_string(i + 1);
				if(!needSave) {
					needSave = true;
				}
			}
		}
	} else {
		fclose(in);
		boxNames = nlohmann::json::array();
		for(int i = 0; i < boxes(); i++) {
			// boxNames[i] = i18n::localize("STORAGE") + " " + std::to_string(i + 1);
			boxNames[i] = Lang::chest + " " + std::to_string(i + 1);
		}

		needSave = true;
	}

	if(boxes() != maxBoxes) {
		resize(maxBoxes);
	}

	if(needSave) {
		save();
	} else {
		sha256(prevHash.data(), data, size);
	}
}

bool Bank::save() const {
	// if(Configuration::getInstance().autoBackup())
	// {
	//     if(!backup() && !Gui::showChoiceMessage(i18n::localize("BACKUP_FAIL_SAVE_1"), i18n::localize("BACKUP_FAIL_SAVE_2")))
	//     {
	//         return false;
	//     }
	// }
	auto paths = this->paths();
	// Gui::waitFrame(i18n::localize("BANK_SAVE"));
	// FSUSER_DeleteFile(ARCHIVE, fsMakePath(PATH_UTF16, StringUtils::UTF8toUTF16(BANK(paths)).c_str()));
	FILE* out = fopen(BANK(paths).c_str(), "wb");
	if(out) {
		fwrite(data, 1, sizeof(BankHeader) + sizeof(BankEntry) * boxes() * 30, out);
		fclose(out);

		std::string jsonData = boxNames.dump(2);
		// FSUSER_DeleteFile(ARCHIVE, fsMakePath(PATH_UTF16, StringUtils::UTF8toUTF16(JSON(paths)).c_str()));
		out = fopen(JSON(paths).c_str(), "wb");
		if(out) {
			fwrite(jsonData.data(), 1, jsonData.size() + 1, out);
			sha256(prevHash.data(), data, sizeof(BankHeader) + sizeof(BankEntry) * boxes() * 30);
		} else {
			// Gui::error(i18n::localize("BANK_NAME_ERROR"), out.result());
		}
		fclose(out);
		return true;
	} else {
		// Gui::error(i18n::localize("BANK_SAVE_ERROR"), out.result());
		fclose(out);
		return false;
	}

	needsCheck = false;
}

void Bank::resize(size_t boxes) {
	size_t newSize = sizeof(BankHeader) + sizeof(BankEntry) * boxes * 30;
	auto paths     = this->paths();
	if(newSize != size) {
		// Gui::showResizeStorage();
		u8* newData = new u8[newSize];
		std::copy(data, data + std::min(newSize, size), newData);
		delete[] data;
		if(newSize > size) {
			std::fill_n(newData + size, newSize - size, 0xFF);
		}
		data = newData;

		// FSUSER_DeleteFile(ARCHIVE, fsMakePath(PATH_UTF16, StringUtils::UTF8toUTF16(BANK(paths)).c_str()));
		// FSUSER_DeleteFile(ARCHIVE, fsMakePath(PATH_UTF16, StringUtils::UTF8toUTF16(JSON(paths)).c_str()));

		((BankHeader*)data)->boxes = boxes;

		boxNames.clear();
		for(size_t i = boxNames.size(); i < boxes; i++) {
			// boxNames[i] = i18n::localize("STORAGE") + " " + std::to_string(i + 1);
			boxNames[i] = Lang::chest + " " + std::to_string(i + 1);
		}

		save();
	}
	size = newSize;
}

std::shared_ptr<PKX> Bank::pkm(int box, int slot) const {
	BankEntry* bank = (BankEntry*)(data + sizeof(BankHeader));
	int index       = box * 30 + slot;
	switch(bank[index].gen) {
		case Generation::FOUR:
			return std::make_shared<PK4>(bank[index].data, false, false);
		case Generation::FIVE:
			return std::make_shared<PK5>(bank[index].data, false, false);
		case Generation::SIX:
			// return std::make_shared<PK6>(bank[index].data, false, false);
		case Generation::SEVEN:
			// return std::make_shared<PK7>(bank[index].data, false, false);
		case Generation::LGPE:
			// return std::make_shared<PB7>(bank[index].data, false);
		case Generation::UNUSED:
		default:
			return std::make_shared<PK5>();
	}
}

void Bank::pkm(std::shared_ptr<PKX> pkm, int box, int slot) {
	BankEntry* bank = (BankEntry*)(data + sizeof(BankHeader));
	int index       = box * 30 + slot;
	BankEntry newEntry;
	if(pkm->species() == 0) {
		std::fill_n((char*)&newEntry, sizeof(BankEntry), 0xFF);
		bank[index] = newEntry;
		needsCheck  = true;
		return;
	}
	newEntry.gen = pkm->generation();
	std::copy(pkm->rawData(), pkm->rawData() + pkm->getLength(), newEntry.data);
	if(pkm->getLength() < 260) {
		std::fill_n(newEntry.data + pkm->getLength(), 260 - pkm->getLength(), 0xFF);
	}
	bank[index] = newEntry;
	needsCheck  = true;
}

bool Bank::backup() const {
	// Gui::waitFrame(i18n::localize("BANK_BACKUP"));
	auto paths = this->paths();
	// Archive::copyFile(Archive::sd(), "/_nds/pkmn-chest/backups/" + bankName + ".bnk.bak", Archive::sd(), "/_nds/pkmn-chest/backups/" + bankName + ".bnk.bak.old");
	// Archive::copyFile(Archive::sd(), "/_nds/pkmn-chest/backups/" + bankName + ".json.bak", Archive::sd(), "/_nds/pkmn-chest/backups/" + bankName + ".json.bak.old");
	const char *backupsPath = (sdFound() ? "sd:/_nds/pkmn-chest/backups/" : "fat:/_nds/pkmn-chest/backups/");
	if(fcopy(BANK(paths).c_str(), (backupsPath + bankName + ".bnk.bak").c_str())) {
		return false;
	}
	fcopy(JSON(paths).c_str(), (backupsPath + bankName + ".json.bak").c_str());
	return true;
}

std::string Bank::boxName(int box) const {
	return boxNames[box].get<std::string>();
}

void Bank::boxName(std::string name, int box) {
	boxNames[box] = name;
}

void Bank::createJSON() {
	boxNames = nlohmann::json::array();
	for(int i = 0; i < boxes(); i++) {
		// boxNames[i] = i18n::localize("STORAGE") + " " + std::to_string(i + 1);
		boxNames[i] = Lang::chest + " " + std::to_string(i + 1);
	}
}

void Bank::createBank(int maxBoxes) {
	if(data) {
		delete[] data;
	}
	data = new u8[size = sizeof(BankHeader) + sizeof(BankEntry) * maxBoxes * 30];
	std::copy(BANK_MAGIC.data(), BANK_MAGIC.data() + BANK_MAGIC.size(), data);
	*(int*)(data + 8)  = BANK_VERSION;
	*(int*)(data + 12) = maxBoxes;
	std::fill_n(data + sizeof(BankHeader), sizeof(BankEntry) * boxes() * 30, 0xFF);
}

bool Bank::hasChanged() const {
	if(!needsCheck) {
		return false;
	}
	u8 hash[SHA256_BLOCK_SIZE];
	sha256(hash, data, sizeof(BankHeader) + sizeof(BankEntry) * boxes() * 30);
	if(memcmp(hash, prevHash.data(), SHA256_BLOCK_SIZE)) {
		return true;
	}
	needsCheck = false;
	return false;
}

const std::string& Bank::name() const {
	return bankName;
}

int Bank::boxes() const {
	return ((BankHeader*)data)->boxes;
}

bool Bank::setName(const std::string& name) {
	auto oldPaths       = paths();
	std::string oldName = bankName;
	bankName            = name;
	auto newPaths       = paths();
	rename(BANK(oldPaths).c_str(), BANK(newPaths).c_str());
	// if(rename(BANK(oldPaths), BANK(newPaths) == [failed])
	// {
	//     bankName = oldName;
	//     return false;
	// }
	rename(JSON(oldPaths).c_str(), JSON(newPaths).c_str());
	// if(R_FAILED(Archive::moveFile(ARCHIVE, JSON(oldPaths), ARCHIVE, JSON(newPaths))))
	// {
		// bankName = oldName;
		// if(R_FAILED(Archive::moveFile(ARCHIVE, BANK(newPaths), ARCHIVE, BANK(oldPaths))))
		// {
		//     Gui::warn(i18n::localize("CRITICAL_BANK_ERROR_1"), i18n::localize("CRITICAL_BANK_ERROR_2"));
		//     return false;
		// }
		// return false;
	// }
	return true;
}

std::pair<std::string, std::string> Bank::paths() const {
	const char *banksPath = (sdFound() ? "sd:/_nds/pkmn-chest/banks/" : "fat:/_nds/pkmn-chest/banks/");
	return {banksPath + bankName + ".bnk", banksPath + bankName + ".json"};
}
