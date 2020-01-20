#include "Sav3.hpp"
#include "endian.hpp"
#include "flagUtil.hpp"
#include "lang.hpp"
#include "PK3.hpp"
#include "utils.hpp"

void Sav3::loadBlocks()
{
    int *o1 = getBlockOrder(0);
    if (length > 0x10000)
    {
        int *o2 = getBlockOrder(0xE000);
        activeSAV = getActiveSaveIndex(o1, o2);
        blockOrder = activeSAV == 0 ? o1 : o2;
    }
    else
    {
        activeSAV = 0;
        blockOrder = o1;
    }

    blockOfs = new int[BLOCK_COUNT];
    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        int index = blockOrder[i];
        blockOfs[i] = index < 0 ? -1 /*was int.MinValue*/ : (index * SIZE_BLOCK) + ABO();
    }
}

int *Sav3::getBlockOrder(int ofs)
{
    int *order = new int[BLOCK_COUNT];
    for (int i = 0; i < BLOCK_COUNT; i++)
        order[i] = Endian::convertTo<s16>(&data[ofs + (i * SIZE_BLOCK) + 0xFF4]);
    return order;
}

int Sav3::getActiveSaveIndex(int *BlockOrder1, int *BlockOrder2)
{
    int zeroBlock1 = BlockOrder1[0];
    int zeroBlock2 = BlockOrder2[0];
    if (zeroBlock2 < 0)
        return 0;
    if (zeroBlock1 < 0)
        return 1;
    u32 count1 = Endian::convertTo<u32>(&data[(zeroBlock1 * SIZE_BLOCK) + 0x0FFC]);
    u32 count2 = Endian::convertTo<u32>(&data[(zeroBlock2 * SIZE_BLOCK) + 0xEFFC]);
    return count1 > count2 ? 0 : 1;
}

Game Sav3::getVersion()
{
    u32 gameCode = Endian::convertTo<u32>(&data[blockOfs[0] + 0xAC]);
    switch (gameCode)
    {
        case 1: return Game::FRLG; // fixed value
        case 0: return Game::RS; // no battle tower record data
        // case uint.MaxValue: return GameVersion.Unknown; // what a hack // Is this needed?
        default:
            // Ruby doesn't set data as far down as Emerald.
            // 00 FF 00 00 00 00 00 00 00 FF 00 00 00 00 00 00
            // ^ byte pattern in Emerald saves, is all zero in Ruby/Sapphire as far as I can tell.
            // Some saves have had data @ 0x550
            if (Endian::convertTo<u64>(&data[blockOfs[0] + 0xEE0]) != 0)
                return Game::E;
            if (Endian::convertTo<u64>(&data[blockOfs[0] + 0xEE8]) != 0)
                return Game::E;
            return Game::RS;
    }
}

Sav3::Sav3(std::shared_ptr<u8[]> dt) : Sav3(dt, 0x20000)
{
    loadBlocks();

    // Japanese games are limited to 5 character OT names; any unused characters are 0xFF.
    // 5 for JP, 7 for INT. There's always 1 terminator, thus we can check 0x6-0x7 being 0xFFFF = INT
    // OT name is stored at the top of the first block.
    japanese = Endian::convertTo<s16>(&data[blockOfs[0]+0x6]) == 0;

    game = getVersion();

    PokeDex = blockOfs[0] + 0x18;

    switch (game)
    {
        case Game::RS:
            seenFlagOffsets = std::vector<int>({ PokeDex + 0x44, blockOfs[1] + 0x938, blockOfs[4] + 0xC0C });
            break;
        case Game::E:
            seenFlagOffsets = std::vector<int>({ PokeDex + 0x44, blockOfs[1] + 0x988, blockOfs[4] + 0xCA4 });
            break;
        default:
            seenFlagOffsets = std::vector<int>({ PokeDex + 0x44, blockOfs[1] + 0x5F8, blockOfs[4] + 0xB98 });
            break;
    }

    initialize();
}

