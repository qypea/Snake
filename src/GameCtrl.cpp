#include "GameCtrl.h"
#include <exception>
#include <cstdio>
#include <chrono>
#include <cstdlib>
#ifdef _WIN32
#include <Windows.h>
#endif

using std::string;
using std::list;

const string GameCtrl::MSG_BAD_ALLOC = "Oops! Not enough memory to run the game! ";
const string GameCtrl::MSG_LOSE = "Oops! You lose! ";
const string GameCtrl::MSG_WIN = "Congratulations! You Win! ";
const string GameCtrl::MSG_ESC = "Game ended! ";
const string GameCtrl::MAP_INFO_FILENAME = "movements.txt";

GameCtrl::GameCtrl() {}

GameCtrl::~GameCtrl() {}

GameCtrl* GameCtrl::getInstance() {
    // According to C++11, static field constructor is thread-safe
    static GameCtrl instance;
    return &instance;
}

void GameCtrl::sleepFor(const long ms) const {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void GameCtrl::sleepUntil(std::chrono::steady_clock::time_point tp, const long ms) const {
    std::this_thread::sleep_until(tp + std::chrono::milliseconds(ms));
}

void GameCtrl::sleepByFPS() const {
    sleepFor(static_cast<long>((1.0 / fps) * 1000));
}

void GameCtrl::exitGame(const std::string &msg) {
    mutexExit.lock();

    // Ask main to stop threads
    threadWork = false;

    // Print message
    Console::setCursor(0, mapRowCnt + 1);
    Console::writeWithColor(msg + "\n", ConsoleColor(WHITE, BLACK, true, false));
}

void GameCtrl::exitGameWithError(const std::string &err) {
    exitGame("Exception: " + err + "\n");
}

void GameCtrl::setMapRow(const Map::size_type &n) {
    mapRowCnt = n;
}

void GameCtrl::setMapCol(const Map::size_type &n) {
    mapColCnt = n;
}

void GameCtrl::setFPS(const double &fps_) {
    fps = fps_;
}

void GameCtrl::setEnableAI(const bool &enable) {
    enableAI = enable;
}

void GameCtrl::setMoveInterval(const long &ms) {
    moveInterval = ms;
}

void GameCtrl::setRunTest(const bool &b) {
    runTest = b;
}

void GameCtrl::setRecordMovements(const bool &b) {
    recordMovements = b;
}

int GameCtrl::run() {
    try {
        init();
        if (runTest) {
            //testCreateFood();
            testGraphSearch();
        }
        while (threadWork) {
            sleepByFPS();
        }
    } catch (const std::exception &e) {
        exitGameWithError(e.what());
    }
    return 0;
}

void GameCtrl::init() {
    Console::clear();
    initMap();
    if (!runTest) {
        initSnakes();
        if (recordMovements) {
            initFiles();
        }
    }
    startThreads();
}

void GameCtrl::teardown() {
    stopThreads();

    // Close movement file
    if (movementFile) {
        fclose(movementFile);
        movementFile = nullptr;
    }
}

void GameCtrl::initMap() {
    if (mapRowCnt < 4 || mapColCnt < 4) {
        string msg = "GameCtrl.initMap(): Map size is at least 4*4. Current size is "
            + intToStr(mapRowCnt) + "*" + intToStr(mapColCnt);
        throw std::range_error(msg.c_str());
    }
    
    map = std::make_shared<Map>(mapRowCnt, mapColCnt);
    if (!map) {
        exitGame(MSG_BAD_ALLOC);
    } else {
        // Add some extra walls manully
    }
}

void GameCtrl::initSnakes() {
    snake.setHeadType(Point::Type::SNAKE_HEAD);
    snake.setBodyType(Point::Type::SNAKE_BODY);
    snake.setTailType(Point::Type::SNAKE_TAIL);
    snake.setMap(map);
    snake.addBody(Pos(1, 3));
    snake.addBody(Pos(1, 2));
    snake.addBody(Pos(1, 1));
}

void GameCtrl::initFiles() {
    movementFile = fopen(MAP_INFO_FILENAME.c_str(), "w");
    if (!movementFile) {
        throw std::runtime_error("GameCtrl.initFiles(): Fail to open file: " + MAP_INFO_FILENAME);
    } else {
        // Write content description to the file
        string str = "Content description:\n";
        str += "#: wall\nH: snake head\nB: snake body\nT: snake tail\nF: food\n\n";
        str += "Movements:\n\n";
        fwrite(str.c_str(), sizeof(char), str.length(), movementFile);
    }
}

void GameCtrl::moveSnake(Snake &s) {
    mutexMove.lock();
    if (map->isAllBody()) {
        mutexMove.unlock();
        exitGame(MSG_WIN);
    } else if (s.isDead()) {
        mutexMove.unlock();
        exitGame(MSG_LOSE);
    } else {
        try {
            s.move();
            if (recordMovements && s.getDirection() != NONE) {
                writeMapToFile();
            }
            mutexMove.unlock();
        } catch (const std::exception) {
            mutexMove.unlock();
            throw;
        }
    }
}

void GameCtrl::writeMapToFile() const {
    if (!movementFile) {
        return;
    }
    auto rows = map->getRowCount();
    auto cols = map->getColCount();
    for (Map::size_type i = 0; i < rows; ++i) {
        for (Map::size_type j = 0; j < cols; ++j) {
            switch (map->getPoint(Pos(i, j)).getType()) {
                case Point::Type::EMPTY:
                    fwrite("  ", sizeof(char), 2, movementFile); break;
                case Point::Type::WALL:
                    fwrite("# ", sizeof(char), 2, movementFile); break;
                case Point::Type::FOOD:
                    fwrite("F ", sizeof(char), 2, movementFile); break;
                case Point::Type::SNAKE_HEAD:
                    fwrite("H ", sizeof(char), 2, movementFile); break;
                case Point::Type::SNAKE_BODY:
                    fwrite("B ", sizeof(char), 2, movementFile); break;
                case Point::Type::SNAKE_TAIL:
                    fwrite("T ", sizeof(char), 2, movementFile); break;
                default:
                    break;
            }
        }
        fwrite("\n", sizeof(char), 1, movementFile);
    }
    fwrite("\n", sizeof(char), 1, movementFile);
}

void GameCtrl::startThreads() {
    threadWork = true;
    gameThread = std::thread(&GameCtrl::game, this);
    gameThread.detach();
    keyboardThread = std::thread(&GameCtrl::keyboard, this);
    keyboardThread.detach();
    if (!runTest) {
        moveThread = std::thread(&GameCtrl::autoMove, this);
        moveThread.detach();
    }
}

void GameCtrl::stopThreads() {
    threadWork = false;
    gameThread.join();
    keyboardThread.join();
    if (!runTest) {
        moveThread.join();
    }
}

void GameCtrl::game() {
    try {
        while (threadWork) {
            if (!runTest) {
                score += scoreTime;
                if (!map->hasFood()) {
                    map->createRandFood();
                    score += scoreFood;
                }
            }
            drawMapContent();
            sleepByFPS();
        }
    } catch (const std::exception &e) {
        exitGameWithError(e.what());
    }
}

void GameCtrl::drawMapContent() const {
    Console::setCursor();
    auto rows = map->getRowCount();
    auto cols = map->getColCount();
    for (Map::size_type i = 0; i < rows; ++i) {
        for (Map::size_type j = 0; j < cols; ++j) {
            const Point &point = map->getPoint(Pos(i, j));
            switch (point.getType()) {
                case Point::Type::EMPTY:
                    Console::writeWithColor("  ", ConsoleColor(BLACK, BLACK));
                    break;
                case Point::Type::WALL:
                    Console::writeWithColor("  ", ConsoleColor(WHITE, WHITE, true, true));
                    break;
                case Point::Type::FOOD:
                    Console::writeWithColor("  ", ConsoleColor(YELLOW, YELLOW, true, true));
                    break;
                case Point::Type::SNAKE_HEAD:
                    Console::writeWithColor("  ", ConsoleColor(RED, RED, true, true));
                    break;
                case Point::Type::SNAKE_BODY:
                    Console::writeWithColor("  ", ConsoleColor(GREEN, GREEN, true, true));
                    break;
                case Point::Type::SNAKE_TAIL:
                    Console::writeWithColor("  ", ConsoleColor(BLUE, BLUE, true, true));
                    break;
                case Point::Type::TEST_VISIT:
                    drawTestPoint(point, ConsoleColor(BLUE, GREEN, true, true));
                    break;
                case Point::Type::TEST_PATH:
                    drawTestPoint(point, ConsoleColor(BLUE, RED, true, true));
                    break;
                default:
                    break;
            }
        }
        Console::write("\n");
    }

    if (!runTest) {
        Console::write("Score: " + intToStr(score) + "\n");
    }
}

std::string dirToStr(Direc d) {
    switch (d) {
        case LEFT:
            return "<"; break;
        case UP:
            return "^"; break;
        case RIGHT:
            return ">"; break;
        case DOWN:
            return "v"; break;
        case NONE:
        default:
            return "O"; break;
    }
}

void GameCtrl::drawTestPoint(const Point &p, const ConsoleColor &consoleColor) const {
    string pointStr = "";
    if (p.getParent() == Pos::INVALID || p.getPos() == Pos::INVALID) {
        pointStr = "  ";
    } else {
        pointStr += dirToStr(p.getParent().getDirectionTo(p.getPos()));
        pointStr += " ";
    }
    Console::writeWithColor(pointStr, consoleColor);
}

void GameCtrl::keyboard() {
    try {
        while (threadWork) {
            if (Console::kbhit()) {
                switch (Console::getch()) {
                    case 'w':
                        keyboardMove(snake, Direc::UP);
                        break;
                    case 'a':
                        keyboardMove(snake, Direc::LEFT);
                        break;
                    case 's':
                        keyboardMove(snake, Direc::DOWN);
                        break;
                    case 'd':
                        keyboardMove(snake, Direc::RIGHT);
                        break;
                    case ' ':
                        pause = !pause;  // Pause or resume game
                        break;
                    case 27:  // Esc
                        exitGame(MSG_ESC);
                        break;
                    default:
                        break;
                }
            }
            sleepByFPS();
        }
    } catch (const std::exception &e) {
        exitGameWithError(e.what());
    }
}

void GameCtrl::keyboardMove(Snake &s, const Direc &d) {
    if (pause) {
        s.setDirection(d);
        moveSnake(s);
    } else if (!enableAI) {
        if (s.getDirection() == d) {
            moveSnake(s);  // Accelerate
        } else {
            s.setDirection(d);
        }
    }
}

void GameCtrl::autoMove() {
    try {
        while (threadWork) {
            auto iterstart = std::chrono::steady_clock::now();
            if (!pause) {
                if (enableAI) {
                    snake.decideNext();
                }
                moveSnake(snake);
            }
            auto iterend = std::chrono::steady_clock::now();
            if (iterend - iterstart
                    >= std::chrono::milliseconds(moveInterval)) {
                throw std::range_error("Took too long to decide");
            }

            sleepUntil(iterstart, moveInterval);
        }
    } catch (const std::exception &e) {
        exitGameWithError(e.what());
    }
}

void GameCtrl::testCreateFood() {
    while (1) {
        map->createRandFood();
        sleepByFPS();
    }
}

void GameCtrl::testGraphSearch() {
    if (mapRowCnt != 20 || mapColCnt != 20) {
        throw std::range_error("GameCtrl.testGraphSearch(): Require map size 20*20.");
    }

    list<Direc> path;
    map->setShowSearchDetails(true);

    // Add walls for testing
    for (int i = 4; i < 16; ++i) {
        map->getPoint(Pos(i, 9)).setType(Point::Type::WALL);   // vertical
        map->getPoint(Pos(4, i)).setType(Point::Type::WALL);   // horizontal #1
        map->getPoint(Pos(15, i)).setType(Point::Type::WALL);  // horizontal #2
    }
   
    Pos from(6, 7), to(14, 13);
    map->findMinPath(from, to, Direc::NONE, path);
    //map->findMaxPath(from, to, Direc::NONE, path);

    // Print result path info
    string res = "Path from " + from.toString() + " to " + to.toString()
        + " of length " + intToStr(path.size()) + ":\n";
    for (const auto &d : path) {
        res += dirToStr(d);
    }
    exitGame(res);
}
