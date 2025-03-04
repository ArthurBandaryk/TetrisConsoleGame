#include <algorithm>
#include <array>
#include <chrono>
#include <thread>
#include <iostream>
#include <exception>

#include <Windows.h>

constexpr std::size_t MAP_WIDTH = 8;
constexpr std::size_t MAP_HEIGHT = 8;
constexpr std::size_t NUM_CHARACTERS = MAP_WIDTH * MAP_HEIGHT;

class Tetris final
{
public:
    Tetris();
    ~Tetris();

private:
    void runGame();
    void renderScene();
    void processInput();
    void update();
    void prepareFrames();
    void prepareMap();
    void prepareConsole();
    void writeToConsole();

    std::array<char, NUM_CHARACTERS + 1> map_ {};
    void* console_{nullptr};

    bool isRunning_ = true;
};

int main(int, char**)
{
    Tetris tetris {};

    return std::cout.good()
        ? EXIT_SUCCESS
        : EXIT_FAILURE;
}

Tetris::Tetris()
{
    runGame();
}

Tetris::~Tetris()
{
    CloseHandle(console_);
}

void Tetris::runGame()
{
    try{ 
        prepareConsole();
    }
    catch (const char* err) {
        std::cerr << err << std::endl;
        std::exit(1);
    }

    prepareMap();

    while (isRunning_)
    {
        prepareFrames();
        processInput();
        update();
        renderScene();
    }
}

void Tetris::renderScene()
{
    writeToConsole();
}

void Tetris::processInput()
{
}

void Tetris::update()
{
}

void Tetris::prepareFrames()
{
    using deltaTime = std::chrono::duration<float, std::micro>;
    using hrClock = std::chrono::high_resolution_clock;

    constexpr short FRAMES = 60;
    constexpr float GAME_TICK = static_cast<float>(1) / FRAMES;

    static auto prevTickTime = hrClock::now();

    while (deltaTime(hrClock::now() - prevTickTime).count() < GAME_TICK)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(0.001s);
    }

    prevTickTime = hrClock::now();
}

void Tetris::prepareMap()
{
    for (std::size_t i = 0; i < MAP_WIDTH; ++i) {
        for (std::size_t j = 0; j < MAP_HEIGHT; ++j)
        {
            if (i % MAP_WIDTH == 0 && i != 0) {
                map_[i] = '\n';
                continue;
            }

            if (i < MAP_WIDTH || i >= NUM_CHARACTERS - MAP_WIDTH || i == j * MAP_WIDTH) {
                map_[i] = '#';
            }
        }
    }

    map_[NUM_CHARACTERS] = '\0';
}

void Tetris::prepareConsole() {
    console_ = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        CONSOLE_TEXTMODE_BUFFER,
        nullptr
    );

    if (console_ == INVALID_HANDLE_VALUE) {
        throw "Invalid console";
    }

    SetConsoleActiveScreenBuffer(console_);
}

void Tetris::writeToConsole()
{
    DWORD bytesWritten = 0;
    auto result = WriteConsoleOutputCharacter(console_, map_.data(), NUM_CHARACTERS, { 0, 0 }, &bytesWritten);

    if (!result) {
        throw "Write to console failed";
    }
}

