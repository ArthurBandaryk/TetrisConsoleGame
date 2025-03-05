#include <Windows.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <memory>
#include <thread>

using consoleDeleter = void (*)(void* console);

constexpr std::size_t MAP_WIDTH = 3;
constexpr std::size_t MAP_HEIGHT = 3;
constexpr std::size_t NUM_CHARACTERS = (MAP_WIDTH) * (MAP_HEIGHT);

class Tetris final
{
public:
    Tetris();
    ~Tetris() = default;

    Tetris(const Tetris&) = delete;
    Tetris(Tetris&&) = delete;

    Tetris& operator=(const Tetris&) = delete;
    Tetris& operator=(Tetris&&) = delete;

    void runGame();

private:
    void renderScene();
    void processInput();
    void update();
    void prepareFrames();
    void prepareMap();
    void prepareConsole();

    // Need one more byte for '\0' null terminator.
    std::array<CHAR_INFO, NUM_CHARACTERS + 1> map_{};

    std::unique_ptr<void, consoleDeleter> console_{ nullptr, nullptr };

    bool isRunning_ = true;
};

int main(int, char**)
{
    Tetris tetris{};

    tetris.runGame();

    return EXIT_SUCCESS;
}

Tetris::Tetris()
{
    prepareConsole();
    prepareMap();
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
    DWORD bytesWritten = 0;

    // Console boundaries for rendering text. See official doc:
    // https://learn.microsoft.com/en-us/windows/console/writeconsoleoutput
    SMALL_RECT sr{ 0, 0, MAP_WIDTH, MAP_HEIGHT };

    auto result = WriteConsoleOutput(console_.get(),
                                     map_.data(),
                                     { MAP_WIDTH, MAP_HEIGHT },
                                     { 0, 0 },
                                     &sr);

    assert(result);
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
    for (std::size_t i = 0; i < MAP_WIDTH; ++i)
    {
        for (std::size_t j = 0; j < MAP_HEIGHT; ++j)
        {
            map_[i + j * MAP_WIDTH].Attributes = COLOR_BACKGROUND;

            if (j % MAP_WIDTH == 0 && j != 0)
            {
                map_[i + j * MAP_WIDTH].Char.UnicodeChar = L'\n';
                continue;
            }

            if (i * j == 0 || i * j % MAP_WIDTH == 0)
            {
                map_[i + j * MAP_WIDTH].Char.UnicodeChar = L'#';
            }
        }
    }

    map_[NUM_CHARACTERS].Char.UnicodeChar = L'\0';
}

void Tetris::prepareConsole()
{
    auto console = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        CONSOLE_TEXTMODE_BUFFER,
        nullptr);

    assert(console != INVALID_HANDLE_VALUE);

    console_ = std::unique_ptr<void, consoleDeleter>(
        console,
        [](void* console) {
            assert(CloseHandle(console));
        });

    SetConsoleActiveScreenBuffer(console_.get());
}
