#include "Map.h"
#include "Pos.h"
#include "Hamilton.h"

#include <vector>
#include <iostream>

void Hamilton::generate(const Map& map) {
    size_t rows = map.getRowCount();
    size_t columns = map.getColCount();

    // TODO: Prims to generate for any valid shape. This is limited to a
    // rectangle with several restrictions.
    if (rows < 4 || columns < 4) {
        throw std::string("Grid too small");
    }
    if (rows % 2 != 0) {
        throw std::string("Rows must be divisible by two");
    }

    // Expand backing stores
    steps.resize(rows);
    sequence.resize(rows);
    for (size_t i=0; i<rows; i++) {
        steps[i].resize(columns);
        sequence[i].resize(columns);
    }

    // Walk through filling in each
    size_t seq = 0;
    {
        size_t i = 1;
        while (i < rows - 1) {
            // Left->right
            for (size_t j = 2; j < columns - 2; j++) {
                sequence[i][j] = seq++;
                steps[i][j] = RIGHT;
            }

            // Down
            {
                size_t j = columns  - 2;
                sequence[i][j] = seq++;
                steps[i][j] = DOWN;
            }
            i++;

            // Right->left
            for (size_t j = columns - 2; j >= 2; j--) {
                sequence[i][j] = seq++;
                steps[i][j] = LEFT;
            }

            // Down
            {
                size_t j = 2;
                sequence[i][j] = seq++;
                steps[i][j] = DOWN;
            }
            i++;
        }
    }

    // Fix up connection between last row and loop back
    steps[rows-2][2] = LEFT;

    // Loop back
    {
        size_t j = 1;
        for (size_t i = rows - 2; i >= 1; i--) {
            sequence[i][j] = seq++;
            steps[i][j] = UP;
        }
    }

    // Fix up connection between loop back and first row
    steps[1][1] = RIGHT;

    maxSequence = seq;
}

Pos Hamilton::next(const Pos& pos) const {
    return pos.getAdjPos(steps[pos.getX()][pos.getY()]);
}

bool Hamilton::lessthan(const Pos& anchor, const Pos& a, const Pos& b) const {
    int anchorD = sequence[anchor.getX()][anchor.getY()];
    int aD = sequence[a.getX()][a.getY()];
    int bD = sequence[b.getX()][b.getY()];

    if (aD < anchorD) {
        aD += maxSequence;
    }
    if (bD < anchorD) {
        bD += maxSequence;
    }

    return aD < bD;
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
