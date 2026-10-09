#pragma once

#include "DxLib.h"

// ===============================
// ---使い方---
// 1. Init() で初期化する。
// 2. BeginScenePass() と EndScenePass() で囲んで、
//    シーンの描画を行う。
// 3. BeginDepthPass() と EndDepthPass() で囲んで、
//    シーンの深度・法線の描画を行う。
// 4. RenderPostProcess() を呼び出して、
//    スクリーン空間アウトライン描画を行う。
// ===============================

/*==============================
// // アウトラインだけ
outlineRenderer_.SetOutlineEnabled(true);
outlineRenderer_.SetFullScreenEffectEnabled(false);

// 全画面エフェクトだけ
outlineRenderer_.SetOutlineEnabled(false);
outlineRenderer_.SetFullScreenEffectEnabled(true);

// 両方無効
outlineRenderer_.SetOutlineEnabled(false);
outlineRenderer_.SetFullScreenEffectEnabled(false);
================================= */


// スクリーン空間アウトライン描画を管理するクラス
class ScreenSpaceOutlineRenderer final
{
public:
	ScreenSpaceOutlineRenderer() = default;
	~ScreenSpaceOutlineRenderer();

	ScreenSpaceOutlineRenderer(const ScreenSpaceOutlineRenderer&) = delete;
	ScreenSpaceOutlineRenderer& operator=(
		const ScreenSpaceOutlineRenderer&
		) = delete;

	bool Init(int width, int height);
	void Release() noexcept;

	void BeginScenePass() const;
	void EndScenePass() const;

	void BeginDepthPass() const;
	void EndDepthPass() const;

	void RenderPostProcess() const;

	bool IsInitialized() const noexcept;

	template<typename SceneDraw, typename DepthDraw>
	void Render(SceneDraw&& sceneDraw, DepthDraw&& depthDraw) const
	{
		BeginScenePass();
		sceneDraw();
		EndScenePass();

		// アウトラインが有効なときだけ深度・法線を描画
		if (outlineEnabled_)
		{
			BeginDepthPass();
			depthDraw();
			EndDepthPass();
		}

		RenderPostProcess();
	}
	// 全画面エフェクト用
	void RenderFullScreenShader() const;

	// シーン描画から全画面エフェクトまでを実行
	template<typename SceneDraw>
	void RenderFullScreenEffect(SceneDraw&& sceneDraw) const
	{
		BeginScenePass();
		sceneDraw();
		EndScenePass();

		RenderFullScreenShader();
	}

	void SetOutlineEnabled(bool enabled) noexcept
	{
		outlineEnabled_ = enabled;
	}

	void SetFullScreenEffectEnabled(bool enabled) noexcept
	{
		fullScreenEffectEnabled_ = enabled;
	}

private:
	int width_{};
	int height_{};

	int depthPixelShader_{ -1 };
	int postProcessPixelShader_{ -1 };
	int fullScreenPixelShader_{ -1 };

	int sceneBuffer_{ -1 };
	int depthBuffer_{ -1 };
	int settingsConstantBuffer_{ -1 };

	VERTEX2DSHADER fullscreenVertices_[6]{};

	void CreateFullscreenVertices();

	// アウトライン処理後の画像を保持するバッファ
	int outlineBuffer_ = -1;

	// エフェクトの有効・無効
	bool outlineEnabled_ = true;
	bool fullScreenEffectEnabled_ = false;

	// 各エフェクトの描画処理
	void RenderOutlinePass(int destinationScreen) const;
	void RenderFullScreenPass(int sourceTexture) const;
};