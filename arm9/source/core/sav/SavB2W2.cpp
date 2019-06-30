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

#include "SavB2W2.hpp"

static constexpr u16 lengths[74] = {0x03e0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0,
		0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x0ff0, 0x09ec, 0x0534, 0x00b0, 0x00a8, 0x1338,
		0x07c4, 0x0d54, 0x0094, 0x0658, 0x0a94, 0x01ac, 0x03ec, 0x005c, 0x01e0, 0x00a8, 0x0460, 0x1400, 0x02a4, 0x00e0, 0x034c, 0x04e0, 0x00f8,
		0x02fc, 0x0094, 0x035c, 0x01d4, 0x01e0, 0x00f0, 0x01b4, 0x04dc, 0x0034, 0x003c, 0x01ac, 0x0b90, 0x00ac, 0x0850, 0x0284, 0x0010, 0x00a8,
		0x016c, 0x0080, 0x00fc, 0x16a8, 0x0498, 0x0060, 0x00fc, 0x03e4, 0x00f0, 0x0094};

	static constexpr u32 chkofs[74] = {0x003E2, 0x013F2, 0x023F2, 0x033F2, 0x043F2, 0x053F2, 0x063F2, 0x073F2, 0x083F2, 0x093F2, 0x0A3F2, 0x0B3F2,
		0x0C3F2, 0x0D3F2, 0x0E3F2, 0x0F3F2, 0x103F2, 0x113F2, 0x123F2, 0x133F2, 0x143F2, 0x153F2, 0x163F2, 0x173F2, 0x183F2, 0x18DEE, 0x19336,
		0x194B2, 0x195AA, 0x1A93A, 0x1B1C6, 0x1BF56, 0x1C096, 0x1C75A, 0x1D296, 0x1D4AE, 0x1D8EE, 0x1D95E, 0x1DBE2, 0x1DCAA, 0x1E162, 0x1F602,
		0x1F9A6, 0x1FAE2, 0x1FE4E, 0x203E2, 0x204FA, 0x207FE, 0x20896, 0x20C5E, 0x20ED6, 0x210E2, 0x211F2, 0x213B6, 0x218DE, 0x21936, 0x21A3E,
		0x21CAE, 0x22892, 0x229AE, 0x23252, 0x23586, 0x23612, 0x237AA, 0x2396E, 0x23A82, 0x23BFE, 0x252AA, 0x2579A, 0x25862, 0x259FE, 0x25DE6,
		0x25EF2, 0x25FA2};

	static constexpr u32 blockOfs[74] = {0x00000, 0x00400, 0x01400, 0x02400, 0x03400, 0x04400, 0x05400, 0x06400, 0x07400, 0x08400, 0x09400, 0x0A400,
		0x0B400, 0x0C400, 0x0D400, 0x0E400, 0x0F400, 0x10400, 0x11400, 0x12400, 0x13400, 0x14400, 0x15400, 0x16400, 0x17400, 0x18400, 0x18E00,
		0x19400, 0x19500, 0x19600, 0x1AA00, 0x1B200, 0x1C000, 0x1C100, 0x1C800, 0x1D300, 0x1D500, 0x1D900, 0x1DA00, 0x1DC00, 0x1DD00, 0x1E200,
		0x1F700, 0x1FA00, 0x1FB00, 0x1FF00, 0x20400, 0x20500, 0x20800, 0x20900, 0x20D00, 0x20F00, 0x21100, 0x21200, 0x21400, 0x21900, 0x21A00,
		0x21B00, 0x21D00, 0x22900, 0x22A00, 0x23300, 0x23600, 0x23700, 0x23800, 0x23A00, 0x23B00, 0x23C00, 0x25300, 0x25800, 0x25900, 0x25A00,
		0x25E00, 0x25F00};

	static constexpr u32 chkMirror[74] = {0x25F00, 0x25F02, 0x25F04, 0x25F06, 0x25F08, 0x25F0A, 0x25F0C, 0x25F0E, 0x25F10, 0x25F12, 0x25F14, 0x25F16,
		0x25F18, 0x25F1A, 0x25F1C, 0x25F1E, 0x25F20, 0x25F22, 0x25F24, 0x25F26, 0x25F28, 0x25F2A, 0x25F2C, 0x25F2E, 0x25F30, 0x25F32, 0x25F34,
		0x25F36, 0x25F38, 0x25F3A, 0x25F3C, 0x25F3E, 0x25F40, 0x25F42, 0x25F44, 0x25F46, 0x25F48, 0x25F4A, 0x25F4C, 0x25F4E, 0x25F50, 0x25F52,
		0x25F54, 0x25F56, 0x25F58, 0x25F5A, 0x25F5C, 0x25F5E, 0x25F60, 0x25F62, 0x25F64, 0x25F66, 0x25F68, 0x25F6A, 0x25F6C, 0x25F6E, 0x25F70,
		0x25F72, 0x25F74, 0x25F76, 0x25F78, 0x25F7A, 0x25F7C, 0x25F7E, 0x25F80, 0x25F82, 0x25F84, 0x25F86, 0x25F88, 0x25F8A, 0x25F8C, 0x25F8E,
		0x25F90, 0x25FA2};

SavB2W2::SavB2W2(u8* dt) {
	length = 0x26000;
	boxes  = 24;
	game   = Game::B2W2;

	data = new u8[length];
	std::copy(dt, dt + length, data);

	PCLayout             = 0x0;
	Trainer1             = 0x19400;
	Trainer2             = 0x21100;
	BattleSubway         = 0x21B00;
	Party                = 0x18E00;
	PokeDex              = 0x21400;
	PokeDexLanguageFlags = 0x21728;
	WondercardFlags      = 0x1C800;
	WondercardData       = 0x1C900;
	PouchHeldItem        = 0x18400;
	PouchKeyItem         = 0x188D8;
	PouchTMHM            = 0x18A24;
	PouchMedicine        = 0x18BD8;
	PouchBerry           = 0x18C98;
	Box                  = 0x400;
}

SavB2W2::~SavB2W2() {}

void SavB2W2::resign(void) {
	const u8 blockCount = 74;
	u8* tmp             = new u8[*std::max_element(lengths, lengths + blockCount)];
	u16 cs;

	for(u8 i = 0; i < blockCount; i++) {
		std::copy(data + blockOfs[i], data + blockOfs[i] + lengths[i], tmp);
		cs                           = ccitt16(tmp, lengths[i]);
		*(u16*)(data + chkMirror[i]) = cs;
		*(u16*)(data + chkofs[i])    = cs;
	}

	delete[] tmp;
}

std::map<Pouch, std::vector<int>> SavB2W2::validItems() const {
	return { {NormalItem, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
						 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
						 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 116, 117, 118, 119, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145,
						 146, 147, 148, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234,
						 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259,
						 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284,
						 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309,
						 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 492, 493, 494, 495, 496, 497, 498,
						 499, 500, 537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559,
						 560, 561, 562, 563, 564, 571, 572, 573, 575, 576, 577, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590}}, {KeyItem, {437, 442, 447, 450, 453, 458, 465, 466, 471, 504, 578, 616, 617, 621, 626, 627, 628, 629, 630, 631, 632, 633, 634, 635, 636, 637, 638}}, {TM, {328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354,
				 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380,
				 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406,
				 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 618, 619, 620, 420, 421, 422, 423, 424, 425}}, {Medicine, {17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
					   49, 50, 51, 52, 53, 54, 134, 504, 565, 566, 567, 568, 569, 570, 591}}, {Berry, {149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174,
					175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
					201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212}}};
}