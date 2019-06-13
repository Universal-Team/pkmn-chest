#include <fstream>

#include "loader.h"

static bool saveIsFile;
static std::string saveFileName;

std::shared_ptr<Sav> save;

bool load(const std::string savePath) {
    saveIsFile   = true;
    saveFileName = savePath;
    std::ifstream in(savePath);
    u32 size;
    char* saveData = nullptr;
    if (in.good())
    {
        in.seekg (0, in.end);
        size     = in.tellg();
        in.seekg (0, in.beg);
        saveData = new char[size];
        in.read(saveData, size);
    }
    else
    {
        // Gui::error(i18n::localize("BAD_OPEN_SAVE"), in.result());
        // loadedTitle  = nullptr;
        saveFileName = "";
        in.close();
        return false;
    }
    in.close();
    save = Sav::getSave((u8*)saveData, size);
    delete[] saveData;
    if (!save)
    {
        // Gui::warn(saveFileName, i18n::localize("SAVE_INVALID"));
        saveFileName = "";
        // loadedTitle  = nullptr;
        return false;
    }
    // if (Configuration::getInstance().autoBackup())
    // {
    //     std::string id;
    //     if (title)
    //     {
    //         backupSave(title->checkpointPrefix());
    //     }
    //     else
    //     {
    //         bool done = false;
    //         for (auto i = sdSaves.begin(); !done && i != sdSaves.end(); i++)
    //         {
    //             for (auto j = i->second.begin(); j != i->second.end(); j++)
    //             {
    //                 if (*j == savePath)
    //                 {
    //                     backupSave(i->first);
    //                     done = true;
    //                     break;
    //                 }
    //             }
    //         }
    //     }
    // }
    return true;
}
