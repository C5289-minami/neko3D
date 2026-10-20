#include "CameraObjectManager.h"
#include "CameraObjectManager.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <utility>
#include <nlohmann/json.hpp>

namespace
{
	std::vector<Camera::Camera*>& RegisteredCameras()
	{
		static std::vector<Camera::Camera*> objects;
		return objects;
	}

	bool IsValidPose(const Camera::CameraPose& pose)
	{
		return std::isfinite(pose.position.x) &&
			std::isfinite(pose.position.y) &&
			std::isfinite(pose.position.z) &&
			std::isfinite(pose.pitch) &&
			std::isfinite(pose.yaw) &&
			std::isfinite(pose.roll) &&
			std::isfinite(pose.fieldOfViewDegrees);
	}
}

CameraObjectManager& CameraObjectManager::GetInstance()
{
	static CameraObjectManager instance;
	return instance;
}

void CameraObjectManager::Add(Camera::Camera& object)
{
	auto& objects = RegisteredCameras();
	if (std::find(objects.begin(), objects.end(), &object) == objects.end())
		objects.push_back(&object);
}

void CameraObjectManager::Remove(Camera::Camera& object)
{
	auto& objects = RegisteredCameras();
	objects.erase(std::remove(objects.begin(), objects.end(), &object), objects.end());
}

bool CameraObjectManager::AddCamera(const std::string& name, const Camera::CameraPose& pose)
{
	if (name.empty() || !IsValidPose(pose) ||
		std::find_if(cameras_.begin(), cameras_.end(),
			[&name](const SavedCameraPose& saved) { return saved.name == name; }) != cameras_.end())
		return false;

	cameras_.push_back({ name, pose });
	if (SaveCameras()) return true;

	cameras_.pop_back();
	return false;
}

bool CameraObjectManager::UpdateCameraPose(const std::string& name, const Camera::CameraPose& pose)
{
	if (!IsValidPose(pose)) return false;

	const auto it = std::find_if(cameras_.begin(), cameras_.end(),
		[&name](const SavedCameraPose& saved) { return saved.name == name; });
	if (it == cameras_.end()) return false;

	const Camera::CameraPose previousPose = it->pose;
	it->pose = pose;
	if (SaveCameras()) return true;

	it->pose = previousPose;
	return false;
}

bool CameraObjectManager::RemoveCamera(const std::string& name)
{
	const auto it = std::find_if(cameras_.begin(), cameras_.end(),
		[&name](const SavedCameraPose& saved) { return saved.name == name; });
	if (it == cameras_.end()) return false;

	const auto index = static_cast<std::size_t>(std::distance(cameras_.begin(), it));
	SavedCameraPose removed = *it;
	cameras_.erase(it);
	if (SaveCameras()) return true;

	cameras_.insert(cameras_.begin() + index, std::move(removed));
	return false;
}

bool CameraObjectManager::LoadCameras(const std::string& filePath)
{
	try
	{
		std::ifstream file(filePath);
		if (!file) return false;

		const nlohmann::json json = nlohmann::json::parse(file);
		const auto cameras = json.find("cameras");
		if (cameras == json.end() || !cameras->is_array()) return false;

		std::vector<SavedCameraPose> loaded;
		for (const auto& saved : *cameras)
		{
			if (!saved.is_object()) return false;
			SavedCameraPose item;
			item.name = saved.at("name").get<std::string>();
			const auto& pose = saved.at("pose");
			const auto& position = pose.at("position");
			if (item.name.empty() || !position.is_array() || position.size() != 3) return false;
			item.pose.position = { position[0].get<float>(), position[1].get<float>(), position[2].get<float>() };
			item.pose.pitch = pose.at("pitch").get<float>();
			item.pose.yaw = pose.at("yaw").get<float>();
			item.pose.roll = pose.at("roll").get<float>();
			item.pose.fieldOfViewDegrees = pose.at("fieldOfViewDegrees").get<float>();
			if (!IsValidPose(item.pose) ||
				std::find_if(loaded.begin(), loaded.end(), [&item](const SavedCameraPose& existing)
					{ return existing.name == item.name; }) != loaded.end())
				return false;
			loaded.push_back(std::move(item));
		}

		cameras_ = std::move(loaded);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CameraObjectManager::SaveCameras(const std::string& filePath) const
{
	try
	{
		nlohmann::json cameras = nlohmann::json::array();
		for (const auto& saved : cameras_)
		{
			if (saved.name.empty() || !IsValidPose(saved.pose)) return false;
			const auto& pose = saved.pose;
			cameras.push_back({
				{ "name", saved.name },
				{ "pose", {
					{ "position", { pose.position.x, pose.position.y, pose.position.z } },
					{ "pitch", pose.pitch },
					{ "yaw", pose.yaw },
					{ "roll", pose.roll },
					{ "fieldOfViewDegrees", pose.fieldOfViewDegrees }
				} }
				});
		}

		const std::filesystem::path path(filePath);
		if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
		std::ofstream output(filePath, std::ios::trunc);
		if (!output) return false;
		output << nlohmann::json{ { "cameras", std::move(cameras) } }.dump(4);
		return output.good();
	}
	catch (const std::exception&)
	{
		return false;
	}
}

Camera::CameraPose CameraObjectManager::GetCameraPose(const std::string& name) const
{
	const auto it = std::find_if(cameras_.begin(), cameras_.end(),
		[&name](const SavedCameraPose& saved) { return saved.name == name; });
	if (it == cameras_.end()) return {};

	return it->pose;
}

void CameraObjectManager::Draw() const
{
}

bool CameraObjectManager::SaveAll() const
{
	return SaveCameras();
}
