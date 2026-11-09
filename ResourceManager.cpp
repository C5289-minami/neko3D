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

	// Fonts
	LoadFont(ResourceKeys::Font_Title, L"./Data/Fonts/Bitcount/static/Bitcount-Light.ttf");
	LoadFont(ResourceKeys::Font_Game, L"./Data/Fonts/WDXL_Lubrifont_JP_N/WDXLLubrifontJPN-Regular.ttf");

	// Buttons
	auto loadButton = [this](const ResourceKeys::Button& button, const std::wstring& backPath, const std::wstring& hoverPath)
		{
			LoadSprite(button.back, backPath);
			LoadSprite(button.hover, hoverPath);
		};

	loadButton(ResourceKeys::Button_Red, L"./Data/Images/Button/button_red.png", L"./Data/Images/Button/button_red_hover.png");
	loadButton(ResourceKeys::Button_Pink, L"./Data/Images/Button/button_pink.png", L"./Data/Images/Button/button_pink_hover.png");
	loadButton(ResourceKeys::Button_Purple, L"./Data/Images/Button/button_purple.png", L"./Data/Images/Button/button_purple_hover.png");
	loadButton(ResourceKeys::Button_Rose, L"./Data/Images/Button/button_rose.png", L"./Data/Images/Button/button_rose_hover.png");
	loadButton(ResourceKeys::Button_E, L"./Data/Images/Button/E_Button.png", L"./Data/Images/Button/button_rose_hover.png");

	LoadSprite(ResourceKeys::PlayerHand.normal.mike,	L"./Data/Images/Hand/mike/1.png");
	LoadSprite(ResourceKeys::PlayerHand.normal.sabi,	L"./Data/Images/Hand/sabi/1.png");
	LoadSprite(ResourceKeys::PlayerHand.normal.kuro,	L"./Data/Images/Hand/kuro/1.png");
	LoadSprite(ResourceKeys::PlayerHand.pressed.mike,	L"./Data/Images/Hand/mike/2.png");
	LoadSprite(ResourceKeys::PlayerHand.pressed.sabi,	L"./Data/Images/Hand/sabi/2.png");
	LoadSprite(ResourceKeys::PlayerHand.pressed.kuro,	L"./Data/Images/Hand/kuro/2.png");

	// Common UI
	LoadSprite(ResourceKeys::UI_Logo, L"./Data/Images/logo.png");
	LoadSprite(ResourceKeys::UI_CreditImage, L"./Data/Images/CreditImage.png");

	// Status UI
	LoadSprite(ResourceKeys::BossStatusBack, L"./Data/Images/Boss/boss_status_back.png");
	LoadSprite(ResourceKeys::PlayerStatus.mike, L"./Data/Images/Player/mike_status_back.png");
	LoadSprite(ResourceKeys::PlayerStatus.sabi, L"./Data/Images/Player/sabi_status_back.png");
	LoadSprite(ResourceKeys::PlayerStatus.kuro, L"./Data/Images/Player/kuro_status_back.png");
	LoadSprite(ResourceKeys::PlayerHeart, L"./Data/Images/Player/heart.png");

	// Result UI
	LoadSprite(ResourceKeys::ClearResult, L"./Data/Images/Result/clear.png");
	LoadSprite(ResourceKeys::GameOverResult, L"./Data/Images/Result/gameover.png");

	// Select UI
	LoadSprite(ResourceKeys::SelectPlayers.mike, L"./Data/Images/Select/mike.png");
	LoadSprite(ResourceKeys::SelectPlayers.sabi, L"./Data/Images/Select/sabi.png");
	LoadSprite(ResourceKeys::SelectPlayers.kuro, L"./Data/Images/Select/kuro.png");
	LoadSprite(ResourceKeys::SelectCommon, L"./Data/Images/Select/common.png");
	// DKey UI
	LoadSprite(ResourceKeys::SelectDKeys.mike, L"./Data/Images/Select/mike_dkey.png");
	LoadSprite(ResourceKeys::SelectDKeys.sabi, L"./Data/Images/Select/sabi_dkey.png");
	LoadSprite(ResourceKeys::SelectDKeys.kuro, L"./Data/Images/Select/kuro_dkey.png");
	// AKey UI
	LoadSprite(ResourceKeys::SelectAKeys.mike, L"./Data/Images/Select/mike_akey.png");
	LoadSprite(ResourceKeys::SelectAKeys.sabi, L"./Data/Images/Select/sabi_akey.png");
	LoadSprite(ResourceKeys::SelectAKeys.kuro, L"./Data/Images/Select/kuro_akey.png");

	// Tutorial UI
	LoadSprite(ResourceKeys::TutorialBack, L"./Data/Images/Tutorial/tutorial.png");
	LoadSprite(ResourceKeys::TutorialStamp, L"./Data/Images/Tutorial/stamp.png");
	LoadSprite(ResourceKeys::TutorialToLeap, L"./Data/Images/Tutorial/tobikakari.png");
	LoadSprite(ResourceKeys::TutorialSkip, L"./Data/Images/Tutorial/skip.png");

	// Player models
	LoadModel(ResourceKeys::Model_Player.mike, L"./Data/Models/Player/mike.mv1");
	LoadModel(ResourceKeys::Model_Player.sabi, L"./Data/Models/Player/sabi.mv1");
	LoadModel(ResourceKeys::Model_Player.kuro, L"./Data/Models/Player/kuro.mv1");

	// Player animations
	LoadModel(ResourceKeys::Model_PlayerAnim_Idle, L"./Data/Models/Player/Animation/player_idle.mv1");
	LoadModel(ResourceKeys::Model_PlayerAnim_Walk, L"./Data/Models/Player/Animation/player_walk.mv1");
	LoadModel(ResourceKeys::Model_PlayerAnim_Jump, L"./Data/Models/Player/Animation/player_jump.mv1");
	LoadModel(ResourceKeys::Model_PlayerAnim_Dodge, L"./Data/Models/Player/Animation/player_blink.mv1");
	LoadModel(ResourceKeys::Model_PlayerAnim_Attack, L"./Data/Models/Player/Animation/player_attack.mv1");
	LoadModel(ResourceKeys::Model_PlayerAnim_Ult, L"./Data/Models/Player/Animation/player_ult.mv1");
	LoadModel(ResourceKeys::Model_PlayerAnim_SelectMike, L"./Data/Models/Player/Animation/player_select_mike.mv1");
	LoadModel(ResourceKeys::Model_PlayerAnim_SelectSabi, L"./Data/Models/Player/Animation/player_select_sabi.mv1");

	// Boss model
	LoadModel(ResourceKeys::Model_Boss, L"./Data/Models/Boss/boss.mv1");

	// Boss animations
	LoadModel(ResourceKeys::Model_BossAnim_Idle, L"./Data/Models/Boss/Animation/boss_idle.mv1");
	LoadModel(ResourceKeys::Model_BossAnim_Sippo, L"./Data/Models/Boss/Animation/boss_sippo.mv1");
	LoadModel(ResourceKeys::Model_BossAnim_Down, L"./Data/Models/Boss/Animation/boss_down.mv1");
	LoadModel(ResourceKeys::Model_BossAnim_Flinch, L"./Data/Models/Boss/Animation/boss_flinch.mv1");
	LoadModel(ResourceKeys::Model_BossAnim_Attck_Hit, L"./Data/Models/Boss/Animation/boss_tataku.mv1");
	LoadModel(ResourceKeys::Model_BossAnim_Die, L"./Data/Models/Boss/Animation/boss_zyoubutu.mv1");
	LoadModel(ResourceKeys::Model_BossAnim_Nekodamashi, L"./Data/Models/Boss/Animation/boss_nekodamashi.mv1");

	// Stage models
	LoadModel(ResourceKeys::Model_PillarL, L"./Data/Models/stage/PillarL.mv1");
	LoadModel(ResourceKeys::Model_PillarM, L"./Data/Models/stage/PillarM.mv1");
	LoadModel(ResourceKeys::Model_PillarS, L"./Data/Models/stage/PillarS.mv1");
	LoadModel(ResourceKeys::Model_Stage, L"./Data/Models/stage/Stage.mv1");
	LoadModel(ResourceKeys::Model_stand_l, L"./Data/Models/stage/stand_l.mv1");
	LoadModel(ResourceKeys::Model_stand_m, L"./Data/Models/stage/stand_m.mv1");
	LoadModel(ResourceKeys::Model_stand_s, L"./Data/Models/stage/stand_s.mv1");

	// Room models
	LoadModel(ResourceKeys::Model_Room, L"./Data/Models/Room/oheya.mv1");
	LoadModel(ResourceKeys::Model_JumpStand, L"./Data/Models/Room/jumpstand.mv1");
	LoadModel(ResourceKeys::Model_Basket, L"./Data/Models/Room/Basket.mv1");
	LoadModel(ResourceKeys::Model_Chair, L"./Data/Models/Room/Chair.mv1");

	// SpriteStudio
	LoadSpriteStudioPlayer(ResourceKeys::SpriteStudio_BossDownGauge, "BossDownGauge", "./Data/UI/supusuta.ssbp");
	LoadSpriteStudioPlayer(ResourceKeys::SpriteStudio_BossHealthGauge, "BossHealthGauge", "./Data/UI/supusuta.ssbp");
	LoadSpriteStudioPlayer(ResourceKeys::SpriteStudio_PlayerUltGauge, "PlayerUltGauge", "./Data/UI/supusuta.ssbp");

	// Music
	LoadMusic(ResourceKeys::Music_Title, L"./Data/Sounds/StartBGM.mp3");
	LoadMusic(ResourceKeys::Music_Battle, L"./Data/Sounds/BattleBGM.mp3");

	// UI sounds
	LoadSound(ResourceKeys::Sound_MenuConfirm, L"./Data/Sounds/suzu.mp3");
	LoadSound(ResourceKeys::SoundSelectPlayers.mike, L"./Data/Sounds/taka.mp3");
	LoadSound(ResourceKeys::SoundSelectPlayers.sabi, L"./Data/Sounds/gishi.mp3");
	LoadSound(ResourceKeys::SoundSelectPlayers.kuro, L"./Data/Sounds/yu.mp3");
	LoadSound(ResourceKeys::Sound_hanko, L"./Data/Sounds/hanko.mp3");
	LoadSound(ResourceKeys::Sound_Blink, L"./Data/Sounds/blink.mp3");
	LoadSound(ResourceKeys::Sound_Fall, L"./Data/Sounds/fall.mp3");

	// Boss sounds
	LoadSound(ResourceKeys::Sound_BossEntrance, L"./Data/Sounds/BossEntrance.mp3");
	LoadSound(ResourceKeys::Sound_Clap, L"./Data/Sounds/clap.mp3");
	LoadSound(ResourceKeys::Sound_Slam, L"./Data/Sounds/slam.mp3");

	SetCreate3DSoundFlag(TRUE);
	LoadSound(ResourceKeys::Sound_TailWhip, L"./Data/Sounds/tailwhip.mp3");
	SetCreate3DSoundFlag(FALSE);

	// Effects
	LoadEffect(ResourceKeys::Effect_Test, L"./Data/Effects/suna.efkefc", 30.0f);
	LoadEffect(ResourceKeys::Effect_PlayerAttack, L"./Data/Effects/attack.efkefc", 30.0f);
	LoadEffect(ResourceKeys::Effect_Impact, L"./Data/Effects/wave.efkefc", 30.0f);
	LoadEffect(ResourceKeys::Effect_Kedama, L"./Data/Effects/kedama.efkefc", 30.0f);
}

