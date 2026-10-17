#include "GameScene.h"
#include "GameContext.h"

#include "GameScene.h"

#ifndef NDEBUG
#include "ModelObjectManager.h"

void GameScene::SetModelEditTargets()
{
    auto& player = gameContext->GetPlayer();
    auto& stage = gameContext->GetStage();

    MObjs().AddActor("Player", player);
    MObjs().Add(player.GetModelObject(), player);
    MObjs().AddActor("Stage", stage);
    for (auto& model : stage.GetPillarModelObjects())
        MObjs().Add(model, stage);
    for (auto& model : stage.GetStageModelObjects())
        MObjs().Add(model, stage);
    MObjs().Add(stage.GetModelObject(), stage);
}
#endif

