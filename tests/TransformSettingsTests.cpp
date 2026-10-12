#include "TransformSettings.h"
#include "TransformDefaults.h"
#include "Player.h"
#include "Boss.h"
#include "GameContext.h"
#include "BossTestScene.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>

bool g_raise_imgui_viewports = false;

namespace
{
    void Check(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    bool Equal(const Vec3& a, const Vec3& b)
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    bool Equal(const ObjectTransform& a, const ObjectTransform& b)
    {
        return Equal(a.scale, b.scale) && Equal(a.position, b.position) && Equal(a.rotation, b.rotation);
    }

    std::string ReadSource(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        Check(static_cast<bool>(file), "Cannot read test source.");
        return { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
    }

    void WriteSource(const std::string& path, const std::string& source)
    {
        std::ofstream file(path, std::ios::binary);
        file.write(source.data(), static_cast<std::streamsize>(source.size()));
        file.close();
        Check(static_cast<bool>(file), "Cannot write test source.");
    }

    std::string Block(const std::string& source, const std::string& id)
    {
        const auto begin = source.find("// BEGIN_TRANSFORM(" + id + ")");
        const auto end = source.find("// END_TRANSFORM(" + id + ")");
        Check(begin != std::string::npos && end != std::string::npos, "Test markers missing.");
        return source.substr(begin, end - begin);
    }
}

int main(int argc, char** argv)
{
    try
    {
        Check(argc == 2, "Pass a test output directory.");
        const std::filesystem::path directory(argv[1]);
        std::filesystem::create_directories(directory);
        const std::string path = (directory / "SavedTransformDefaults.h").string();
        Check(std::filesystem::path(TransformSettings::SourceFilePath()).is_absolute(),
            "Source location must not depend on the runtime working directory.");
        const auto original = ReadSource(TransformSettings::SourceFilePath());
        WriteSource(path, original);
        Player player;
        Boss boss;
        const ObjectTransform playerValue{ { 100.0f, 95.0f, 105.0f }, { 12.5f, 300.0f, -67.0f },
            { 0.25f, 1.5707963f, -0.5f } };
        const ObjectTransform bossValue{ { 1.5f, 2.0f, 2.5f }, { -250.0f, 20.0f, 200.0f },
            { -0.1f, 3.1415926f, 0.2f } };
        player.SetTransform(playerValue);
        boss.SetTransform(bossValue);
        const std::vector<DebugTransformTarget> targets{
            MakeDebugTransformTarget("game/player", "Player", player),
            MakeDebugTransformTarget("game/boss", "Boss", boss) };
        std::string status;

        Check(TransformSettings::Save(targets, status, path), "Source save failed.");
        const auto savedSource = ReadSource(path);
        Check(Block(savedSource, "game/player").find("12.5f, 300.0f, -67.0f") != std::string::npos,
            "Player position was not written as C++ float literals.");
        Check(Block(savedSource, "game/boss").find("1.5f, 2.0f, 2.5f") != std::string::npos,
            "Boss scale was not written to source.");
        Check(Block(savedSource, "boss_test/player") == Block(original, "boss_test/player"),
            "Save changed an unrelated scene.");
        Check(savedSource.substr(0, 3) == original.substr(0, 3), "Source BOM was changed.");
        Check(savedSource.substr(savedSource.find("    struct Entry")) ==
            original.substr(original.find("    struct Entry")), "Save changed code outside marked defaults.");

        Check(Equal(player.GetModelObject().position, playerValue.position), "Player model position is stale.");
        Check(Equal(player.GetModelObject().rotation, playerValue.rotation), "Player model rotation is stale.");
        Check(Equal(player.GetModelObject().scale, playerValue.scale), "Player model scale is stale.");
        player.SetPosition({ 7.0f, 8.0f, 9.0f });
        Check(Equal(player.GetModelObject().position, player.GetPosition()), "Direct position edit is stale.");
        player.Reset();
        Check(Equal(player.GetTransform(), TransformDefaults::GamePlayer), "Reset did not use compiled defaults.");
        Check(Equal(player.GetModelObject().rotation, player.GetTransform().rotation), "Reset model rotation is stale.");
        player.SetTransform(playerValue);

        Check(TransformSettings::ResetToDefaults(targets, status), "Compiled defaults reset failed.");
        Check(Equal(player.GetTransform(), TransformDefaults::GamePlayer), "Source save changed this build's defaults.");
        Check(Equal(boss.GetTransform(), TransformDefaults::GameBoss), "Boss default reset failed.");
        player.SetTransform(playerValue);
        boss.SetTransform(bossValue);

        GameContext context;
        context.SetPlayerGravityEnabled(false);
        Check(!context.IsPlayerGravityEnabled(), "Gravity disable failed after merge.");
        context.SetPlayerGravityEnabled(true);
        Check(context.IsPlayerGravityEnabled(), "Gravity enable failed after merge.");
        BossTestScene bossScene(&context);
        auto gameControls = context.GetDebugControls();
        auto testControls = bossScene.GetDebugControls();
        gameControls.camera->focusPlayer();
        testControls.camera->focusPlayer();
        Check(gameControls.camera->isActive() && testControls.camera->isActive(), "Debug camera did not activate.");
        gameControls.targets[0].set(playerValue);
        testControls.targets[0].set(bossValue);
        Check(Equal(context.GetPlayerPosition(), playerValue.position), "Game edit failed during debug camera.");
        Check(Equal(testControls.targets[0].get(), bossValue), "Boss scene edit failed during debug camera.");
        Check(Equal(gameControls.targets[0].get(), playerValue), "Scene objects leaked into each other.");
        const Vec3 cameraPosition{ 10.0f, 200.0f, 300.0f };
        gameControls.camera->setPosition(cameraPosition);
        testControls.camera->setPosition(cameraPosition);
        Check(Equal(context.GetCameraPosition(), cameraPosition), "Game camera edit failed.");
        Check(Equal(testControls.camera->getPosition(), cameraPosition), "Boss camera edit failed.");
        Check(Equal(context.GetPlayerPosition(), playerValue.position), "Camera edit changed player.");
        Check(Equal(context.GetPlayerCamera().GetPosition(), { 400.0f, 400.0f, 400.0f }), "Normal camera changed.");
        gameControls.setPaused(true);
        testControls.setPaused(true);
        Check(gameControls.isPaused() && testControls.isPaused(), "Pause control failed.");

        Check(TransformSettings::Save(testControls.targets, status, path), "Other scene source save failed.");
        const auto beforeSingleSave = ReadSource(path);
        player.SetPosition({ 12.5f, 300.0f, -67.0f });
        Check(TransformSettings::Save({ targets[0] }, status, path), "Single source save failed.");
        Check(Block(ReadSource(path), "game/boss") == Block(beforeSingleSave, "game/boss"),
            "Single save overwrote boss.");
        Check(Block(ReadSource(path), "boss_test/player") == Block(beforeSingleSave, "boss_test/player"),
            "Single save overwrote another scene.");
        const auto validSource = ReadSource(path);

        auto broken = validSource;
        const std::string endMarker = "// END_TRANSFORM(game/boss)";
        broken.erase(broken.find(endMarker), endMarker.size());
        WriteSource(path, broken);
        Check(!TransformSettings::Save(targets, status, path), "Missing marker was accepted.");
        Check(ReadSource(path) == broken, "Failed multi-object save partially modified source.");
        WriteSource(path, validSource + "\n// BEGIN_TRANSFORM(game/player)\n");
        const auto duplicateSource = ReadSource(path);
        Check(!TransformSettings::Save(targets, status, path), "Duplicate marker was accepted.");
        Check(ReadSource(path) == duplicateSource, "Duplicate marker failure changed source.");
        WriteSource(path, validSource);

        auto invalid = playerValue;
        invalid.scale.y = 0.0f;
        player.SetTransform(invalid);
        Check(!TransformSettings::Save(targets, status, path), "Zero scale was accepted.");
        Check(ReadSource(path) == validSource, "Invalid transform damaged source.");
        invalid.scale.y = std::numeric_limits<float>::infinity();
        player.SetTransform(invalid);
        Check(!TransformSettings::Save(targets, status, path), "Infinite scale was accepted.");
        Check(ReadSource(path) == validSource, "Non-finite transform damaged source.");
        player.SetTransform(playerValue);

        Check(SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_READONLY) != 0, "Cannot set read-only fixture.");
        const bool readOnlySave = TransformSettings::Save(targets, status, path);
        Check(SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_NORMAL) != 0, "Cannot restore fixture attributes.");
        Check(!readOnlySave && ReadSource(path) == validSource, "Read-only save damaged source.");
        Check(!std::filesystem::exists(path + ".tmp"), "Temporary source file was left behind.");
        Check(!TransformSettings::Save(targets, status, path + ".missing.h"), "Missing source was created.");
        auto unknown = targets[0];
        unknown.id = "unknown/player";
        Check(!TransformSettings::Save({ unknown }, status, path), "Unknown source target was accepted.");
        Check(ReadSource(path) == validSource, "Unknown target changed source.");
        Check(ReadSource(TransformSettings::SourceFilePath()) == original, "Tests changed the real defaults header.");

        // 書き出したヘッダーを、後でコンパイラーでも確認する
        std::cout << "PASS: source save, partial save, compiled defaults, debug editing, invalid data.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
