#pragma once
#include <DxLib.h>
#include <algorithm> // std::clamp用

class Volume
{
public:
	Volume() = default;
	~Volume() = default;
	// シングルトンのインスタンス取得
	static Volume& GetInstance() {
		static Volume instance;
		return instance;
	}
	Volume(const Volume&) = delete;
	Volume& operator=(const Volume&) = delete;

	void Reset() {
		BGM = 1.0f;
		SE = 1.0f;
	}

	void SetBGM(float vol) { BGM = std::clamp(vol, 0.0f, 1.0f); }
	void SetSE(float vol) { SE = std::clamp(vol, 0.0f, 1.0f); }

	void AddBGM(float vol) { BGM = std::clamp(BGM + vol, 0.0f, 1.0f); }
	void AddSE(float vol) { SE = std::clamp(SE + vol, 0.0f, 1.0f); }

	void GetVolume(float& bgm, float& se) const {bgm = BGM; se = SE; }

	// BGMの再生
	void PlayBGM(int handle) {
		if (handle < 0) return;

		DxLib::ChangeVolumeSoundMem(static_cast<int>(BGM * 255.0f), handle);
		DxLib::PlaySoundMem(handle, DX_PLAYTYPE_LOOP);
	}

	// SEの再生
	void PlaySE(int handle) {
		if (handle < 0) return;
		DxLib::ChangeVolumeSoundMem(static_cast<int>(SE * 255.0f), handle);
		DxLib::PlaySoundMem(handle, DX_PLAYTYPE_BACK);
	}

	void StopBGM(int handle) {
		if (handle < 0) return;
		DxLib::StopSoundMem(handle);
	}

	void StopSE(int handle) {
		if (handle < 0) return;
		DxLib::StopSoundMem(handle);
	}

private:
	float BGM = 1.0f;
	float SE = 1.0f;
};
inline Volume& V() { return Volume::GetInstance(); } // ショートカット