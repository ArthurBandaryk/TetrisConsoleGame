/* clang-format off */
#include <Windows.h>
/* clang-format on */

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <memory>
#include <string_view>
#include <thread>

using consoleDeleter = void (*)(void* console);

constexpr std::size_t MAP_WIDTH = 72;
constexpr std::size_t MAP_HEIGHT = 24;
constexpr std::size_t NUM_CHARACTERS = MAP_WIDTH * MAP_HEIGHT;

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
    void createBlock();

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

    createBlock();

    auto result = WriteConsoleOutput(
        console_.get(),
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

    for (float dt = deltaTime(hrClock::now() - prevTickTime).count(); dt <= GAME_TICK;)
    {
        float timeUntilTickRelease = GAME_TICK - dt;

        if (timeUntilTickRelease < 0)
        {
            break;
        }

        auto sleepDelta = std::chrono::duration<float, std::micro>(timeUntilTickRelease);

        std::this_thread::sleep_for(sleepDelta);
    }

    prevTickTime = hrClock::now();
}

void Tetris::prepareMap()
{
    for (std::size_t i = 0; i < MAP_HEIGHT; ++i)
    {
        for (std::size_t j = 0; j < MAP_WIDTH; ++j)
        {
            const auto index = j + i * MAP_WIDTH;

            map_[index].Attributes = COLOR_BACKGROUND;

            if (j == 0 || j == MAP_WIDTH - 1 || i == 0 || i == MAP_HEIGHT - 1)
            {
                map_[index].Char.UnicodeChar = L'#';
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

void Tetris::createBlock()
{
    std::string_view block{ "   @      @      @   @@@@@@@" }; // __|__

    constexpr std::size_t BLOCK_WIDTH{ 7 }, BLOCK_HEIGHT{ 4 };

    constexpr std::size_t START_X = MAP_WIDTH >> 1;
    constexpr std::size_t START_Y = MAP_HEIGHT >> 1;

    CHAR_INFO chBlock[BLOCK_WIDTH * BLOCK_HEIGHT];

    for (std::size_t i = 0; i < block.size(); ++i)
    {
        chBlock[i].Char.UnicodeChar = block[i];
        chBlock[i].Attributes = COLOR_BACKGROUND;
    }

    SMALL_RECT sr{
        START_X,
        START_Y,
        START_X + BLOCK_WIDTH,
        START_Y + BLOCK_HEIGHT
    };

    auto result = WriteConsoleOutput(console_.get(),
                                     chBlock,
                                     COORD{ BLOCK_WIDTH, BLOCK_HEIGHT },
                                     COORD{ 0, 0 },
                                     &sr);

    assert(result);
}