void Sav3::initialize(void)
{
    // Set up PC data buffer beyond end of save file.
    Box = length;
    // TODO: Resize or something
    // Array.Resize(ref Data, Data.Length + SIZE_RESERVED); // More than enough empty space.

    // Copy chunk to the allocated location
    for (int i = 5; i < BLOCK_COUNT; i++)
    {
        int blockIndex = blockOrder[i];
        if (blockIndex == -1) // block empty
            continue;
        memcpy((u8*)Box + ((i - 5) * 0xF80), data.get() + (blockIndex * SIZE_BLOCK) + ABO(), chunkLength[i]);
    }

    switch (game)
    {
        case Game::RS:
            OFS_PCItem = blockOfs[1] + 0x0498;
            OFS_PouchHeldItem = blockOfs[1] + 0x0560;
            OFS_PouchKeyItem = blockOfs[1] + 0x05B0;
            OFS_PouchBalls = blockOfs[1] + 0x0600;
            OFS_PouchTMHM = blockOfs[1] + 0x0640;
            OFS_PouchBerry = blockOfs[1] + 0x0740;
            eventFlag = blockOfs[2] + 0x2A0;
            // EventConst = EventFlag + (EventFlagMax / 8);
            // DaycareOffset = blockOfs[4] + 0x11C;
            break;
        case Game::E:
            OFS_PCItem = blockOfs[1] + 0x0498;
            OFS_PouchHeldItem = blockOfs[1] + 0x0560;
            OFS_PouchKeyItem = blockOfs[1] + 0x05D8;
            OFS_PouchBalls = blockOfs[1] + 0x0650;
            OFS_PouchTMHM = blockOfs[1] + 0x0690;
            OFS_PouchBerry = blockOfs[1] + 0x0790;
            eventFlag = blockOfs[2] + 0x2F0;
            // EventConst = EventFlag + (EventFlagMax / 8);
            // DaycareOffset = blockOfs[4] + 0x1B0;
            break;
        case Game::FRLG:
            OFS_PCItem = blockOfs[1] + 0x0298;
            OFS_PouchHeldItem = blockOfs[1] + 0x0310;
            OFS_PouchKeyItem = blockOfs[1] + 0x03B8;
            OFS_PouchBalls = blockOfs[1] + 0x0430;
            OFS_PouchTMHM = blockOfs[1] + 0x0464;
            OFS_PouchBerry = blockOfs[1] + 0x054C;
            eventFlag = blockOfs[1] + 0xEE0;
            // EventConst = blockOfs[2] + 0x80;
            // DaycareOffset = blockOfs[4] + 0x100;
            break;
        default:
            break;
    }

    // LoadEReaderBerryData();

    // Sanity Check SeenFlagOffsets -- early saves may not have block 4 initialized yet
    // SeenFlagOffsets = SeenFlagOffsets.Where(z => z >= 0).ToArray(); // TODO: What does this do?
}

void Sav3::resign(void) {
    // TODO
}

u32 Sav3::securityKey(void) const
{
    switch (game)
    {
        case Game::E:
            return Endian::convertTo<u32>(&data[blockOfs[0] + 0xAC]);
        case Game::FRLG:
            return Endian::convertTo<u32>(&data[blockOfs[0] + 0xF20]);
        default:
            return 0u;
    }
}

u16 Sav3::TID(void) const
{
    return Endian::convertTo<u16>(&data[blockOfs[0] + 0xA]);
}
void Sav3::TID(u16 v)
{
    Endian::convertFrom<u16>(&data[blockOfs[0] + 0xA], v);
}

u16 Sav3::SID(void) const
{
    return Endian::convertTo<u16>(&data[blockOfs[0] + 0xC]);
}
void Sav3::SID(u16 v)
{
    Endian::convertFrom<u16>(&data[blockOfs[0] + 0xC], v);
}

u8 Sav3::version(void) const
{
    // TODO
}
void Sav3::version(u8)
{
    // TODO
}

u8 Sav3::gender(void) const
{
    return data[blockOfs[0] + 8];
}
void Sav3::gender(u8 v)
{
    data[blockOfs[0] + 8] = v;
}


u8 Sav3::subRegion(void) const
{
    return 0; // TODO: Confirm this is unused
}
void Sav3::subRegion(u8)
{
    // TODO: Confirm this is unused
}

