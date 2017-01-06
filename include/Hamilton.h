#pragma once

#include "Map.h"
#include "Pos.h"

#include <vector>
#include <iostream>

class Hamilton {
public:
    void generate(const Map& map);
    Pos next(const Pos& pos) const;
    bool lessthan(const Pos& anchor, const Pos& a, const Pos& b) const;

    friend std::ostream& operator<<(std::ostream& os, const Hamilton& h);

private:
    std::vector<std::vector<Direc> > steps;
    std::vector<std::vector<uint> > sequence;
    uint maxSequence;
};
