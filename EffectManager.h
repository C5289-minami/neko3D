#pragma once
#include "Vector3.h"
#include <unordered_map>

class EffectManager
{
public:
	void Update(float deltaTime);
	void Draw() const;
	static int PlayEffect(int key, const Vec3& position, float yaw = 0.0f);
	void PlayHit(int key, const Vec3& position, float yaw);
private:
};