#include "Snake.h"
#include "GameCtrl.h"

using std::vector;
using std::list;
using std::shared_ptr;

Snake::Snake() {
}

Snake::~Snake() {
}

bool Snake::isDead() const {
    return dead;
}

bool Snake::addBody(const Pos &p) {
    if (map && map->isInside(p)) {
        if (body.size() == 0) {  // Insert a head
            map->getPoint(p).setType(headType);
        } else {  // Insert a body
            if (body.size() > 1) {
                auto oldTail = getTail();
                map->getPoint(oldTail).setType(bodyType);
            }
            map->getPoint(p).setType(tailType);
        }
        body.push_back(p);
        return true;
    } else {
        return false;
    }
}

void Snake::setDirection(const Direc &d) {
    direc = d;
}

void Snake::setHeadType(const Point::Type &type) {
    headType = type;
}

void Snake::setBodyType(const Point::Type &type) {
    bodyType = type;
}

void Snake::setTailType(const Point::Type &type) {
    tailType = type;
}

Direc Snake::getDirection() const {
    return direc;
}

void Snake::setMap(std::shared_ptr<Map> m) {
    map = m;

    hamilton.generate(*map);
}

const Pos& Snake::getHead() const {
    return *body.begin();
}

const Pos& Snake::getTail() const {
    return *body.rbegin();
}

Snake::size_type Snake::length() const {
    return body.size();
}

void Snake::removeTail() {
    if (map) {
        map->getPoint(getTail()).setType(Point::Type::EMPTY);
    }
    body.pop_back();
    if (body.size() > 1) {
        map->getPoint(getTail()).setType(tailType);
    }
}

void Snake::move() {
    if (isDead() || direc == NONE || !map) {
        return;
    }

    map->getPoint(getHead()).setType(bodyType);
    Pos newHead = getHead().getAdjPos(direc);
    body.push_front(newHead);

    if (!map->isSafe(newHead)) {
        dead = true;
    } else {
        if (map->getPoint(newHead).getType() != Point::Type::FOOD) {
            removeTail();
        } else {
            map->removeFood();
        }
    }

    map->getPoint(newHead).setType(headType);
}

void Snake::move(const std::list<Direc> &path) {
    for (const auto &d : path) {
        setDirection(d);
        move();
    }
}

void Snake::findPathTo(const int type, const Pos &to, std::list<Direc> &path) {
    if (to == Pos::INVALID) {
        return;
    }
    // To search the path to goal, first set the goal grid type to EMPTY
    // and then start searching because the original type of the goal
    // grid may be a FOOD or another type which is ignored by the search algorithm.
    // After searching, restore the goal grid type.
    auto originType = map->getPoint(to).getType();
    map->getPoint(to).setType(Point::Type::EMPTY);
    if (type == 0) {
        map->findMinPath(getHead(), to, direc, path);
    } else if (type == 1) {
        map->findMaxPath(getHead(), to, direc, path);
    }
    map->getPoint(to).setType(originType);
}

void Snake::findMinPathToFood(std::list<Direc> &path) {
    findPathTo(0, map->getFood(), path);
}

void Snake::findMinPathToTail(std::list<Direc> &path) {
    findPathTo(0, getTail(), path);
}

void Snake::findMaxPathToTail(std::list<Direc> &path) {
    findPathTo(1, getTail(), path);
}

void Snake::decideNext() {
    if (isDead() || !map) {
        return;
    }

    Pos nextH = hamilton.next(getHead());
    Direc dirH = getHead().getDirectionTo(nextH);

    // Step1: Find shortest path, follow if not before tail..head
    list<Direc> pathToFood;
    findMinPathToFood(pathToFood);
    if (!pathToFood.empty()) {
        Direc dirF = *(pathToFood.begin());
        Pos nextF = getHead().getAdjPos(dirF);
        auto headLoc = hamilton.location(getTail(), getHead());
        auto nextLoc = hamilton.location(getTail(), nextF);
        auto foodLoc = hamilton.location(getTail(), map->getFood());
        if (headLoc < nextLoc && nextLoc < foodLoc) {
            this->setDirection(dirF);
            return;
        }
    }

    // Step2:
    // If no suitable path is found in step1, make the snake move
    // along the hamilton path
    {
        this->setDirection(dirH);
    }
}
