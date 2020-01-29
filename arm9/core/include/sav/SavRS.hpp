#ifndef SAVRS_HPP
#define SAVRS_HPP

#include "Sav3.hpp"

class SavRS : public Sav3
{
public:
    SavRS(std::shared_ptr<u8[]> dt);

    std::map<Pouch, std::vector<int>> validItems(void) const override;
};

#endif