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

#include "PK5.hpp"
#include "loader.hpp"
#include "random.hpp"

void PK5::shuffleArray(u8 sv) {
	static const int blockLength = 32;
	u8 index                     = sv * 4;

	u8 cdata[length];
	std::copy(data, data + length, cdata);

	for(u8 block = 0; block < 4; block++) {
		u8 ofs = blockPosition(index + block);
		std::copy(cdata + 8 + blockLength * ofs, cdata + 8 + blockLength * ofs + blockLength, data + 8 + blockLength * block);
	}
}

void PK5::crypt(void) {
	u32 seed = checksum();

	for(int i = 0x08; i < 136; i += 2) {
		seed = seedStep(seed);
		data[i] ^= (seed >> 16);
		data[i + 1] ^= (seed >> 24);
	}

	seed = PID();
	for(u32 i = 136; i < length; i += 2) {
		seed = seedStep(seed);
		data[i] ^= (seed >> 16);
		data[i + 1] ^= (seed >> 24);
	}
}

PK5::PK5(u8* dt, bool ekx, bool party) {
	length = party ? 220 : 136;
	data   = new u8[length];
	std::fill_n(data, length, 0);

	std::copy(dt, dt + length, data);
	if(ekx)
		decrypt();
}

std::shared_ptr<PKX> PK5::clone(void) {
	return std::make_shared<PK5>(data, false, length == 236);
}

Generation PK5::generation(void) const {
	return Generation::FIVE;
}

u32 PK5::encryptionConstant(void) const {
	return PID();
}
void PK5::encryptionConstant(u32 v) {
	(void)v;
}

u8 PK5::currentFriendship(void) const {
	return otFriendship();
}
void PK5::currentFriendship(u8 v) {
	otFriendship(v);
}

u8 PK5::currentHandler(void) const {
	return 0;
}
void PK5::currentHandler(u8 v) {
	(void)v;
}

u8 PK5::abilityNumber(void) const {
	return hiddenAbility() ? 4 : 1 << ((PID() >> 16) & 1);
}
void PK5::abilityNumber(u8 v) {
	if(shiny()) {
		do {
			PID(PKX::getRandomPID(species(), gender(), version(), nature(), alternativeForm(), v, PID(), generation()));
		} while(!shiny());
	} else {
		do {
			PID(PKX::getRandomPID(species(), gender(), version(), nature(), alternativeForm(), v, PID(), generation()));
		} while(shiny());
	}
}

u32 PK5::PID(void) const {
	return *(u32*)(data);
}
void PK5::PID(u32 v) {
	*(u32*)(data) = v;
}

u16 PK5::sanity(void) const {
	return *(u16*)(data + 0x04);
}
void PK5::sanity(u16 v) {
	*(u16*)(data + 0x04) = v;
}

u16 PK5::checksum(void) const {
	return *(u16*)(data + 0x06);
}
void PK5::checksum(u16 v) {
	*(u16*)(data + 0x06) = v;
}

u16 PK5::species(void) const {
	return *(u16*)(data + 0x08);
}
void PK5::species(u16 v) {
	*(u16*)(data + 0x08) = v;
}

u16 PK5::heldItem(void) const {
	return *(u16*)(data + 0x0A);
}
void PK5::heldItem(u16 v) {
	*(u16*)(data + 0x0A) = v;
}

u16 PK5::TID(void) const {
	return *(u16*)(data + 0x0C);
}
void PK5::TID(u16 v) {
	*(u16*)(data + 0x0C) = v;
}

u16 PK5::SID(void) const {
	return *(u16*)(data + 0x0E);
}
void PK5::SID(u16 v) {
	*(u16*)(data + 0x0E) = v;
}

u32 PK5::experience(void) const {
	return *(u32*)(data + 0x10);
}
void PK5::experience(u32 v) {
	*(u32*)(data + 0x10) = v;
}

u8 PK5::otFriendship(void) const {
	return data[0x14];
}
void PK5::otFriendship(u8 v) {
	data[0x14] = v;
}

u8 PK5::ability(void) const {
	return data[0x15];
}
void PK5::ability(u8 v) {
	data[0x15] = v;
}

void PK5::setAbility(u8 v) {
	u8 abilitynum;

	if(v == 0)
		abilitynum = 1;
	else if(v == 1)
		abilitynum = 2;
	else
		abilitynum = 4;

	abilityNumber(abilitynum);
	ability(abilities(v));
}

u16 PK5::markValue(void) const {
	return data[0x16];
}
void PK5::markValue(u16 v) {
	data[0x16] = v;
}