u8 Sav3::country(void) const
{
    return 0; // TODO: Confirm this is unused
}
void Sav3::country(u8)
{
    // TODO: Confirm this is unused
}

u8 Sav3::consoleRegion(void) const
{
    return 0; // TODO: Confirm this is unused
}
void Sav3::consoleRegion(u8)
{
    // TODO: Confirm this is unused
}

Language Sav3::language(void) const
{
    // TODO: Other languages? Is this unused?
    return japanese ? Language::JP : Language::EN;
}
void Sav3::language(Language v)
{
    // TODO: Unused?
}

std::string Sav3::otName(void) const
{
    return StringUtils::getString3(data.get(), blockOfs[0] + 0x10, japanese ? 5 : 7, japanese);
}
void Sav3::otName(const std::string &v)
{
    StringUtils::setString3(data.get(), v, blockOfs[0] + 0x10, japanese ? 5 : 7, japanese);
}

u32 Sav3::money(void) const
{
    switch (game) {
        case Game::RS:
        case Game::E:
            return Endian::convertTo<u32>(&data[blockOfs[1] + 0x0490]) ^ securityKey();
        case Game::FRLG:
            return Endian::convertTo<u32>(&data[blockOfs[1] + 0x0290]) ^ securityKey();
        default:
            return 0;
    }
}
void Sav3::money(u32 v)
{
    switch (game) {
        case Game::RS:
        case Game::E:
            Endian::convertFrom<u32>(&data[blockOfs[1] + 0x0490], v ^ securityKey());
        case Game::FRLG:
            Endian::convertFrom<u32>(&data[blockOfs[1] + 0x0290], v ^ securityKey());
        default:
            break;
    }
}

// TODO:? Coins

u32 Sav3::BP(void) const
{
    return Endian::convertTo<u16>(&data[blockOfs[0] + 0xEB8]);
}
void Sav3::BP(u32 v) {
    if (v > 9999)
    {
        v = 9999;
    }

    Endian::convertFrom<u16>(&data[blockOfs[0] = 0xEB8], v);
}

// TODO:? BPEarned

// TODO:? BerryPowder

bool Sav3::getEventFlag(int flagNumber) const
{
    if (flagNumber >= 8 * (game == Game::E ? 300 : 288))
        return 0;

    int start = eventFlag;
    if (game == Game::FRLG && flagNumber >= 0x500)
    {
        flagNumber -= 0x500;
        start = blockOfs[2];
    }
    return FlagUtil::getFlag(data.get(), start + (flagNumber >> 3), flagNumber & 7);
}

void Sav3::setEventFlag(int flagNumber, bool value)
{
    if (flagNumber >= 8 * (game == Game::E ? 300 : 288))
        return;

    int start = eventFlag;
    if (game == Game::FRLG && flagNumber >= 0x500)
    {
        flagNumber -= 0x500;
        start = blockOfs[2];
    }
    FlagUtil::setFlag(data.get(), start + (flagNumber >> 3), flagNumber & 7, value);
}

u8 Sav3::badges(void) const
{
    int startFlag = 0;
    switch (game)
    {
        case Game::FRLG:
            startFlag = 0x820;
            break;
        case Game::RS:
            startFlag = 0x807;
        case Game::E:
            startFlag = 0x867;
        default:
            break;
    }

    int ret = 0;
    for (int i = 0; i < 8; i++)
    {
        if (getEventFlag(startFlag + i))
            ret++;
    }

    return ret;
}

u16 Sav3::playedHours(void) const
{
    return Endian::convertTo<u16>(&data[blockOfs[0] + 0xE]);
}
void Sav3::playedHours(u16 v)
{
    Endian::convertFrom<u16>(&data[blockOfs[0] + 0xE], v);
}

u8 Sav3::playedMinutes(void) const
{
    return data[blockOfs[0] + 0x10];
}
void Sav3::playedMinutes(u8 v)
{
    data[blockOfs[0] + 0x10] = v;
}

u8 Sav3::playedSeconds(void) const
{
    return data[blockOfs[0] + 0x11];
}
void Sav3::playedSeconds(u8 v)
{
    data[blockOfs[0] + 0x11] = v;
}

