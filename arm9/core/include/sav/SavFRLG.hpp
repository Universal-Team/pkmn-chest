#ifndef SAVFRLG_HPP
#define SAVFRLG_HPP

#include "Sav3.hpp"

class SavFRLG : Sav3
{
public:
    SavFRLG(std::shared_ptr<u8[]> dt);

    std::map<Pouch, std::vector<int>> validItems(void) const override;
};


#endif