u8 PK5::language(void) const {
	return data[0x17];
}
void PK5::language(u8 v) {
	data[0x17] = v;
}

u8 PK5::ev(u8 ev) const {
	return data[0x18 + ev];
}
void PK5::ev(u8 ev, u8 v) {
	data[0x18 + ev] = v;
}

u8 PK5::contest(u8 contest) const {
	return data[0x1E + contest];
}
void PK5::contest(u8 contest, u8 v) {
	data[0x1E + contest] = v;
}

bool PK5::ribbon(u8 ribcat, u8 ribnum) const {
	static u8 ribIndex[12] = {0x24, 0x25, 0x26, 0x27, 0x3C, 0x3D, 0x3E, 0x3F, 0x60, 0x61, 0x62, 0x63};
	return (data[ribIndex[ribcat]] & (1 << ribnum)) == 1 << ribnum;
}

void PK5::ribbon(u8 ribcat, u8 ribnum, u8 v) {
	static u8 ribIndex[12] = {0x24, 0x25, 0x26, 0x27, 0x3C, 0x3D, 0x3E, 0x3F, 0x60, 0x61, 0x62, 0x63};
	data[ribIndex[ribcat]] = (u8)((data[ribIndex[ribcat]] & ~(1 << ribnum)) | (v ? 1 << ribnum : 0));
}

u16 PK5::move(u8 m) const {
	return *(u16*)(data + 0x28 + m * 2);
}
void PK5::move(u8 m, u16 v) {
	*(u16*)(data + 0x28 + m * 2) = v;
}

u8 PK5::PP(u8 m) const {
	return data[0x30 + m];
}
void PK5::PP(u8 m, u8 v) {
	data[0x30 + m] = v;
}

u8 PK5::PPUp(u8 m) const {
	return data[0x34 + m];
}
void PK5::PPUp(u8 m, u8 v) {
	data[0x34 + m] = v;
}

u8 PK5::iv(u8 stat) const {
	u32 buffer = *(u32*)(data + 0x38);
	return (u8)((buffer >> 5 * stat) & 0x1F);
}

void PK5::iv(u8 stat, u8 v) {
	u32 buffer = *(u32*)(data + 0x38);
	buffer &= ~(0x1F << 5 * stat);
	buffer |= v << (5 * stat);
	*(u32*)(data + 0x38) = buffer;
}

bool PK5::egg(void) const {
	return ((*(u32*)(data + 0x38) >> 30) & 0x1) == 1;
}
void PK5::egg(bool v) {
	*(u32*)(data + 0x38) = (u32)((*(u32*)(data + 0x38) & ~0x40000000) | (u32)(v ? 0x40000000 : 0));
}

bool PK5::nicknamed(void) const {
	return ((*(u32*)(data + 0x38) >> 31) & 0x1) == 1;
}
void PK5::nicknamed(bool v) {
	*(u32*)(data + 0x38) = (*(u32*)(data + 0x38) & 0x7FFFFFFF) | (v ? 0x80000000 : 0);
}

bool PK5::fatefulEncounter(void) const {
	return (data[0x40] & 1) == 1;
}
void PK5::fatefulEncounter(bool v) {
	data[0x40] = (u8)((data[0x40] & ~0x01) | (v ? 1 : 0));
}

u8 PK5::gender(void) const {
	return (data[0x40] >> 1) & 0x3;
}
void PK5::gender(u8 g) {
	data[0x40] = u8((data[0x40] & ~0x06) | (g << 1));
	if(shiny()) {
		do {
			PID(PKX::getRandomPID(species(), g, version(), nature(), alternativeForm(), abilityNumber(), PID(), generation()));
		} while(!shiny());
	} else {
		do {
			PID(PKX::getRandomPID(species(), g, version(), nature(), alternativeForm(), abilityNumber(), PID(), generation()));
		} while(shiny());
	}
}

u8 PK5::alternativeForm(void) const {
	return data[0x40] >> 3;
}
void PK5::alternativeForm(u8 v) {
	data[0x40] = u8((data[0x40] & 0x07) | (v << 3));
}

u8 PK5::nature(void) const {
	return data[0x41];
}
void PK5::nature(u8 v) {
	data[0x41] = v;
}

bool PK5::hiddenAbility(void) const {
	return (data[0x42] & 1) == 1;
}
void PK5::hiddenAbility(bool v) {
	data[0x42] = (u8)((data[0x42] & ~0x01) | (v ? 1 : 0));
}

bool PK5::nPokemon(void) const {
	return (data[0x42] & 2) == 2;
}
void PK5::nPokemon(bool v) {
	data[0x42] = (u8)((data[0x42] & ~0x02) | (v ? 2 : 0));
}

