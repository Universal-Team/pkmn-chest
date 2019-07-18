#ifndef SUMMARY_H
#define SUMMARY_H

#include <Sav.hpp>

std::pair<int, int> getPokeballPosition(u8 ball);

std::shared_ptr<PKX> showPokemonSummary(std::shared_ptr<PKX> pkm);

#endif
