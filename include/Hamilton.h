#pragma once

#include "Map.h"
#include "Pos.h"

#include <vector>
#include <iostream>

class Hamilton {
public:
    typedef uint location_type;

    void generate(Map& map);
    Pos next(const Pos& pos) const;
    location_type location(const Pos& anchor, const Pos& a) const;

    friend std::ostream& operator<<(std::ostream& os, const Hamilton& h);

private:
    std::vector<std::vector<Direc> > steps;
    std::vector<std::vector<location_type> > sequence;
    location_type maxSequence;
};