std::string PK5::nickname(void) const {
	return StringUtils::getString(data, 0x48, 11, u'\uFFFF');
}
void PK5::nickname(const std::string& v) {
	StringUtils::setString(data, v, 0x48, 11, u'\uFFFF', 0);
}

u8 PK5::version(void) const {
	return data[0x5F];
}
void PK5::version(u8 v) {
	data[0x5F] = v;
}

std::string PK5::otName(void) const {
	return StringUtils::getString(data, 0x68, 8, u'\uFFFF');
}
void PK5::otName(const std::string& v) {
	StringUtils::setString(data, v, 0x68, 8, u'\uFFFF', 0);
}

u8 PK5::eggYear(void) const {
	return data[0x78];
}
void PK5::eggYear(u8 v) {
	data[0x78] = v;
}

u8 PK5::eggMonth(void) const {
	return data[0x79];
}
void PK5::eggMonth(u8 v) {
	data[0x79] = v;
}

u8 PK5::eggDay(void) const {
	return data[0x7A];
}
void PK5::eggDay(u8 v) {
	data[0x7A] = v;
}

u8 PK5::metYear(void) const {
	return data[0x7B];
}
void PK5::metYear(u8 v) {
	data[0x7B] = v;
}

u8 PK5::metMonth(void) const {
	return data[0x7C];
}
void PK5::metMonth(u8 v) {
	data[0x7C] = v;
}

u8 PK5::metDay(void) const {
	return data[0x7D];
}
void PK5::metDay(u8 v) {
	data[0x7D] = v;
}

u16 PK5::eggLocation(void) const {
	return *(u16*)(data + 0x7E);
}
void PK5::eggLocation(u16 v) {
	*(u16*)(data + 0x7E) = v;
}

u16 PK5::metLocation(void) const {
	return *(u16*)(data + 0x80);
}
void PK5::metLocation(u16 v) {
	*(u16*)(data + 0x80) = v;
}

u8 PK5::pkrs(void) const {
	return data[0x82];
}
void PK5::pkrs(u8 v) {
	data[0x82] = v;
}

u8 PK5::pkrsDays(void) const {
	return data[0x82] & 0xF;
};
void PK5::pkrsDays(u8 v) {
	data[0x82] = (u8)((data[0x82] & ~0xF) | v);
}

u8 PK5::pkrsStrain(void) const {
	return data[0x82] >> 4;
};
void PK5::pkrsStrain(u8 v) {
	data[0x82] = (u8)((data[0x82] & 0xF) | v << 4);
}

u8 PK5::ball(void) const {
	return data[0x83];
}
void PK5::ball(u8 v) {
	data[0x83] = v;
}

u8 PK5::metLevel(void) const {
	return data[0x84] & ~0x80;
}
void PK5::metLevel(u8 v) {
	data[0x84] = (data[0x84] & 0x80) | v;
}

u8 PK5::otGender(void) const {
	return data[0x84] >> 7;
}
void PK5::otGender(u8 v) {
	data[0x84] = (data[0x84] & ~0x80) | (v << 7);
}

u8 PK5::encounterType(void) const {
	return data[0x85];
}
void PK5::encounterType(u8 v) {
	data[0x85] = v;
}

void PK5::refreshChecksum(void) {
	u16 chk = 0;
	for(u8 i = 8; i < 136; i += 2) {
		chk += *(u16*)(data + i);
	}
	checksum(chk);
}

u8 PK5::hpType(void) const {
	return 15 * ((iv(0) & 1) + 2 * (iv(1) & 1) + 4 * (iv(2) & 1) + 8 * (iv(3) & 1) + 16 * (iv(4) & 1) + 32 * (iv(5) & 1)) / 63;
}
void PK5::hpType(u8 v) {
	static constexpr u16 hpivs[16][6] = {
		{1, 1, 0, 0, 0, 0}, // Fighting
		{0, 0, 0, 1, 0, 0}, // Flying
		{1, 1, 0, 1, 0, 0}, // Poison
		{1, 1, 1, 1, 0, 0}, // Ground
		{1, 1, 0, 0, 1, 0}, // Rock
		{1, 0, 0, 1, 1, 0}, // Bug
		{1, 0, 1, 1, 1, 0}, // Ghost
		{1, 1, 1, 1, 1, 0}, // Steel
		{1, 0, 1, 0, 0, 1}, // Fire
		{1, 0, 0, 1, 0, 1}, // Water
		{1, 0, 1, 1, 0, 1}, // Grass
		{1, 1, 1, 1, 0, 1}, // Electric
		{1, 0, 1, 0, 1, 1}, // Psychic
		{1, 0, 0, 1, 1, 1}, // Ice
		{1, 0, 1, 1, 1, 1}, // Dragon
		{1, 1, 1, 1, 1, 1}, // Dark
	};

	for(u8 i = 0; i < 6; i++) {
		iv(i, (iv(i) & 0x1E) + hpivs[v][i]);
	}
}