// TODO:? playedFrames, u8 at 0x12

u8 Sav3::currentBox(void) const
{
    return data[Box];
}
void Sav3::currentBox(u8 v)
{
    data[Box] = v;
}

u32 Sav3::boxOffset(u8 box, u8 slot) const
{
    return Box + 4 + (SIZE_STORED * box * 30);
}

u32 Sav3::partyOffset(u8 slot) const
{
    return blockOfs[1] + (game == Game::FRLG ? 0x38 : 0x238) + (SIZE_PARTY * slot);
}

std::shared_ptr<PKX> Sav3::pkm(u8 slot) const
{
    return std::make_shared<PK3>(&data[partyOffset(slot), true]);
}
std::shared_ptr<PKX> Sav3::pkm(u8 box, u8 slot) const
{
    return std::make_shared<PK3>(&data[boxOffset(box, slot)]);
}

void Sav3::pkm(std::shared_ptr<PKX> pk, u8 slot)
{
    // TODO: Not sure why the std::make_unique<PK3> has an error...
    // if(pk->generation() == Generation::THREE)
    // {
    //     u8 buf[SIZE_PARTY] = {0};
    //     std::copy(pk->rawData(), pk->rawData() + pk->getLength(), buf);
    //     std::unique_ptr<PK3> pk3 = std::make_unique<PK3>(buf, true, true);

    //     if (pk->getLength() != SIZE_PARTY)
    //     {
    //         for (int i = 0; i < 6; i++)
    //         {
    //             pk3->partyStat(Stat(i), pk3->stat(Stat(i)));
    //         }
    //         pk3->partyLevel(pk3->level());
    //         pk3->partyCurrHP(pk3->stat(Stat::HP));
    //     }

    //     pk3->encrypt();
    //     std::copy(pk3->rawData(), pk3->rawData() + pk3->getLength(), &data[partyOffset(slot)]);
    // }
}
void Sav3::pkm(std::shared_ptr<PKX> pk, u8 box, u8 slot, bool applyTrade)
{
    if (pk->generation() == Generation::THREE)
    {
        if (applyTrade)
        {
            trade(pk);
        }

        std::copy(pk->rawData(), pk->rawData() + SIZE_STORED, &data[boxOffset(box, slot)]);
    }
}

void Sav3::trade(std::shared_ptr<PKX> pk)
{
    // TODO: This is copied from gen 4 and probably needs to change some
    if (pk->egg() && (otName() != pk->otName() || TID() != pk->TID() || SID() != pk->SID() || gender() != pk->otGender()))
    {
        pk->metLocation(2002);
    }
}

std::shared_ptr<PKX> Sav3::emptyPkm() const
{
    return std::make_shared<PK3>();
}

bool Sav3::canSetDex(int species)
{
    if (species <= 0)
        return false;
    if (species > maxSpecies())
        return false;
    // if (game == GameVersion.Invalid) // TODO:? I didn't add invalid game
    //     return false;
    // if (blockOfs.Any(z => z < 0))    // TODO: I've got no clue what this means
    //     return false;
    return true;
}

u32 Sav3::dexPIDUnown(void)
{
    return Endian::convertTo<u32>(&data[PokeDex + 0x4]);
}
void Sav3::dexPIDUnown(u32 v)
{
    Endian::convertFrom<u32>(&data[PokeDex + 0x4], v);
}

u32 Sav3::dexPIDSpinda(void)
{
    return Endian::convertTo<u32>(&data[PokeDex + 0x8]);
}
void Sav3::dexPIDSpinda(u32 v)
{
    Endian::convertFrom<u32>(&data[PokeDex + 0x8], v);
}

void Sav3::dex(std::shared_ptr<PKX> pk)
{
    int species = pk->species();
    if (!canSetDex(species))
        return;

    switch (species)
    {
        case 201: // Unown
            if (!getSeen(species))
                dexPIDUnown(pk->PID());
            break;
        case 327: // Spinda
            if (!getSeen(species))
                dexPIDSpinda(pk->PID());
            break;
    }
    setCaught(species, true);
    setSeen(species, true);
}

