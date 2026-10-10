// =============================
// Resources/ResourceManager.cpp
// =============================
#include "ResourceManager.h"
#include "DxPlus/DxPlus.h"
#include "ResourceKeys.h"

#include <cstddef>
#include <string>
#include <vector>

ResourceManager& ResourceManager::GetInstance()
{
    static ResourceManager instance;
    return instance;
}

void ResourceManager::Initialize()
{
    ssResMan = ss::ResourceManager::getInstance();
}

void ResourceManager::LoadAll()
{
    Initialize();

    LoadFont(ResourceKeys::Font_Title,
        L"./Data/Fonts/Bitcount/static/Bitcount-Light.ttf");

    // モデルの読み込み
    LoadModel(ResourceKeys::Model_Stage,      L"./Data/Models/field.mv1");
    LoadModel(ResourceKeys::Model_StageAlpha, L"./Data/Models/field.mv1");
    LoadModel(ResourceKeys::Model_Paladin,    L"./Data/Models/Paladin.mv1");
    LoadModel(ResourceKeys::Model_Boss,       L"./DevData/Models/boss_test.mv1");
    LoadModel(ResourceKeys::Model_Test,       L"./Data/Models/Sword.mv1");



    LoadSpriteStudioPlayer(
        ResourceKeys::SpriteStudio_TitleCharacter,
        "character_template1",
        "./Data/Images/character_template1.ssbp");

    LoadSpriteStudioPlayer(
        ResourceKeys::SpriteStudio_Gauge,
        "UI",
        "./DevData/UI/UI.ssbp");

    SetCreate3DSoundFlag(TRUE);
    LoadSound(ResourceKeys::Sound_BossBite, L"./Data/Sounds/Explosion.mp3");
    SetCreate3DSoundFlag(FALSE);
}

void ResourceManager::UnloadAll()
{
    UnloadGrids();
    UnloadFonts();
    UnloadMusics();
    UnloadSounds();
    UnloadModels();
    UnloadSpriteStudioPlayers();

    if (ssResMan != nullptr)
    {
        ssResMan->removeAllData();
    }

    spriteStudioDataKeys.clear();
}

// ===============================[ GRIDS ]===================================

const DxPlus::Sprite::SpriteBase* ResourceManager::GridAt(
    const std::wstring& key, int x, int y) const
{
    auto it = grids.find(key);
    if (it == grids.end()) return nullptr;

    const auto& grid = it->second;
    if (x < 0 || x >= grid.num.x || y < 0 || y >= grid.num.y) return nullptr;

    const int idx = y * grid.num.x + x;
    if (idx < 0 || idx >= static_cast<int>(grid.frames.size())) return nullptr;

    return &grid.frames[idx];
}

const DxPlus::Sprite::SpriteBase* ResourceManager::GetSprite(
    const std::wstring& key) const
{
    return GridAt(key);
}

const DxPlus::Sprite::SpriteBase* ResourceManager::LoadUISprite(
    const std::wstring& key, const std::wstring& path)
{
    return LoadTextureAsSpriteCenter(key, path);
}

ss::Player* ResourceManager::GetSpriteStudioPlayer(
    const std::wstring& key) const
{
    auto it = spriteStudioPlayers.find(key);
    if (it == spriteStudioPlayers.end()) return nullptr;
    return it->second.get();
}

std::vector<std::string> ResourceManager::GetSpriteStudioAnimationNames(
    const std::wstring& key) const
{
    auto it = spriteStudioAnimationNames.find(key);
    if (it == spriteStudioAnimationNames.end()) return std::vector<std::string>();
    return it->second;
}

bool ResourceManager::PlaySpriteStudioAnimation(
    const std::wstring& key,
    std::size_t animationIndex,
    int loopCount,
    float speed)
{
    ss::Player* player = GetSpriteStudioPlayer(key);
    if (player == nullptr || loopCount < 0 || speed <= 0.0f) return false;

    auto namesIt = spriteStudioAnimationNames.find(key);
    if (namesIt == spriteStudioAnimationNames.end()) return false;
    if (animationIndex >= namesIt->second.size()) return false;

    const std::string& animationName = namesIt->second[animationIndex];
    player->play(animationName, loopCount, 0);
    // play() resets the step to 1.0f, so set the requested speed afterward.
    player->setStep(speed);
    return true;
}

bool ResourceManager::StopSpriteStudioAnimation(const std::wstring& key)
{
    ss::Player* player = GetSpriteStudioPlayer(key);
    if (player == nullptr) return false;
    player->stop();
    return true;
}

bool ResourceManager::PauseSpriteStudioAnimation(const std::wstring& key)
{
    ss::Player* player = GetSpriteStudioPlayer(key);
    if (player == nullptr) return false;
    player->animePause();
    return true;
}

bool ResourceManager::ResumeSpriteStudioAnimation(const std::wstring& key)
{
    ss::Player* player = GetSpriteStudioPlayer(key);
    if (player == nullptr) return false;
    player->animeResume();
    return true;
}

bool ResourceManager::SetSpriteStudioSpeed(
    const std::wstring& key, float speed)
{
    ss::Player* player = GetSpriteStudioPlayer(key);
    if (player == nullptr || speed <= 0.0f) return false;
    player->setStep(speed);
    return true;
}

bool ResourceManager::SetSpriteStudioLoop(
    const std::wstring& key, int loopCount)
{
    ss::Player* player = GetSpriteStudioPlayer(key);
    if (player == nullptr || loopCount < 0) return false;
    player->setLoop(loopCount);
    return true;
}

void ResourceManager::UnloadGrids()
{
    grids.clear();
}

