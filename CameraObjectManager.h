#pragma once
#pragma once

#include <string>
#include <vector>
#include "Camera.h"

struct SavedCameraPose
{
	std::string name;
	Camera::CameraPose pose;
};

// カメラ編集用のオブジェクト管理クラス
class CameraObjectManager
{
public:
	static CameraObjectManager& GetInstance();
	void Add(Camera::Camera& object);
    void Remove(Camera::Camera& object);
	bool AddCamera(const std::string& name, const Camera::CameraPose& pose);
	bool RemoveCamera(const std::string& name);
	bool LoadCameras(const std::string& filePath = "./Data/Config/camera_shots.json");
	bool SaveCameras(const std::string& filePath = "./Data/Config/camera_shots.json") const;
	const std::vector<SavedCameraPose>& GetCameras() const { return cameras_; }
	void Draw() const;
	bool SaveAll() const;
private:
	std::vector<SavedCameraPose> cameras_;

};

inline CameraObjectManager& CObjs() { return CameraObjectManager::GetInstance(); }

