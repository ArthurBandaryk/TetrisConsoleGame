#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <thread>

constexpr std::size_t MAP_WIDTH = 8;
constexpr std::size_t MAP_HEIGHT = 8;

class Tetris final
{
public:
    Tetris()
    {
        runGame();
    }

private:
    void runGame();
    void renderScene() const;
    void processInput();
    void update();
    void prepareFrames();
    void prepareMap();

    std::array<char, MAP_WIDTH * MAP_HEIGHT> map_ {};

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
    prepareMap();

    while (isRunning_)
    {
        prepareFrames();
        processInput();
        update();
        renderScene();

        system("cls");
    }
}

void Tetris::renderScene() const
{
    std::for_each(map_.cbegin(), map_.cend(), [](auto el) {std::cout << el; });
    std::cout << std::endl;
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
        for (size_t j = 0; j < MAP_HEIGHT; ++j)
        {
            if (i % MAP_WIDTH == 0 && i != 0) {
                map_[i] = '\n';
                continue;
            }

            if (i < MAP_WIDTH || i >= MAP_WIDTH * MAP_HEIGHT - MAP_WIDTH || i == j * MAP_WIDTH) {
                map_[i] = '#';
            }
        }
    }
}
