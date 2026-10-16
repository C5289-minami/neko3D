// =============================
// Gameplay/Actors/Stage.cpp
// =============================
#include "Stage.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "Consts.h"
#include "imgui.h"
#include "TransformSettings.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <stdexcept>

namespace
{
    // 透過表示するステージモデルの大きさと、プレイヤー頭上の余白。
    float AlphaStageScale = 0.15f;
    float AlphaStageHeadClearance = 40.0f;

    float AlphaStageFadeSpeed = 2.4f;
}

void Stage::Init()
{
    //int modelHandle = DxLib::MV1DuplicateModel(RM().GetModel(ResourceKeys::Model_Stage));
    stagePillars_[0].modelKey = ResourceKeys::Model_hashira_l;
    stagePillars_[1].modelKey = ResourceKeys::Model_hashira_m;
    stagePillars_[2].modelKey = ResourceKeys::Model_hashira_s;

    stageModels_[0].modelKey = ResourceKeys::Model_stand_l;
    stageModels_[1].modelKey = ResourceKeys::Model_stand_m;

    // 地面の当たり判定に使うモデル
    stageModel_.modelKey = ResourceKeys::Model_StageFbx;
    LoadModelSettings();
    SetupCollisionInfo();
    Reset();
}
void Stage::SetupCollisionInfo()//当たり判定のセットアップ
{
    const auto setup = [](const ModelObject& model)//ラムダ式で書き換えれるように
    {
        int handle = RM().GetModel(model.modelKey);
        if (handle < 0) return -1;

        // 描画前でも、保存済みの配置・回転・拡大率で判定できるようにする。
        model.Draw();

        int result = MV1SetupCollInfo(handle, -1, 8, 8, 8);
        if (result < 0) return -1;

        return handle;
    };

    groundCollisionHandle_ = setup(stageModel_);
    for (int i = 0; i < stagePillars_.size(); ++i)
    {
        pillarCollisionHandles_[i] = setup(stagePillars_[i]);
    }
    for (int i = 0; i < stageModels_.size(); ++i)
    {
        stageCollisionHandles_[i] = setup(stageModels_[i]);
    }
    // コリジョン情報はRMによるMV1DeleteModel時にモデルと一緒に解放される。
}

void Stage::RefreshCollisionInfo()
{
    const auto refresh = [](const ModelObject& model, int handle)
    {
        if (handle < 0) return -1;
        // モデルが解放・差し替えされていたら、古いハンドルは使わない。
        if (RM().GetModel(model.modelKey) != handle) return -1;

        model.Draw();

		int result = MV1RefreshCollInfo(handle, -1);// コリジョン情報を更新する
        if (result < 0) return -1;

        return handle;
    };

    groundCollisionHandle_ = refresh(stageModel_, groundCollisionHandle_);
    for (int i = 0; i < stagePillars_.size(); ++i)
    {
        pillarCollisionHandles_[i] = refresh(stagePillars_[i], pillarCollisionHandles_[i]);
    }
    for (int i = 0; i < stageModels_.size(); ++i)
    {
        stageCollisionHandles_[i] = refresh(stageModels_[i], stageCollisionHandles_[i]);
    }
}

void Stage::GetCollisionModelHandles(std::vector<int>& handles) const
{
    // 呼び出し元の配列を、現在使えるハンドルの一覧に更新する。
    handles.clear();
    handles.reserve(pillarCollisionHandles_.size() + stageCollisionHandles_.size() + 1);

    if (groundCollisionHandle_ >= 0)
    {
        handles.emplace_back(groundCollisionHandle_);
    }
    for (int handle : pillarCollisionHandles_)
    {
        if (handle >= 0)
        {
            handles.emplace_back(handle);
        }
    }
    for (int handle : stageCollisionHandles_)
    {
        if (handle >= 0)
        {
            handles.emplace_back(handle);
        }
    }
}

bool Stage::IsHitPillar(const Collision::Ellipsoid& player) const
{
    bool touching = false;
    for (const auto& pillar : stagePillars_)
        touching |= Collision::IsHitEllipsoidEllipsoid(player, pillar.GetHitEllipsoid());
    return touching;
}

