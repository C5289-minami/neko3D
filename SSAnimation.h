#pragma once

#include <algorithm> 
#include <cmath>     
#include "./SSPlayer/SS6Player.h"

namespace SSAnimation
{
	class SSGauge {
	public:
		SSGauge(ss::Player& player) : player_(&player) {}
		SSGauge() = default;

		void SetPlayer(ss::Player& player) { player_ = &player; }

		void Reset()
		{
			currentProgress_ = 0.0f;
			currentFrame_ = 0.0f;
			player_->setFrameNo(0);
		}

		void Update(float deltaTime, float progress)
		{
			if (!player_) return;

			int totalFrames = player_->getTotalFrame();
			if (totalFrames <= 0) return;

			// 有効な最大フレーム番号は総フレーム数 - 1
			float maxFrame = static_cast<float>(totalFrames - 1);

			currentProgress_ = std::clamp(progress, 0.0f, 1.0f);
			float targetFrame = currentProgress_ * maxFrame;

			// 目標フレームへ向けて滑らかに移動
			float frameDifference = targetFrame - currentFrame_;

			// 差が極小になったら直接目標値にセット
			if (std::abs(frameDifference) < 0.01f) {
				currentFrame_ = targetFrame;
			}
			else {
				// deltaTime * 追従速度。1.0を超えないようclamp
				float speedFactor = std::clamp(deltaTime * 5.0f, 0.0f, 1.0f);
				currentFrame_ += frameDifference * speedFactor;
			}

			// 四捨五入して int にキャストしアニメーションに適用
			// roundは小数点以下を四捨五入する
			player_->setFrameNo(static_cast<int>(std::round(currentFrame_)));
		}

	private:
		ss::Player* player_ = nullptr;
		float currentProgress_ = 0.0f;
		float currentFrame_ = 0.0f; 
	};
}