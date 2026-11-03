#include "TutorialTaskManager.h"
#include "TutorialChecklist.h"

#include <algorithm>
#include <cstddef>



void TutorialTaskManager::Init(TutorialChecklist& checklist)
{
    objectives_.clear();

    checklist_ = &checklist;
    checklist_->Init();
}

void TutorialTaskManager::Update(float deltaTime)
{
    if (checklist_)
        checklist_->Update(deltaTime);
}

void TutorialTaskManager::AddTask(
    TutorialType type,
    const std::wstring& text,
    DxPlus::Vec2 stampPosition,
    float requiredProgress)
{
    if (FindIndex(type) != -1) return;

    TutorialObjective objective;
    objective.type = type;
    objective.requiredProgress = std::max(0.001f, requiredProgress);

    objectives_.push_back(objective);

    if (checklist_)
        checklist_->AddItem(text, stampPosition);
}

void TutorialTaskManager::AddProgress(
    TutorialType type,
    float amount)
{
    if (amount <= 0.0f) return;

    const int index = FindIndex(type);
    if (index == -1) return;

    auto& objective = objectives_[index];

    if (objective.completed) return;

    objective.progress = std::min(
        objective.progress + amount,
        objective.requiredProgress
    );

    if (objective.progress >= objective.requiredProgress)
    {
        objective.completed = true;

        if (checklist_)
            checklist_->CompleteItem(static_cast<std::size_t>(index));
    }
}

void TutorialTaskManager::Complete(TutorialType type)
{
    const int index = FindIndex(type);
    if (index == -1) return;

    auto& objective = objectives_[index];

    AddProgress(type, objective.requiredProgress - objective.progress);
}

bool TutorialTaskManager::IsCompleted(TutorialType type) const
{
    const int index = FindIndex(type);

    return index != -1 && objectives_[index].completed;
}

bool TutorialTaskManager::IsAllFinished() const
{
    if (objectives_.empty() || !checklist_) return false;

    for (const auto& objective : objectives_)
    {
        if (!objective.completed)
            return false;
    }

    return checklist_->IsAllStampFinished();
}

void TutorialTaskManager::OnPlayerAction(PlayerActionEvent event)
{
    switch (event)
    {
    case PlayerActionEvent::Jumped:
        Complete(TutorialType::Jump);
        break;
	case PlayerActionEvent::Attacked:
        Complete(TutorialType::Attack);
		break;
    case PlayerActionEvent::DoubleJumped:
        Complete(TutorialType::DoubleJump);
        break;
    case PlayerActionEvent::ReachedPillarTop:
        Complete(TutorialType::Climb);
        break;
    }
}

int TutorialTaskManager::FindIndex(TutorialType type) const
{
    for (int i = 0; i < static_cast<int>(objectives_.size()); ++i)
    {
        if (objectives_[i].type == type)
            return i;
    }

    return -1;
}