void ResourceManager::UnloadAll()
{
	UnloadGrids();
	UnloadFonts();
	UnloadMusics();
	UnloadSounds();
	UnloadModels();
	UnloadSpriteStudioPlayers();
	UnloadEffects();

	if (ssResMan != nullptr)
	{
		ssResMan->removeAllData();

		delete ssResMan;
		ssResMan = nullptr;
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

const DxPlus::Sprite::SpriteBase* ResourceManager::LoadSprite(
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

std::unique_ptr<ss::Player> ResourceManager::CreateSpriteStudioPlayerInstance(
	const std::wstring& key) const
{
	if (ssResMan == nullptr) return nullptr;
	auto dataKey = spriteStudioDataKeyByPlayerKey.find(key);
	if (dataKey == spriteStudioDataKeyByPlayerKey.end()) return nullptr;

	std::unique_ptr<ss::Player> player(ss::Player::create(ssResMan));
	if (!player) return nullptr;
	player->setData(dataKey->second);
	player->setAlpha(255);
	player->setFlip(false, false);
	return player;
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
	// play() は再生速度を 1.0f に戻すため、呼び出し後に指定された速度を設定する。
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
	auto existing = musics.find(key);
	if (existing != musics.end()) return existing->second;

	const int music = DxLib::LoadSoundMem(path.c_str());
	if (music == -1)
	{
		OutputDebugStringW((L"Failed to load music " + path + L"\n").c_str());
		return -1;
	}

	musics[key] = music;
	return music;
}

int ResourceManager::LoadSound(
	const std::wstring& key, const std::wstring& path)
{
	auto existing = sounds.find(key);
	if (existing != sounds.end()) return existing->second;

	const int sound = DxLib::LoadSoundMem(path.c_str());
	if (sound == -1)
	{
		OutputDebugStringW((L"Failed to load sound " + path + L"\n").c_str());
		return -1;
	}

	sounds[key] = sound;
	return sound;
}

int ResourceManager::LoadModel(
	const std::wstring& key, const std::wstring& path)
{
	// C++14 でもビルドできるように、C++17 の初期化文付き if 構文は使わない。
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

	// C++17 の初期化文付き if 構文は使わない。
	auto existingPlayer = spriteStudioPlayers.find(key);
	if (existingPlayer != spriteStudioPlayers.end())
	{
		return existingPlayer->second.get();
	}

	// 各 SSBP データのキーは一度だけ登録する。
	if (spriteStudioDataKeys.insert(dataKey).second)
	{
		ssResMan->addDataWithKey(dataKey, path);
	}
	spriteStudioDataKeyByPlayerKey[key] = dataKey;

	std::unique_ptr<ss::Player> player(ss::Player::create(ssResMan));
	if (!player)
	{
		DxPlus::Utils::FatalError(L"Failed to create SpriteStudio player");
		return nullptr;
	}

	player->setData(dataKey);

	// SS プレイヤーから取得したアニメーション名を自動的にキャッシュする。
	spriteStudioAnimationNames[key] = ssResMan->getAnimeName(dataKey);

	// 初期アニメーションが明示的に指定されている場合だけ再生する。
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

int ResourceManager::LoadEffect(const std::wstring& key, const std::wstring& path, float scale)
{
	// 二重ロード防止（キャッシュ）
	if (auto it = effects.find(key); it != effects.end())
		return it->second;

	int h = LoadEffekseerEffect(path.c_str(), scale);
	if (h == -1) DxPlus::Utils::FatalError((L"Failed to load effect " + path).c_str());

	effects[key] = h;
	return h;
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
	spriteStudioDataKeyByPlayerKey.clear();
}

void ResourceManager::UnloadEffects()
{
	for (auto& e : effects)
	{
		if (e.second >= 0) DeleteEffekseerEffect(e.second);
	}
	effects.clear();
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
	auto existing = fonts.find(fontName);
	if (existing != fonts.end()) return existing->second.handle;

	if (AddFontResourceExW(path.c_str(), FR_PRIVATE, 0) == 0)
	{
		DxPlus::Utils::FatalError(
			(std::wstring(L"Failed to add font: ") + path).c_str());
	}

	const int handle = DxPlus::Text::InitializeFont(fontName.c_str(), 40, 2);
	if (handle == -1)
	{
		RemoveFontResourceExW(path.c_str(), FR_PRIVATE, 0);
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
