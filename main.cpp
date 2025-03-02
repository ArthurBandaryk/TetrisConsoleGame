#include <array>
#include <chrono>
#include <iostream>
#include <thread>

class Tetris final
{
public:
    Tetris()
    {
        runGame();
    }

private:
    void runGame();
    void renderScene();
    void processInput();
    void update();
    void prepareFrames();
    void prepareMap();

    std::array<char, 12 * 36> map {};

    bool isRunning_ = true;
};

int main(int, char**)
{
    Tetris tetris {};

    return std::cout.good()
        ? EXIT_SUCCESS
        : EXIT_FAILURE;
}

void Tetris::runGame()
{
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
}
