#pragma once

class Motion
{
public:
	// 進行率の変化のさせ方
	enum class Easing
	{
		Linear,   // 一定の速さ
		EaseIn,   // ゆっくり始まり、徐々に速くなる
		EaseOut,  // 速く始まり、徐々にゆっくりになる
		EaseInOut // 始まりと終わりがゆっくり
	};

	Motion() = default;
	// from から to まで、duration 秒かけて動かす
	Motion(float from, float to, float duration, Easing easing = Easing::Linear);

	// モーションを最初から開始する
	void Start(float from, float to, float duration, Easing easing = Easing::Linear);
	// to から from に向かって逆再生を開始する
	void StartReverse();
	// 進行率を 0 に戻す
	void Reset();
	// deltaTime 秒ぶん進める
	void Update(float deltaTime);
	// deltaTime 秒ぶん進めて、現在の値を返す
	float Process(float deltaTime);
	// 進行率を増やしながら進める
	void Increase(float deltaTime);
	// 進行率を減らしながら進める
	void Decrease(float deltaTime);

	// イージング適用前の進行率（0～1）
	float GetProgress() const { return progress; }
	// イージング適用後の進行率（0～1）
	float GetEasedProgress() const;
	// from と to の間の現在値
	float GetValue() const;
	float GetDuration() const { return duration; }
	float GetElapsedTime() const { return progress * duration; }
	bool IsFinished() const { return finished; }
	// 実行中のイージングを変更する
	void SetEasing(Easing value) { easing = value; }

private:
	float from{};
	float to{ 1.0f };
	float duration{ 1.0f };
	float progress{};
	int direction{ 1 };
	Easing easing{ Easing::Linear };
	bool finished{};
};
