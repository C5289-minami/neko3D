#include "ModelObjectManager.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

ModelObjectManager& ModelObjectManager::GetInstance()
{
    static ModelObjectManager instance;
    return instance;
}

void ModelObjectManager::AddActor(const std::string& name, ActorBase& actor)
{
    if (name.empty()) return;
    const auto existing = std::find_if(actors_.begin(), actors_.end(), [&actor](const auto& entry)
    {
        return entry.second == &actor;
    });
    if (existing != actors_.end()) return;

    const auto duplicateName = std::find_if(actors_.begin(), actors_.end(), [&name](const auto& entry)
    {
        return entry.first == name;
    });
    if (duplicateName != actors_.end()) return;

    actors_.emplace_back(name, &actor);
    LoadActorTransform(actor);
}

// オブジェクトの追加・削除・描画・保存
void ModelObjectManager::Add(ModelObject& object)
{
	// すでに追加されている場合は何もしない
    if (std::find(objects_.begin(), objects_.end(), &object) != objects_.end()) return;
	// 初期化して追加
    objects_.push_back(&object);
}

void ModelObjectManager::Add(ModelObject& object, ActorBase& owner)
{
    Add(object);
    const auto existing = std::find_if(ownership_.begin(), ownership_.end(), [&object](const auto& entry)
    {
        return entry.first == &object;
    });
    if (existing != ownership_.end())
    {
        existing->second = &owner;
        return;
    }
    ownership_.emplace_back(&object, &owner);
}

void ModelObjectManager::Remove(ModelObject& object)
{
	// オブジェクトを削除
    objects_.erase(std::remove(objects_.begin(), objects_.end(), &object), objects_.end());
    ownership_.erase(std::remove_if(ownership_.begin(), ownership_.end(), [&object](const auto& entry)
    {
        return entry.first == &object;
    }), ownership_.end());
}

ActorBase* ModelObjectManager::GetOwner(const ModelObject& object) const
{
    const auto owner = std::find_if(ownership_.begin(), ownership_.end(), [&object](const auto& entry)
    {
        return entry.first == &object;
    });
    return owner == ownership_.end() ? nullptr : owner->second;
}

void ModelObjectManager::Draw() const
{
	// 登録されているオブジェクトを描画
    for (const auto* object : objects_)
        if (object) object->Draw();
}

bool ModelObjectManager::SaveAll() const
{
  // 登録されているActorとModelObjectをすべて保存
    bool success = true;
    for (const auto* object : objects_)
        if (object) success = object->Save() && success;
    for (const auto& [name, actor] : actors_)
    {
        (void)name;
        if (actor) success = SaveActorTransform(*actor) && success;
    }
    return success;
}

bool ModelObjectManager::LoadActorTransform(ActorBase& actor, const std::string& filePath) const
{
    const auto registered = std::find_if(actors_.begin(), actors_.end(), [&actor](const auto& entry)
    {
        return entry.second == &actor;
    });
    if (registered == actors_.end()) return false;

    try
    {
        std::ifstream file(filePath);
        if (!file) return false;

        nlohmann::json json;
        file >> json;
        const auto actors = json.find("actors");
        if (actors == json.end() || !actors->is_object()) return false;
        const auto saved = actors->find(registered->first);
        if (saved == actors->end() || !saved->is_object()) return false;

        const auto readVector = [](const nlohmann::json& value, Vec3& output)
        {
            if (!value.is_array() || value.size() != 3) return false;
            output = { value[0].get<float>(), value[1].get<float>(), value[2].get<float>() };
            return std::isfinite(output.x) && std::isfinite(output.y) && std::isfinite(output.z);
        };

        Vec3 position{};
        Vec3 rotation{};
        if (!readVector(saved->at("position"), position) ||
            !readVector(saved->at("rotation"), rotation))
            return false;

        actor.SetPosition(position);
        actor.SetPitch(rotation.x);
        actor.SetYaw(rotation.y);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool ModelObjectManager::SaveActorTransform(const ActorBase& actor, const std::string& filePath) const
{
    const auto registered = std::find_if(actors_.begin(), actors_.end(), [&actor](const auto& entry)
    {
        return entry.second == &actor;
    });
    if (registered == actors_.end()) return false;

    const Vec3 position = actor.GetPosition();
    const Vec3 rotation = actor.GetRotation();
    if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
        !std::isfinite(position.z) || !std::isfinite(rotation.x) ||
        !std::isfinite(rotation.y) || !std::isfinite(rotation.z))
        return false;

    try
    {
        nlohmann::json json = nlohmann::json::object();
        std::ifstream input(filePath);
        if (input)
        {
            input >> json;
            if (!json.is_object()) return false;
        }
        if (!json.contains("actors") || !json["actors"].is_object())
            json["actors"] = nlohmann::json::object();

        json["actors"][registered->first] = {
            { "position", { position.x, position.y, position.z } },
            { "rotation", { rotation.x, rotation.y, rotation.z } }
        };

        const std::filesystem::path path(filePath);
        if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
        std::ofstream output(filePath, std::ios::trunc);
        if (!output) return false;
        output << json.dump(4);
        return output.good();
    }
    catch (const std::exception&)
    {
        return false;
    }
}
