#include "Map.h"
#include "Pos.h"
#include "Hamilton.h"

#include <vector>
#include <iostream>
#include <cassert>

void Hamilton::generate(Map& map) {
    size_t rows = map.getRowCount();
    size_t columns = map.getColCount();

    // Expand backing stores
    steps.resize(rows);
    sequence.resize(rows);
    for (size_t i=0; i<rows; i++) {
        steps[i].resize(columns);
        sequence[i].resize(columns);
    }

    // Get two empty spaces from map
    std::vector<Pos> empty;
    map.getEmptyPoints(empty);

    Pos first = empty[1];
    Pos second = empty[0];
    assert(first.getDirectionTo(second) != NONE);

    std::list<Direc> path;
    map.findMaxPath(first, second, first.getDirectionTo(second), path);

    uint seq = 0;
    Pos cur = first;
    for (auto d: path) {
        size_t i = cur.getX();
        size_t j = cur.getY();
        steps[i][j] = d;
        sequence[i][j] = ++seq;

        cur = cur.getAdjPos(d);
    }
    steps[second.getX()][second.getY()] = second.getDirectionTo(first);
    sequence[second.getX()][second.getY()] = 0;

    maxSequence = seq;
    if (maxSequence+1 != empty.size()) {
        throw std::runtime_error("Unable to generate covering hamilton path");
    }
}

Pos Hamilton::next(const Pos& pos) const {
    return pos.getAdjPos(steps[pos.getX()][pos.getY()]);
}

Hamilton::location_type Hamilton::location(const Pos& anchor, const Pos& a) const {
    int anchorD = sequence[anchor.getX()][anchor.getY()];
    int aD = sequence[a.getX()][a.getY()];

    if (aD < anchorD) {
        aD += maxSequence;
    }
    return aD;
}

std::ostream& operator<<(std::ostream& out, const Hamilton& h) {
    for (size_t i = 0; i < h.steps.size(); i++) {
        for (size_t j = 0; j < h.steps[0].size(); j++) {
            switch (h.steps[i][j]) {
            case RIGHT:
                out << std::string(">");
                break;
            case LEFT:
                out << std::string("<");
                break;
            case UP:
                out << std::string("^");
                break;
            case DOWN:
                out << std::string("v");
                break;
            case NONE:
                out << std::string("#");
                break;
            }
        }
        out << std::endl;
    }
    out << std::endl;

    for (size_t i = 0; i < h.sequence.size(); i++) {
        for (size_t j = 0; j < h.sequence[0].size(); j++) {
            out << h.sequence[i][j] << ",";
        }
        out << std::endl;
    }

    out << "max: " << h.maxSequence << std::endl;

    return out;
}
