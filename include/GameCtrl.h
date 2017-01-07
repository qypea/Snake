#pragma once

#include "Snake.h"
#include "Console.h"
#include <thread>
#include <mutex>

/*
Game controller.
*/
class GameCtrl {
public:
    static const std::string MSG_BAD_ALLOC;
    static const std::string MSG_LOSE;
    static const std::string MSG_WIN;
    static const std::string MSG_ESC;
    static const std::string MAP_INFO_FILENAME;

    ~GameCtrl();

    /*
    Forbid copy
    */
    GameCtrl(const GameCtrl &m) = delete;
    GameCtrl& operator=(const GameCtrl &m) = delete;

    /*
    Get the only instance
    */
    static GameCtrl* getInstance();

    /*
    Sleep current thread.
    */
    void sleepFor(const long ms) const;
    void sleepUntil(std::chrono::steady_clock::time_point tp, const long ms) const;

    /*
    Print a message and exit the game. (thread-safe)
    */
    void exitGame(const std::string &msg);

    /*
    Print an error and exit the game. (thread-safe)
    */
    void exitGameWithError(const std::string &err);

    /*
    Game configuration setters.
    */
    void setMapRow(const Map::size_type &n);
    void setMapCol(const Map::size_type &n);
    void setFPS(const double &fps_);
    void setMoveInterval(const long &ms);
    void setEnableAI(const bool &enable);
    void setRunTest(const bool &b);
    void setRecordMovements(const bool &b);

    /*
    Run the game.

    @return the exit status of the program.
    */
    int run();

private:
    Map::size_type mapRowCnt = 10;
    Map::size_type mapColCnt = 10;
    long int score = 0;
    long int scoreFood = 100;
    long int scoreTime = -1;
    double fps = 60.0;
    long moveInterval = 30;
    bool enableAI = true;
    bool runTest = false;
    bool recordMovements = false;

    bool pause = false;  // Field to implement pause/resume game

    std::chrono::steady_clock::duration thinkingTime;

    Snake snake;
    std::shared_ptr<Map> map;

    bool threadWork = true;      // Thread running switcher
    std::thread gameThread;      // Thread to draw the map
    std::thread keyboardThread;  // Thread to receive keyboard instructions
    std::thread moveThread;      // Thread to move the snake

    std::mutex mutexMove;  // Mutex for moveSnake()
    std::mutex mutexExit;  // Mutex for exitGame()

    FILE *movementFile = nullptr;  // File to save snake movements

    /*
    Private constructor for singleton.
    */
    GameCtrl();

    /*
    Sleep for a time calculated by FPS value.
    */
    void sleepByFPS() const;

    /*
    Initialize.
    */
    void init();
    void initMap();
    void initSnakes();
    void initFiles();
    void teardown();

    /*
    Move the snake and check if game is over. (thread-safe)
    */
    void moveSnake(Snake &s);

    /*
    Write the map content to movement file.
    */
    void writeMapToFile() const;

    /*
    Start all threads.
    */
    void startThreads();
    void stopThreads();

    /*
    Thread contents for gameThread
    Draw the game elements, add food, etc
    */
    void game();

    /*
    Draw the map content.
    */
    void drawMapContent() const;

    /*
    Draw a point in testing program.

    @param p the point to draw
    @param the color of the point
    */
    void drawTestPoint(const Point &p, const ConsoleColor &consoleColor) const;

    /*
    Callback for keyboard thread.
    Execute keyboard instructions.
    */
    void keyboard();

    /*
    Execute keyboard move instruction.

    @param s the snake to move
    @param d the direction to move
    */
    void keyboardMove(Snake &s, const Direc &d);

    /*
    Callback for food thread.
    Create food on the map if it doesn't exist.
    */
    void createFood();

    /*
    Callback for auto move thread.
    Auto move the snake on the map.
    */
    void autoMove();

    /*
    Test create map food program.
    */
    void testCreateFood();

    /*
    Test graph search algorithms.
    */
    void testGraphSearch();
    void testHamilton();
};
