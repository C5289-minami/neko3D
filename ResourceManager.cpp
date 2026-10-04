// =============================
// Resources/ResourceManager.cpp
// =============================
#include "ResourceManager.h"
#include "DxPlus/DxPlus.h"
#include "ResourceKeys.h"

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

    LoadFont(ResourceKeys::Font_Title,      L"./Data/Fonts/Bitcount/static/Bitcount-Light.ttf");

    // モデルの読み込み
    LoadModel(ResourceKeys::Model_Stage,    L"./Data/Models/Stage.mv1");
    LoadModel(ResourceKeys::Model_Paladin,    L"./Data/Models/Paladin.mv1");
    LoadModel(ResourceKeys::Model_Boss,    L"./Data/Models/Boss.mv1");

    LoadSpriteStudioPlayer(ResourceKeys::SpriteStudio_TitleCharacter,
        "character_template1", "./Data/Images/character_template1.ssbp",
        "character_template_3head/stance");
    LoadSpriteStudioPlayer(ResourceKeys::SpriteStudio_Gauge,
        "character_template1", "./Data/Images/character_template1.ssbp",
        "character_template_3head/stance");
}

void ResourceManager::UnloadAll()
{
    UnloadGrids();
    UnloadFonts();
    UnloadMusics();
    UnloadSounds();
    UnloadModels();
    UnloadSpriteStudioPlayers();
    if (ssResMan)
    {
        ssResMan->removeAllData();
    }
    spriteStudioDataKeys.clear();
}

// ===============================[  GRIDS  ]===================================

[[nodiscard]] const DxPlus::Sprite::SpriteBase* ResourceManager::GridAt(const std::wstring& key, int x, int y) const
{
    auto it = grids.find(key);
    if (it == grids.end()) return nullptr;
    const auto& g = it->second;
    if (x < 0 || x >= g.num.x || y < 0 || y >= g.num.y) return nullptr;
    int idx = y * g.num.x + x;
    if (idx >= static_cast<int>(g.frames.size())) return nullptr;
    return &g.frames[idx];
}

const DxPlus::Sprite::SpriteBase* ResourceManager::GetSprite(const std::wstring& key) const
{
    return GridAt(key);
}

const DxPlus::Sprite::SpriteBase* ResourceManager::LoadUISprite(const std::wstring& key,
    const std::wstring& path)
{
    return LoadTextureAsSpriteCenter(key, path);
}

ss::Player* ResourceManager::GetSpriteStudioPlayer(const std::wstring& key) const
{
    auto it = spriteStudioPlayers.find(key);
    return it != spriteStudioPlayers.end() ? it->second.get() : nullptr;
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

int ResourceManager::LoadMusic(const std::wstring& key, const std::wstring& path)
{
    int music = DxLib::LoadSoundMem(path.c_str());
    if (music == -1) DxPlus::Utils::FatalError((L"Failed to load music " + path).c_str());
    musics[key] = music;
    return music;
}

int ResourceManager::LoadSound(const std::wstring& key, const std::wstring& path)
{
    int sound = DxLib::LoadSoundMem(path.c_str());
    if (sound == -1) DxPlus::Utils::FatalError((L"Failed to load music " + path).c_str());
    sounds[key] = sound;
    return sound;
}

int ResourceManager::LoadModel(const std::wstring& key, const std::wstring& path)
{
    // 二重ロード防止（キャッシュ）
    if (auto it = models.find(key); it != models.end())
        return it->second;

    int h = DxLib::MV1LoadModel(path.c_str());
    if (h == -1) DxPlus::Utils::FatalError((L"Failed to load model " + path).c_str());

    models[key] = h;
    return h;
}

ss::Player* ResourceManager::LoadSpriteStudioPlayer(const std::wstring& key,
    const std::string& dataKey, const std::string& path, const std::string& animation)
{
    if (!ssResMan)
    {
        Initialize();
    }

    if (auto it = spriteStudioPlayers.find(key); it != spriteStudioPlayers.end())
    {
        return it->second.get();
    }

    if (spriteStudioDataKeys.insert(dataKey).second)
    {
        ssResMan->addDataWithKey(dataKey, path);
    }

    auto player = std::unique_ptr<ss::Player>(ss::Player::create(ssResMan));
    if (!player)
    {
        DxPlus::Utils::FatalError(L"Failed to create SpriteStudio player");
        return nullptr;
    }

    player->setData(dataKey);
    player->play(animation);
    player->setAlpha(255);
    player->setFlip(false, false);

    auto* result = player.get();
    spriteStudioPlayers.emplace(key, std::move(player));
    return result;
}

void ResourceManager::UnloadMusics()
{
    for (auto& m : musics)
    {
        if (m.second >= 0) DxLib::DeleteSoundMem(m.second);
    }
    musics.clear();
}

void ResourceManager::UnloadSounds()
{
    for (auto& s : sounds)
    {
        if (s.second >= 0) DxLib::DeleteSoundMem(s.second);
    }
    sounds.clear();
}

void ResourceManager::UnloadModels()
{
    for (auto& m : models)
    {
        if (m.second >= 0) DxLib::MV1DeleteModel(m.second);
    }
    models.clear();
}

void ResourceManager::UnloadSpriteStudioPlayers()
{
    spriteStudioPlayers.clear();
}

// ===============================[  FONTS  ]===================================

int ResourceManager::GetFont(const std::wstring& fontName) const
{
    auto it = fonts.find(fontName);
    if (it == fonts.end())
    {
        DxPlus::Utils::FatalError((L"Font not found: " + fontName).c_str());
    }
    return it->second.handle;
}

int ResourceManager::LoadFont(const std::wstring& fontName, const std::wstring& path)
{
    if (AddFontResourceExW(path.c_str(), FR_PRIVATE, 0) == 0)
    {
        DxPlus::Utils::FatalError((std::wstring(L"Failed to add font: ") + path).c_str());
    }

    int handle = DxPlus::Text::InitializeFont(fontName.c_str(), 40, 2);
    if (handle == -1)
    {
        DxPlus::Utils::FatalError((std::wstring(L"Failed to init font: ") + fontName).c_str());
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
    for (const auto& pair : fonts)// kv:key-value
    {
        keys.push_back(pair.first);
    }

    for (const auto& name : keys)
    {
        UnloadFont(name);
    }
}
