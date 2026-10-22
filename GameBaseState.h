#pragma once
#pragma once
#include "State.h"

class GameScene;
class GameStateMachine;

class GameBaseState : public State
{
public:
	explicit GameBaseState(
		GameStateMachine& stateMachine,
       GameScene& gameScene
	) : stateMachine_(stateMachine),
     gameScene_(gameScene)
	{
	}
protected:
	GameStateMachine& stateMachine_;
 GameScene& gameScene_;
};