void Stage::DrawCollisionDebug() const
{
    for (const auto& pillar : stagePillars_)
        Collision::DrawDebugEllipsoid(pillar.GetHitEllipsoid());
}

void Stage::Reset()
{
    RefreshCollisionInfo();
    // 透過モデルの配置情報は毎回リセット
    alphaModelReady_ = false;

    // リセット直後は透過効果が残らないように不透明度を初期値へ
    alphaModel_.SetAlpha(1.0f);
}

void Stage::Draw() const
{
    // 柱と台座をそれぞれ描画
    for (const auto& model : stagePillars_)
    {
        model.Draw();
    }
    for (const auto& model : stageModels_)
    {
        model.Draw();
    }

    stageModel_.Draw();
}
void Stage::ResetAlphaModel(const Vec3& playerPosition)
{

    alphaModelReady_ = false;

    // ResourceKeys
    alphaModel_.model.modelKey = ResourceKeys::Model_StageAlpha;
    alphaModel_.model.scale = { AlphaStageScale, AlphaStageScale, AlphaStageScale };
    alphaModel_.model.rotation = {};
    alphaModel_.model.center = {};
    alphaModel_.SetAlpha(1.0f);

    const int handle = RM().GetModel(ResourceKeys::Model_StageAlpha);
    if (handle < 0) return;

    MV1SetPosition(handle, VGet(0.0f, 0.0f, 0.0f));
    MV1SetRotationXYZ(handle, VGet(0.0f, 0.0f, 0.0f));
    MV1SetScale(handle, DxConv::ToVECTOR(alphaModel_.model.scale));// スケールを適用してから範囲を取得するため、先にスケールをセットする。
    if (MV1SetupReferenceMesh(handle, -1, TRUE, TRUE) < 0) return;

    MV1RefreshReferenceMesh(handle, -1, TRUE, TRUE);
    const auto mesh = MV1GetReferenceMesh(handle, -1, TRUE, TRUE);
    const bool hasVertices = mesh.VertexNum > 0;

    //スケール適用後のモデルの範囲として保存する。
    const Vec3 minimum{ mesh.MinPosition.x, mesh.MinPosition.y, mesh.MinPosition.z };
    const Vec3 maximum{ mesh.MaxPosition.x, mesh.MaxPosition.y, mesh.MaxPosition.z };

    // 参照メッシュは範囲を取得したら解放する。
	MV1TerminateReferenceMesh(handle, -1, TRUE, TRUE);// 参照メッシュを解放しないと、モデルの範囲が正しく取得できない場合がある。 へぇ
    if (!hasVertices) return;

   //Playerの頭の上に出てくる
    alphaModel_.model.position = {
        playerPosition.x - (minimum.x + maximum.x) * 0.5f,
        playerPosition.y + Const::PLAYER_HEIGHT + AlphaStageHeadClearance - minimum.y,
        playerPosition.z - (minimum.z + maximum.z) * 0.5f
    };

    alphaModelReady_ = true;
}

void Stage::UpdateAlphaModel(float deltaTime, const Player& player)
{
    if (!alphaModelReady_) return;

    alphaModel_.FadeTo(player, AlphaStageFadeSpeed, deltaTime);
}

