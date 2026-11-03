#pragma once

#include <string>
#include <vector>

#include "TutorialType.h"
#include "PlayerActionEvent.h"
#include "DxPlus/DxPlus.h"

class TutorialChecklist;

struct TutorialObjective
{
    TutorialType type{};

    float progress{};
    float requiredProgress{ 1.0f };

    bool completed{};
};

class TutorialTaskManager
{
public:
    void Init(TutorialChecklist& checklist);

	void Update(float deltaTime);

    void AddTask(
        TutorialType type,
        const std::wstring& text,
        DxPlus::Vec2 stampPosition = {},
        float requiredProgress = 1.0f
    );

    // êiíªÇâ¡éZÇ∑ÇÈ
    void AddProgress(TutorialType type, float amount = 1.0f);

    // àÍäáíBê¨Ç∑ÇÈ
    void Complete(TutorialType type);

    bool IsCompleted(TutorialType type) const;
	bool IsAllFinished() const;
  

    void OnPlayerAction(PlayerActionEvent event);

private:
    int FindIndex(TutorialType type) const;

    std::vector<TutorialObjective> objectives_;

    TutorialChecklist* checklist_{};
};