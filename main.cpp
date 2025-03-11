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
using dtMilli = std::chrono::duration<float, std::milli>;
using dtSecs = std::chrono::duration<float>;
using hrClock = std::chrono::high_resolution_clock;

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
    /* clang-format off */
    enum class BlockType
    {
        QUAD, Z, T, L, I,
    };
    /* clang-format on */

    struct BlockInfo
    {
        std::string representation{};
        COORD size{};
    };

    struct MovingBlock
    {
        COORD position{};
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
    void createMover();
    void movePiece();
    bool isCollidable() const;

    std::size_t getOneDimensionalIndexFrom2D(std::size_t i, std::size_t j) const;

    // Need one more byte for '\0' null terminator.
    std::array<CHAR_INFO, NUM_CHARACTERS + 1> map_{};

    std::unordered_map<BlockType, BlockInfo> blocks_{
        { BlockType::QUAD, { "@@@@", { 2, 2 } } },
        { BlockType::Z, { "@@@    @@@", { 5, 2 } } },
        { BlockType::T, { "  @  @@@@@", { 5, 2 } } },
        { BlockType::L, { "@   @@@@", { 4, 2 } } },
        { BlockType::I, { "@@@", { 1, 3 } } },
    };

    std::unique_ptr<void, consoleDeleter> console_{ nullptr, nullptr };

    MovingBlock mover_{};

    float updateFrequency_{ 0.1f };

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
    createMover();
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
    static auto lastTimeUpdate = hrClock::now();

    auto delta = dtSecs(hrClock::now() - lastTimeUpdate).count();

    if (delta < updateFrequency_)
    {
        return;
    }

    movePiece();

    lastTimeUpdate = hrClock::now();
}

void Tetris::prepareFrames()
{
    constexpr short FRAMES = 60;
    constexpr float GAME_TICK = static_cast<float>(1) / FRAMES;

    static auto prevTickTime = hrClock::now();

    float realFrameDuration = dtMilli(hrClock::now() - prevTickTime).count();
    float tickRemainingTime = GAME_TICK - realFrameDuration;

    if (tickRemainingTime < 0)
    {
        prevTickTime = hrClock::now();
        return;
    }

    auto sleepDelta = dtMilli(tickRemainingTime);

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

    auto index1D = getOneDimensionalIndexFrom2D(position.X, position.Y);

    for (std::size_t i = 0; i < blockHeight; ++i)
    {
        for (size_t j = 0; j < blockWidth; j++)
        {
            auto realPos = index1D + i * MAP_WIDTH + j;
            map_[realPos].Char.UnicodeChar = block.representation[j + i * blockWidth];
        }
    }
}

void Tetris::createTestBlocks()
{
    // Test function which just creates all blocks on random positions.
    auto generateRandomBlockPosition = [](const BlockInfo& block) {
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

void Tetris::createMover()
{
    auto type = BlockType::T;
    auto pos = COORD{ (MAP_WIDTH >> 1) - 1, 0 };

    mover_.position = pos;
    mover_.size = blocks_[type].size;

    createBlock(type, pos);
}

void Tetris::movePiece()
{
    if (isCollidable())
    {
        return;
    }

    auto startIndex1D = getOneDimensionalIndexFrom2D(mover_.position.X, mover_.position.Y);

    for (int i = mover_.size.Y - 1; i >= 0; --i)
    {
        for (int j = 0; j < mover_.size.X; j++)
        {
            // Start with last line of block in order to prevent clearing relevant characters.
            auto chIndex = startIndex1D + MAP_WIDTH * i + j;

            // Shift down every character on the current line.
            map_[chIndex + MAP_WIDTH].Char.UnicodeChar = map_[chIndex].Char.UnicodeChar;
        }
    }

    // Clear first line of block, since after shifting process this is not relevant.
    for (std::size_t i = 0; i < mover_.size.X; ++i)
    {
        map_[startIndex1D + i].Char.UnicodeChar = L'\0';
    }

    mover_.position.Y += 1;
}

bool Tetris::isCollidable() const
{
    auto bottomLeftY = mover_.position.Y + mover_.size.Y;
    auto bottomLeftX = mover_.position.X;
    auto index = bottomLeftX + bottomLeftY * MAP_WIDTH;

    for (int i = 0; i < mover_.size.X; ++i)
    {
        auto ch = map_[index + i].Char.UnicodeChar;
        if (ch == L'@' || ch == L'#')
        {
            return true;
        }
    }

    return false;
}

std::size_t Tetris::getOneDimensionalIndexFrom2D(std::size_t x, std::size_t y) const
{
    return x + y * MAP_WIDTH;
}
