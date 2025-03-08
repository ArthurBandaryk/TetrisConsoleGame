/* clang-format off */
#include <Windows.h>
/* clang-format on */

#include <array>
#include <cassert>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

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
    enum class BlockType
    {
        QUAD,
        Z,
        T,
        L,
        I,
    };

    struct Block
    {
        std::string representation{};
        COORD size{};
    };

    void renderScene();
    void processInput();
    void update();
    void prepareFrames();
    void prepareMap();
    void prepareConsole();
    void createBlock(BlockType type, const COORD& position);
    void createTestBlocks();

    // Need one more byte for '\0' null terminator.
    std::array<CHAR_INFO, NUM_CHARACTERS + 1> map_{};

    std::unordered_map<BlockType, Block> blocks_{
        { BlockType::QUAD, { "@@@@", { 2, 2 } } },
        { BlockType::Z, { "+++    +++", { 5, 2 } } },
        { BlockType::T, { "  &  &&&&&", { 5, 2 } } },
        { BlockType::L, { "%   %%%%", { 4, 2 } } },
        { BlockType::I, { "KKK", { 1, 3 } } },
    };

    std::unique_ptr<void, consoleDeleter> console_{ nullptr, nullptr };

    bool isRunning_{ true };
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
    createTestBlocks();
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
    using deltaTime = std::chrono::duration<float, std::milli>;
    using hrClock = std::chrono::high_resolution_clock;

    constexpr short FRAMES = 60;
    constexpr float GAME_TICK = static_cast<float>(1) / FRAMES;

    static auto prevTickTime = hrClock::now();

    float realFrameDuration = deltaTime(hrClock::now() - prevTickTime).count();
    float tickRemainingTime = GAME_TICK - realFrameDuration;

    if (tickRemainingTime < 0)
    {
        prevTickTime = hrClock::now();
        return;
    }

    auto sleepDelta = deltaTime(tickRemainingTime);

    std::this_thread::sleep_for(sleepDelta);

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

            if (j == 0 || j == MAP_WIDTH - 1 || i == MAP_HEIGHT - 1)
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

void Tetris::createBlock(BlockType blockType, const COORD& position)
{
    const auto& block = blocks_[blockType];

    const auto blockWidth = block.size.X;
    const auto blockHeight = block.size.Y;

    for (std::size_t i = 0; i < blockHeight; ++i)
    {
        for (size_t j = 0; j < blockWidth; j++)
        {
            const auto realPos = position.X + position.Y * MAP_WIDTH + i * MAP_WIDTH + j;
            map_[realPos].Char.UnicodeChar = block.representation[j + i * blockWidth];
        }
    }
}

void Tetris::createTestBlocks()
{
    // Test function which just creates all blocks on random positions.
    auto generateRandomBlockPosition = [](const Block& block) {
        short maxX = MAP_WIDTH - block.size.X;
        short maxY = MAP_HEIGHT - block.size.Y;
        short minX = 1;
        short minY = 0;

        return COORD{
            static_cast<short>(std::rand() % (maxX - minX) + minX),
            static_cast<short>(std::rand() % (maxY - minY) + minY),
        };
    };

    for (const auto& [type, block] : blocks_)
    {
        const auto pos = generateRandomBlockPosition(block);
        createBlock(type, pos);
    }
}
