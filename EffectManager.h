#pragma once
#include "Vector3.h"
#include <unordered_map>

class EffectManager
{
public:
	void Load();
	void Update(float deltaTime);
	void Draw() const;
	void Finalize();
	void PlayHit(int key, const Vec3& position, float yaw);
private:
};

