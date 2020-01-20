#ifndef SAV3_HPP
#define SAV3_HPP

#include "Sav.hpp"
#include "personal.hpp"

class Sav3 : public Sav
{
protected:
    bool japanese;
    Game game;
    int maxSpecies(void) const override { return 386; } // Index 412
    int maxMove(void) const override { return 354; }
    int maxItem(void) const override { return 374; }
    int maxAbility(void) const override { return 77; }
    int maxBall(void) const override { return 0xC; }
    int OFS_PCItem, OFS_PouchHeldItem, OFS_PouchKeyItem, OFS_PouchBalls, OFS_PouchTMHM, OFS_PouchBerry, eventFlag;

    mutable std::set<int> items, moves, species, abilities, balls;

    int *blockOrder, *blockOfs;
    std::vector<int> seenFlagOffsets;

    void initialize();

    void loadBlocks();
    int *getBlockOrder(int ofs);
    int getActiveSaveIndex(int *BlockOrder1, int *BlockOrder2);
    Game getVersion();
    const int SIZE_BLOCK = 0x1000;
    const int BLOCK_COUNT = 14;
    const int SIZE_RESERVED = 0x10000; // unpacked box data will start after the save data
    // const int SIZE_BLOCK_USED = 0xF80;

    const u16 chunkLength[14] =
    {
        0xf2c, // 0 | Small Block (Trainer Info)
        0xf80, // 1 | Large Block Part 1
        0xf80, // 2 | Large Block Part 2
        0xf80, // 3 | Large Block Part 3
        0xf08, // 4 | Large Block Part 4
        0xf80, // 5 | PC Block 0
        0xf80, // 6 | PC Block 1
        0xf80, // 7 | PC Block 2
        0xf80, // 8 | PC Block 3
        0xf80, // 9 | PC Block 4
        0xf80, // A | PC Block 5
        0xf80, // B | PC Block 6
        0xf80, // C | PC Block 7
        0x7d0  // D | PC Block 8
    };

    const unsigned int SIZE_STORED = 80;
    const unsigned int SIZE_PARTY = 100;

    int ABO() { return activeSAV * SIZE_BLOCK * 0xE; };

    int activeSAV;

    u32 securityKey() const;

    bool getEventFlag(int flagNumber) const;
    void setEventFlag(int flagNumber, bool value);

    bool canSetDex(int species);

    u32 dexPIDUnown(void);
    void dexPIDUnown(u32 v);
    u32 dexPIDSpinda(void);
    void dexPIDSpinda(u32 v);

    bool getCaught(int species) const;
    void setCaught(int species, bool caught);
    bool getSeen(int species) const;
    void setSeen(int species, bool seen);


    std::vector<int> validNormalItems = {13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 63, 64, 65, 66, 67, 68, 69, 70, 71, 73, 74, 75, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 93, 94, 95, 96, 97, 98, 103, 104, 106, 107, 108, 109, 110, 111, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 254, 255, 256, 257, 258},
                     validBalls = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
                     validKeyItemsRS = {259, 260, 261, 262, 263, 264, 265, 266, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288},
                     validKeyItemsFRLG = {349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374},
                     validKeyItemsE = {375, 376},
                     validTMs = {289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346},
                     validBerries = {133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175};

public:
    Sav3(std::shared_ptr<u8[]> data, u32 length) : Sav(data, length) {}
    Sav3(std::shared_ptr<u8[]> data);
    virtual ~Sav3() {}
    void resign(void);
    void encrypt(void) override { resign(); }
    void decrypt(void) override {}

    u16 TID(void) const override;
    void TID(u16 v) override;
    u16 SID(void) const override;
    void SID(u16 v) override;
    u8 version(void) const override;
    void version(u8 v) override;
    u8 gender(void) const override;
    void gender(u8 v) override;
    u8 subRegion(void) const override;
    void subRegion(u8 v) override;
    u8 country(void) const override;
    void country(u8 v) override;
    u8 consoleRegion(void) const override;
    void consoleRegion(u8 v) override;
    Language language(void) const override;
    void language(Language v) override;
    std::string otName(void) const override;
    void otName(const std::string& v) override;
    u32 money(void) const override;
    void money(u32 v) override;
    u32 BP(void) const override;
    void BP(u32 v) override;
    u8 badges(void) const override;
    u16 playedHours(void) const override;
    void playedHours(u16 v) override;
    u8 playedMinutes(void) const override;
    void playedMinutes(u8 v) override;
    u8 playedSeconds(void) const override;
    void playedSeconds(u8 v) override;

    u8 currentBox(void) const override;
    void currentBox(u8 v) override;
    u32 boxOffset(u8 box, u8 slot) const override;
    u32 partyOffset(u8 slot) const override;

    std::shared_ptr<PKX> pkm(u8 slot) const override;
    std::shared_ptr<PKX> pkm(u8 box, u8 slot) const override;

    // NOTICE: this sets a pkx into the savefile, not a ekx
    // that's because PKSM works with decrypted boxes and
    // crypts them back during resigning
    void pkm(std::shared_ptr<PKX> pk, u8 slot) override;
    void pkm(std::shared_ptr<PKX> pk, u8 box, u8 slot, bool applyTrade) override;

    void trade(std::shared_ptr<PKX> pk) override;
    std::shared_ptr<PKX> emptyPkm() const override;

    void dex(std::shared_ptr<PKX> pk) override;
    int dexSeen(void) const override;
    int dexCaught(void) const override;
    int emptyGiftLocation(void) const override;
    std::vector<Sav::giftData> currentGifts(void) const override;
    void mysteryGift(WCX& wc, int& pos) override;
    std::unique_ptr<WCX> mysteryGift(int pos) const override;
    void cryptBoxData(bool crypted) override;
    std::string boxName(u8 box) const override;
    void boxName(u8 box, const std::string& name) override;
    u8 boxWallpaper(u8 box) const override;
    void boxWallpaper(u8 box, u8 v) override;
    u8 partyCount(void) const override;
    void partyCount(u8 count) override;

    int maxBoxes(void) const override { return 14; }
    size_t maxWondercards(void) const override { return 8; } //TODO
    Generation generation(void) const override { return Generation::THREE; }
    const std::set<int>& availableItems(void) const override;
    const std::set<int>& availableMoves(void) const override;
    const std::set<int>& availableSpecies(void) const override;
    const std::set<int>& availableAbilities(void) const override;
    const std::set<int>& availableBalls(void) const override;

    void item(const Item& item, Pouch pouch, u16 slot) override;
    std::unique_ptr<Item> item(Pouch pouch, u16 slot) const override;
    std::vector<std::pair<Pouch, int>> pouches(void) const override;
    std::map<Pouch, std::vector<int>> validItems(void) const override;
    std::string pouchName(Pouch pouch) const override;

    u8 formCount(u16 species) const override { return 0; } // TODO: Do this somehow?
};

#endif