int ResourceManager::GetMusic(const std::wstring& key) const
{
    auto it = musics.find(key);
    return (it != musics.end()) ? it->second : -1;
}

int ResourceManager::GetSound(const std::wstring& key) const
{
    auto it = sounds.find(key);
    return (it != sounds.end()) ? it->second : -1;
}

int ResourceManager::GetModel(const std::wstring& key) const
{
    auto it = models.find(key);
    return (it != models.end()) ? it->second : -1;
}

int ResourceManager::GetEffect(const std::wstring& key) const
{
    auto it = effects.find(key);
    return (it != effects.end()) ? it->second : -1;
}

int ResourceManager::LoadMusic(
    const std::wstring& key, const std::wstring& path)
{
    const int music = DxLib::LoadSoundMem(path.c_str());
    if (music == -1)
    {
        DxPlus::Utils::FatalError((L"Failed to load music " + path).c_str());
    }

    musics[key] = music;
    return music;
}

int ResourceManager::LoadSound(
    const std::wstring& key, const std::wstring& path)
{
    const int sound = DxLib::LoadSoundMem(path.c_str());
    if (sound == -1)
    {
        DxPlus::Utils::FatalError((L"Failed to load sound " + path).c_str());
    }

    sounds[key] = sound;
    return sound;
}

int ResourceManager::LoadModel(
    const std::wstring& key, const std::wstring& path)
{
    // Do not use C++17 if-initializer syntax so this also builds under C++14.
    auto it = models.find(key);
    if (it != models.end()) return it->second;

    const int handle = DxLib::MV1LoadModel(path.c_str());
    if (handle == -1)
    {
        DxPlus::Utils::FatalError((L"Failed to load model " + path).c_str());
    }

    models[key] = handle;
    return handle;
}

ss::Player* ResourceManager::LoadSpriteStudioPlayer(
    const std::wstring& key,
    const std::string& dataKey,
    const std::string& path,
    const std::string& animation)
{
    if (ssResMan == nullptr)
    {
        Initialize();
    }

    // Do not use C++17 if-initializer syntax.
    auto existingPlayer = spriteStudioPlayers.find(key);
    if (existingPlayer != spriteStudioPlayers.end())
    {
        return existingPlayer->second.get();
    }

    // Register each SSBP data key only once.
    if (spriteStudioDataKeys.insert(dataKey).second)
    {
        ssResMan->addDataWithKey(dataKey, path);
    }

    std::unique_ptr<ss::Player> player(ss::Player::create(ssResMan));
    if (!player)
    {
        DxPlus::Utils::FatalError(L"Failed to create SpriteStudio player");
        return nullptr;
    }

    player->setData(dataKey);

    // Automatically cache the animation names returned by the SS player.
    spriteStudioAnimationNames[key] = ssResMan->getAnimeName(dataKey);

    // Play an initial animation only when one was explicitly supplied.
    if (!animation.empty())
    {
        player->play(animation);
    }

    player->setAlpha(255);
    player->setFlip(false, false);

    ss::Player* result = player.get();
    spriteStudioPlayers.emplace(key, std::move(player));
    return result;
}

void ResourceManager::UnloadMusics()
{
    for (auto& music : musics)
    {
        if (music.second >= 0) DxLib::DeleteSoundMem(music.second);
    }
    musics.clear();
}

void ResourceManager::UnloadSounds()
{
    for (auto& sound : sounds)
    {
        if (sound.second >= 0) DxLib::DeleteSoundMem(sound.second);
    }
    sounds.clear();
}

void ResourceManager::UnloadModels()
{
    for (auto& model : models)
    {
        if (model.second >= 0) DxLib::MV1DeleteModel(model.second);
    }
    models.clear();
}

void ResourceManager::UnloadSpriteStudioPlayers()
{
    spriteStudioPlayers.clear();
    spriteStudioAnimationNames.clear();
}

// ===============================[ FONTS ]===================================

int ResourceManager::GetFont(const std::wstring& fontName) const
{
    auto it = fonts.find(fontName);
    if (it == fonts.end())
    {
        DxPlus::Utils::FatalError((L"Font not found: " + fontName).c_str());
    }
    return it->second.handle;
}

int ResourceManager::LoadFont(
    const std::wstring& fontName, const std::wstring& path)
{
    if (AddFontResourceExW(path.c_str(), FR_PRIVATE, 0) == 0)
    {
        DxPlus::Utils::FatalError(
            (std::wstring(L"Failed to add font: ") + path).c_str());
    }

    const int handle = DxPlus::Text::InitializeFont(fontName.c_str(), 40, 2);
    if (handle == -1)
    {
        DxPlus::Utils::FatalError(
            (std::wstring(L"Failed to init font: ") + fontName).c_str());
    }

    fonts[fontName] = { handle, path };
    return handle;
}

void ResourceManager::UnloadFont(const std::wstring& fontName)
{
    auto it = fonts.find(fontName);
    if (it == fonts.end()) return;

    auto& info = it->second;
    if (info.handle != -1)
    {
        DxPlus::Text::DeleteFont(info.handle);
        info.handle = -1;
    }

    if (!info.path.empty())
    {
        RemoveFontResourceExW(info.path.c_str(), FR_PRIVATE, 0);
    }

    fonts.erase(it);
}

void ResourceManager::UnloadFonts()
{
    std::vector<std::wstring> keys;
    keys.reserve(fonts.size());

    for (const auto& pair : fonts)
    {
        keys.push_back(pair.first);
    }

    for (const auto& name : keys)
    {
        UnloadFont(name);
    }
}
