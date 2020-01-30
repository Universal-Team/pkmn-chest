#include "SavRS.hpp"

SavRS::SavRS(std::shared_ptr<u8[]> dt) : Sav3(dt)
{
    game = Game::RS;

    seenFlagOffsets = std::vector<int>({ PokeDex + 0x44, blockOfs[1] + 0x938, blockOfs[4] + 0xC0C });

    OFS_PCItem = blockOfs[1] + 0x0498;
    OFS_PouchHeldItem = blockOfs[1] + 0x0560;
    OFS_PouchKeyItem = blockOfs[1] + 0x05B0;
    OFS_PouchBalls = blockOfs[1] + 0x0600;
    OFS_PouchTMHM = blockOfs[1] + 0x0640;
    OFS_PouchBerry = blockOfs[1] + 0x0740;
    eventFlag = blockOfs[2] + 0x2A0;
    // EventConst = EventFlag + (EventFlagMax / 8);
    // DaycareOffset = blockOfs[4] + 0x11C;
}

std::map<Sav::Pouch, std::vector<int>> SavRS::validItems() const
{
    std::map<Sav::Pouch, std::vector<int>> items = {
        {NormalItem, {13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 63, 64, 65, 66, 67, 68, 69, 70, 71, 73, 74, 75, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 93, 94, 95, 96, 97, 98, 103, 104, 106, 107, 108, 109, 110, 111, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 254, 255, 256, 257, 258}},
        {Ball, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}},
        {KeyItem, {259, 260, 261, 262, 263, 264, 265, 266, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288}},
        {TM, {289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346}},
        {Berry, {133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175}},
        {PCItem, {}}
    };

    // PC can hold any item // TODO: Except maybe the orbs? gotta check PKHeX
    items[PCItem].insert(items[PCItem].end(), items[NormalItem].begin(), items[NormalItem].end());
    items[PCItem].insert(items[PCItem].end(), items[Ball].begin(), items[Ball].end());
    items[PCItem].insert(items[PCItem].end(), items[KeyItem].begin(), items[KeyItem].end());
    items[PCItem].insert(items[PCItem].end(), items[TM].begin(), items[TM].end());
    items[PCItem].insert(items[PCItem].end(), items[Berry].begin(), items[Berry].end());

    return items;
}
