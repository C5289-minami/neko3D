#pragma once
#include "Vector3.h"
#include <unordered_map>

class EffectManager
{
public:
	void Update(float deltaTime);
	void Draw() const;
	void PlayHit(int key, const Vec3& position, float yaw);
private:
};