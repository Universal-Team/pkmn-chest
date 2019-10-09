#ifndef SUMMARY_HPP
#define SUMMARY_HPP
#include "Sav.hpp"

std::pair<int, int> getPokeballPosition(u8 ball);

std::shared_ptr<PKX> showPokemonSummary(std::shared_ptr<PKX> pkm);

#endif
