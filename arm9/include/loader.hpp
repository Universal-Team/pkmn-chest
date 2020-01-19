#ifndef LOADER_HPP
#define LOADER_HPP

#include <memory>

#include "Sav.hpp"

extern std::shared_ptr<Sav> save;

bool loadSave(std::string savePath);

void saveChanges(std::string savePath);

#endif
