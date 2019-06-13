#ifndef LOADER_H
#define LOADER_H

#include <memory>

#include "sav/Sav.hpp"

extern std::shared_ptr<Sav> save;

bool load(const std::string savePath);

#endif