void Stage::DrawAlphaModel() const
{
    if (alphaModelReady_) alphaModel_.Draw();
}
namespace
{
    constexpr const char* StageSettingsPath = "Data/Settings/stage_models.json";
    constexpr const char* StagePillarNames[] = {"hashira_l", "hashira_m", "hashira_s"};
    constexpr const char* StageModelNames[] = {"stand_l", "stand_m"};
    nlohmann::json ModelVectorJson(const Vec3& v) { return nlohmann::json::array({v.x, v.y, v.z}); }
    Vec3 ReadModelVector(const nlohmann::json& v)
    {
        if (!v.is_array() || v.size() != 3) throw std::runtime_error("Expected 3 numbers.");
        return {v.at(0).get<float>(), v.at(1).get<float>(), v.at(2).get<float>()};
    }
}
bool Stage::SaveModelSettings()
{
    try
    {
        nlohmann::json doc;
        doc["version"] = 1;
        const auto saveModels = [&doc](const auto& models, const auto& names)
        {
            for (int i = 0; i < models.size(); ++i)
            {
                const auto& m = models[i];
                if (!TransformSettings::IsValid({m.scale, m.position, m.rotation})) throw std::runtime_error("Invalid transform.");
                doc["models"][names[i]] = {{"position", ModelVectorJson(m.position)}, {"rotation", ModelVectorJson(m.rotation)}, {"scale", ModelVectorJson(m.scale)}};
            }
        };
        saveModels(stagePillars_, StagePillarNames);
        saveModels(stageModels_, StageModelNames);
        std::filesystem::create_directories("Data/Settings");
        // 書き込み完了後に保存ファイルを置き換える
        const std::filesystem::path path(StageSettingsPath), temporary(std::string(StageSettingsPath) + ".tmp");
        std::ofstream output(temporary);
        if (!output) throw std::runtime_error("Cannot open save file.");
        output << doc.dump(4) << '\n';
        output.close();
        if (!output) throw std::runtime_error("Write failed.");
        if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) throw std::runtime_error("Cannot replace save file.");
        modelSettingsStatus_ = "Saved.";
        return true;
    }
    catch (const std::exception& e) { modelSettingsStatus_ = std::string("Save failed: ") + e.what(); return false; }
}
bool Stage::LoadModelSettings()
{
    try
    {
        std::ifstream input(StageSettingsPath);
        if (!input) { modelSettingsStatus_ = "No settings file. Using current values."; return false; }
        nlohmann::json doc;
        input >> doc;
        if (doc.at("version").get<int>() != 1) throw std::runtime_error("Unsupported version.");
        // 全モデルを確認してから反映する
        auto loadedPillars = stagePillars_;
        auto loadedModels = stageModels_;
        const auto loadModels = [&doc](auto& models, const auto& names)
        {
            for (int i = 0; i < models.size(); ++i)
            {
                const auto& v = doc.at("models").at(names[i]);

                auto& m = models[i];

                m.position = ReadModelVector(v.at("position"));
                m.rotation = ReadModelVector(v.at("rotation"));
                m.scale = ReadModelVector(v.at("scale"));

                if (!TransformSettings::IsValid({m.scale, m.position, m.rotation})) throw std::runtime_error("Invalid transform.");
            }
        };

        loadModels(loadedPillars, StagePillarNames);
        loadModels(loadedModels, StageModelNames);

        stagePillars_ = loadedPillars;
        stageModels_ = loadedModels;

        RefreshCollisionInfo();

        modelSettingsStatus_ = "Loaded.";

        return true;
    }
    catch (const std::exception& e) { modelSettingsStatus_ = std::string("Load failed: ") + e.what(); return false; }
}
void Stage::DrawDebugUI()
{
    ImGui::Begin("Stage parts");
    const auto drawModels = [this](auto& models, const auto& handles, const auto& names)
        {
        for (int i = 0; i < models.size(); ++i)
        {
            ImGui::PushID(names[i]);
            if (ImGui::TreeNode(names[i]))
            {
                auto& m = models[i];

                ObjectTransform v{m.scale, m.position, m.rotation};

                bool changed = ImGui::DragFloat3("Position", &v.position.x, 1.0f);
                changed |= ImGui::DragFloat3("Rotation (radians)", &v.rotation.x, 0.01f);
                changed |= ImGui::DragFloat3("Scale", &v.scale.x, 0.01f, 0.001f, 10000.0f);

                if (changed && TransformSettings::IsValid(v))
                {
                    m.position = v.position;
                    m.rotation = v.rotation;
                    m.scale = v.scale;

                    RefreshCollisionInfo();
                }
                ImGui::Text("MV1 collision: %s (handle: %d)",
                    handles[i] >= 0 ? "Ready" : "Unavailable",
                    handles[i]);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    };
    drawModels(stagePillars_, pillarCollisionHandles_, StagePillarNames);
    drawModels(stageModels_, stageCollisionHandles_, StageModelNames);
    if (ImGui::Button("Save JSON")) SaveModelSettings();
    ImGui::SameLine();
    if (ImGui::Button("Load JSON")) LoadModelSettings();
    ImGui::TextWrapped("%s", StageSettingsPath);
    ImGui::TextWrapped("%s", modelSettingsStatus_.c_str());
    ImGui::End();
}
