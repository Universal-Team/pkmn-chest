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

#ifndef BANK_HPP
#define BANK_HPP

#include "json.hpp"
#include "Sav.hpp"

extern "C" {
#include "sha256.h"
}

class Bank {
public:
	Bank(const std::string& name, int maxBoxes);
	~Bank() { delete[] data; }
	std::shared_ptr<PKX> pkm(int box, int slot) const;
	void pkm(std::shared_ptr<PKX> pkm, int box, int slot);
	void resize(size_t boxes);
	void load(int maxBoxes);
	bool save() const;
	bool backup() const;
	std::string boxName(int box) const;
	std::pair<std::string, std::string> paths() const;
	void boxName(std::string name, int box);
	bool hasChanged() const;
	int boxes() const;
	const std::string& name() const;
	bool setName(const std::string& name);

private:
	static constexpr int BANK_VERSION            = 2;
	static std::string BANK_MAGIC;
	void createJSON();
	void createBank(int maxBoxes);
	void convert();
	struct BankHeader {
		const char MAGIC[8];
		int version;
		int boxes;
	};
	struct BankEntry {
		Generation gen;
		u8 data[260];
	};
	u8* data = nullptr;
	nlohmann::json boxNames;
	size_t size;
	mutable std::array<u8, SHA256_BLOCK_SIZE> prevHash;
	mutable bool needsCheck = false;
	std::string bankName;
};

#endif