bool Sav3::getCaught(int species) const
{
    int bit = species - 1;
    int ofs = bit >> 3;
    int caughtOffset = PokeDex + 0x10;
    return FlagUtil::getFlag(data.get(), caughtOffset + ofs, bit & 7);
}

void Sav3::setCaught(int species, bool caught)
{
    int bit = species - 1;
    int ofs = bit >> 3;
    int caughtOffset = PokeDex + 0x10;
    FlagUtil::setFlag(data.get(), caughtOffset + ofs, bit & 7, caught);
}

bool Sav3::getSeen(int species) const
{
    int bit = species - 1;
    int ofs = bit >> 3;
    int seenOffset = PokeDex + 0x44;
    return FlagUtil::getFlag(data.get(), seenOffset + ofs, bit & 7);
}

void Sav3::setSeen(int species, bool seen)
{
    int bit = species - 1;
    int ofs = bit >> 3;

    for(int o : seenFlagOffsets)
        FlagUtil::setFlag(data.get(), o + ofs, bit & 7, seen);
}

int Sav3::dexSeen(void) const
{
    int ret = 0;
    for (int i = 1; i < maxSpecies(); i++)
    {
        if (getSeen(i))
            ret++;
    }

    return ret;
}

int Sav3::dexCaught(void) const
{
    int ret = 0;
    for (int i = 1; i < maxSpecies(); i++)
    {
        if (getCaught(i))
            ret++;
    }

    return ret;
}

int Sav3::emptyGiftLocation(void) const
{
    // TODO
}

std::vector<Sav::giftData> Sav3::currentGifts(void) const
{
    // TODO
}

void Sav3::mysteryGift(WCX& wc, int& pos)
{
    // TODO
}

std::unique_ptr<WCX> Sav3::mysteryGift(int pos) const
{
    // TODO
}

void Sav3::cryptBoxData(bool crypted)
{
    // TODO
}

std::string Sav3::boxName(u8 box) const
{   
    return StringUtils::getString3(data.get(), boxOffset(maxBoxes(), 0) + (box * 9), 9, japanese);
}
void Sav3::boxName(u8 box, const std::string &v)
{
    return StringUtils::setString3(data.get(), v, boxOffset(maxBoxes(), 0) + (box * 9), 9, japanese);
}

// Note: This is needed for pkmn-chest, but not in PKSM's core currently
u8 Sav3::boxWallpaper(u8 box) const
{
    int offset = boxOffset(maxBoxes(), 0);
    offset += (maxBoxes() * 0x9) + box;
    return data[offset];
}
void Sav3::boxWallpaper(u8 box, u8 v)
{
    int offset = boxOffset(maxBoxes(), 0);
    offset += (maxBoxes() * 0x9) + box;
    data[offset] = v;
}

u8 Sav3::partyCount(void) const
{
    return data[blockOfs[1] + (game == Game::FRLG ? 0x34 : 0x234)];
}
void Sav3::partyCount(u8 v)
{
    data[blockOfs[1] + (game == Game::FRLG ? 0x34 : 0x234)] = v;
}

const std::set<int>& Sav3::availableItems(void) const
{
    if (items.empty())
    {
        fill_set(items, 0, maxItem());
    }
    return items;
}

const std::set<int>& Sav3::availableMoves(void) const
{
    if (moves.empty())
    {
        fill_set(moves, 0, maxMove());
    }
    return moves;
}

const std::set<int>& Sav3::availableSpecies(void) const
{
    if (species.empty())
    {
        fill_set(species, 1, maxSpecies());
    }
    return species;
}

const std::set<int>& Sav3::availableAbilities(void) const
{
    if (abilities.empty())
    {
        fill_set(abilities, 1, maxAbility());
    }
    return abilities;
}

const std::set<int>& Sav3::availableBalls(void) const
{
    if (balls.empty())
    {
        fill_set(balls, 1, maxBall());
    }
    return balls;
}

