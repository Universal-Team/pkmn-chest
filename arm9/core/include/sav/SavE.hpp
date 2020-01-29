#ifndef SAVE_HPP
#define SAVE_HPP

#include "Sav3.hpp"

class SavE : public Sav3
{
public:
    SavE(std::shared_ptr<u8[]> dt);

    std::map<Pouch, std::vector<int>> validItems(void) const override;
};


#endif