u16 PK5::TSV(void) const {
	return (TID() ^ SID()) >> 3;
}
u16 PK5::PSV(void) const {
	return ((PID() >> 16) ^ (PID() & 0xFFFF)) >> 3;
}

u8 PK5::level(void) const {
	u8 i      = 1;
	u8 xpType = expType();
	while(experience() >= expTable(i, xpType) && ++i < 100);
	return i;
}

void PK5::level(u8 v) {
	experience(expTable(v - 1, expType()));
}

bool PK5::shiny(void) const {
	return TSV() == PSV();
}
void PK5::shiny(bool v) {
	if(v) {
		while(!shiny()) {
			PID(PKX::getRandomPID(species(), gender(), version(), nature(), alternativeForm(), abilityNumber(), PID(), generation()));
		}
	} else {
		while(shiny()) {
			PID(PKX::getRandomPID(species(), gender(), version(), nature(), alternativeForm(), abilityNumber(), PID(), generation()));
		}
	}
}

u16 PK5::formSpecies(void) const {
	u16 tmpSpecies = species();
	u8 form        = alternativeForm();
	u8 formcount   = PersonalBWB2W2::formCount(tmpSpecies);

	if(form && form < formcount) {
		u16 backSpecies = tmpSpecies;
		tmpSpecies      = PersonalBWB2W2::formStatIndex(tmpSpecies);
		if(!tmpSpecies) {
			tmpSpecies = backSpecies;
		} else if(form < formcount) {
			tmpSpecies += form - 1;
		}
	}

	return tmpSpecies;
}

u16 PK5::stat(const u8 stat) const {
	u16 calc;
	u8 mult = 10, basestat = 0;

	if(stat == 0)
		basestat = baseHP();
	else if(stat == 1)
		basestat = baseAtk();
	else if(stat == 2)
		basestat = baseDef();
	else if(stat == 3)
		basestat = baseSpe();
	else if(stat == 4)
		basestat = baseSpa();
	else if(stat == 5)
		basestat = baseSpd();

	if(stat == 0)
		calc = 10 + (2 * basestat + iv(stat) + ev(stat) / 4 + 100) * level() / 100;
	else
		calc = 5 + (2 * basestat + iv(stat) + ev(stat) / 4) * level() / 100;
	if(nature() / 5 + 1 == stat)
		mult++;
	if(nature() % 5 + 1 == stat)
		mult--;
	return calc * mult / 10;
}

std::shared_ptr<PKX> PK5::previous(void) const {
	u8 dt[136];
	std::copy(data, data + 136, dt);

	// Clear nature field
	dt[0x41] = 0;

	std::shared_ptr<PKX> pk4 = std::make_shared<PK4>(dt);

	// Force normal Arceus form
	if(pk4->species() == 493) {
		pk4->alternativeForm(0);
	}

	pk4->nickname(nickname());
	pk4->otName(otName());
	pk4->heldItem(0);
	pk4->otFriendship(70);
	pk4->ball(ball());
	// met location ???
	for(int i = 0; i < 4; i++) {
		if(pk4->move(i) > save->maxMove()) {
			pk4->move(i, 0);
		}
	}
	pk4->fixMoves();

	pk4->refreshChecksum();
	return pk4;
}

int PK5::partyCurrHP(void) const {
	if(length == 136) {
		return -1;
	}
	return *(u16*)(data + 0x8E);
}

void PK5::partyCurrHP(u16 v) {
	if(length != 136) {
		*(u16*)(data + 0x8E) = v;
	}
}

int PK5::partyStat(const u8 stat) const {
	if(length == 136) {
		return -1;
	}
	return *(u16*)(data + 0x90 + stat * 2);
}

void PK5::partyStat(const u8 stat, u16 v) {
	if(length != 136) {
		*(u16*)(data + 0x90 + stat * 2) = v;
	}
}

int PK5::partyLevel() const {
	if(length == 136) {
		return -1;
	}
	return *(data + 0x8C);
}

void PK5::partyLevel(u8 v) {
	if(length != 136) {
		*(data + 0x8C) = v;
	}
}
