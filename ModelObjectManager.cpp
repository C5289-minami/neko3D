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

std::string ModelObjectManager::GetActorKey(const ActorBase& actor) const
{
    return actor.GetIdentifier();
}

std::vector<std::pair<std::string, ActorBase*>> ModelObjectManager::GetActors() const
{
    std::vector<std::pair<std::string, ActorBase*>> actors;
    for (const auto& [object, actor] : ownership_)
    {
        if (!object || !actor) continue;
        const auto existing = std::find_if(actors.begin(), actors.end(), [actor](const auto& entry)
        {
            return entry.second == actor;
        });
        if (existing != actors.end()) continue;

        const std::string key = GetActorKey(*actor);
        if (!key.empty()) actors.emplace_back(key, actor);
    }
    return actors;
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
    for (auto entry = ownership_.begin(); entry != ownership_.end(); ++entry)
    {
        ActorBase* actor = entry->second;
        if (!actor) continue;
        const bool alreadySaved = std::any_of(ownership_.begin(), entry, [actor](const auto& earlier)
        {
            return earlier.second == actor;
        });
        if (!alreadySaved) success = SaveActorTransform(*actor) && success;
    }
    return success;
}

bool ModelObjectManager::LoadActorTransform(ActorBase& actor, const std::string& filePath) const
{
    const std::string actorKey = GetActorKey(actor);
    if (actorKey.empty()) return false;

    try
    {
        std::ifstream file(filePath);
        if (!file) return false;

        nlohmann::json json;
        file >> json;
        const auto actors = json.find("actors");
        if (actors == json.end() || !actors->is_object()) return false;
        const auto saved = actors->find(actorKey);
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
    const std::string actorKey = GetActorKey(actor);
    if (actorKey.empty()) return false;

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

        json["actors"][actorKey] = {
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