void Sav3::item(const Item& item, Pouch pouch, u16 slot)
{
    // TODO: Make sure those are actually the right offsets
    Item3 inject = (Item3)item;
    auto write   = inject.bytes();
    switch (pouch)
    {
        case NormalItem:
            std::copy(write.first, write.first + write.second, &data[OFS_PouchHeldItem + slot * 4]);
            break;
        case KeyItem:
            std::copy(write.first, write.first + write.second, &data[OFS_PouchKeyItem + slot * 4]);
            break;
        case Ball:
            std::copy(write.first, write.first + write.second, &data[OFS_PouchBalls + slot * 4]);
            break;
        case TM:
            std::copy(write.first, write.first + write.second, &data[OFS_PouchTMHM + slot * 4]);
            break;
        case Berry:
            std::copy(write.first, write.first + write.second, &data[OFS_PouchBerry + slot * 4]);
            break;
        case PCItem:
            std::copy(write.first, write.first + write.second, &data[OFS_PCItem + slot * 4]);
            break;
        default:
            return;
    }
}
std::unique_ptr<Item> Sav3::item(Pouch pouch, u16 slot) const
{
    // TODO: Make sure those are actually the right offsets
    switch (pouch)
    {
        case NormalItem:
            return std::make_unique<Item3>(&data[OFS_PouchHeldItem + slot * 4]);
        case KeyItem:
            return std::make_unique<Item3>(&data[OFS_PouchKeyItem + slot * 4]);
        case Ball:
            return std::make_unique<Item3>(&data[OFS_PouchBalls + slot * 4]);
        case TM:
            return std::make_unique<Item3>(&data[OFS_PouchTMHM + slot * 4]);
        case Berry:
            return std::make_unique<Item3>(&data[OFS_PouchBerry + slot * 4]);
        case PCItem:
            return std::make_unique<Item3>(&data[OFS_PCItem + slot * 4]);
        default:
            return nullptr;
    }
}

std::vector<std::pair<Sav::Pouch, int>> Sav3::pouches(void) const
{
    // TODO: Is this the right thing for the int?
    return {
        {NormalItem, (OFS_PouchKeyItem - OFS_PouchHeldItem)/4},
        {KeyItem, (OFS_PouchBalls - OFS_PouchKeyItem)/4},
        {Ball, (OFS_PouchTMHM - OFS_PouchBalls)/4},
        {TM, (OFS_PouchBerry - OFS_PouchTMHM)/4},
        {Berry, game == Game::FRLG ? 43 : 46},
        {PCItem, (OFS_PouchHeldItem - OFS_PCItem)/4}
    };
}

std::map<Sav::Pouch, std::vector<int>> Sav3::validItems() const
{
    std::vector<int> keyItems, pcItems;

    switch (game)
    {
        case Game::FRLG:
            keyItems = validKeyItemsRS;
            keyItems.insert(keyItems.end(), validKeyItemsFRLG.begin(), validKeyItemsFRLG.end());
            break;
        case Game::E:
            keyItems = validKeyItemsRS;
            keyItems.insert(keyItems.end(), validKeyItemsFRLG.begin(), validKeyItemsFRLG.end());
            keyItems.insert(keyItems.end(), validKeyItemsE.begin(), validKeyItemsE.end());
            break;
        default:
            keyItems = validKeyItemsRS;
            break;
    }

    pcItems.insert(pcItems.begin(), validNormalItems.begin(), validNormalItems.end());
    pcItems.insert(pcItems.begin(), validBalls.begin(), validBalls.end());
    pcItems.insert(pcItems.begin(), keyItems.begin(), keyItems.end());
    pcItems.insert(pcItems.begin(), validTMs.begin(), validTMs.end());
    pcItems.insert(pcItems.begin(), validBerries.begin(), validBerries.end());

    return {{NormalItem, validNormalItems}, {Ball, validBalls}, {KeyItem, keyItems}, {TM, validTMs}, {Berry, validBerries}, {PCItem, pcItems}};
}

std::string Sav3::pouchName(Pouch pouch) const
{
    switch (pouch)
    {
        case NormalItem:
            return Lang::get("items");
        case KeyItem:
            return Lang::get("keyItems");
        case Ball:
            return Lang::get("balls");
        case TM:
            return Lang::get("tmHm");
        case Berry:
            return Lang::get("berries");
        case PCItem:
            return Lang::get("pcItems");
        default:
            return "";
    }
}
