#include "UIBase.h"
#include "ResourceManager.h"
#include <algorithm>

void UIBase::Init() 
{
	progressRate = 0.0f;
	timer = 0.0f;
	alpha = 0;
}

void UIBase::Update(float deltaTime)
{
    finAlpha = std::clamp(finAlpha, 0, 255);
	if (deltaTime < 0.0f) deltaTime = 0.0f;
	// 表示条件をチェック
	bool condition = displayCondition();
	
	// 表示条件が満たされていて、かつ現在の状態が非表示または消失モーション中の場合は出現モーションへ移行
	if (condition && (state == State::False || state == State::Hide)) {
		if (state == State::Hide) {
			timer = duration * (1.0f - progressRate);
			progressRate = 1.0f - progressRate;
		} else {
			timer = 0.0f;
			progressRate = 0.0f;
		}
		state = State::Do;
	}

	// 表示条件が満たされなくなった瞬間に消失モーションへ移行
	if (!condition && (state == State::Normal || state == State::Do)) {
		if (state == State::Do) {
			timer = duration * (1.0f - progressRate);
			progressRate = 1.0f - progressRate;
		} else {
			timer = 0.0f;
			progressRate = 0.0f;
		}
		state = State::Hide;
	}

	// 完全に非表示状態なら何もしない
	if (!condition && state == State::False) return;

	
	timer += deltaTime; // タイマーを更新
	if (duration > 0.0f)
		progressRate = timer / duration; // 進行率計算
	else 
		progressRate = 1.0f; // 継続時間が0以下の場合は常に100%の進行率とする

	if(progressRate >= 1.0f) {
		progressRate = 1.0f;
		if (state == State::Do) {
			state = State::Normal; // 出現モーションが完了したら通常状態に戻す
		} else if (state == State::Hide) {
			state = State::False; // 消失モーションが完了したら非表示に
		}
	}

	// 進行率と状態に応じてアルファ値を更新
	if (state == State::Do) {
		alpha = static_cast<int>(finAlpha * progressRate);
	} else if (state == State::Hide) {
		alpha = static_cast<int>(finAlpha * (1.0f - progressRate));
	} else if (state == State::Normal) {
		alpha = finAlpha;
	} else {
		alpha = 0;
	}

	UpdateStartMotion(); // モーションの更新

}

void UIBase::Draw()
{
	// False状態に遷移し終わったら描画を停止する
	if (state == State::False) return;
 const auto* sprite = RM().GetSprite(key);
	if (!sprite) return;
	DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
 sprite->Draw(position, DxPlus::Vec2(scale, scale));
	DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// 方向を入れると出てくるMotionを設定する関数
void UIBase::SetToMove(DxPlus::Vec2 dir)
{
	state = State::Do; // モーション実行中に設定
	timer = 0.0f; // タイマーをリセット
	Direction = dir; // モーションの方向を設定
	if(duration > 0)
		position = basePosition - Direction * MoveDistance; // 初期位置をモーションの方向に一定量ずらす
}

void UIBase::UpdateStartMotion()
{
	if (duration > 0.0f) {
		float t = progressRate;
		if (state == State::Do) {
			// 出現: 元の位置へ減速しながら近づく (Ease-Out)
			float easeT = t * (2.0f - t);
			position = (basePosition - Direction * MoveDistance) + Direction * (MoveDistance * easeT);
		} 
		else if (state == State::Hide) {
			// 消失: 逆方向へ加速しながら離れる (Ease-In)
			float easeT = t * t;
			position = basePosition - Direction * (MoveDistance * easeT);
		}
	}
}



