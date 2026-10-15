#pragma once
#include "Scene.h"
#include "Player.h"
#include "Room.h"
#include "Camera.h"
#include "Grid.h"

class NekoMotionTestScene : public Scene
{
public:
	explicit NekoMotionTestScene(class GameContext* context) : Scene(context) {}
	void Init() override;
	void Update(float deltaTime) override;
	void Render() const override;
private:
	Player player_;
	Room room_;
	Camera::DebugCamera debugCamera_;
	Grid grid_;
};

