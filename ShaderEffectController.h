#pragma once
// shader effect controller

// 設定値を取得・変更するためのAPIのような役割。
// アウトラインの太さを変更したり、トゥーンの影の強さを設定したりする。

#include "OutlineRenderer.h"

struct ShaderEffectSettings
{
    bool toonEnabled = true;
    bool outlineEnabled = false;
    bool fullScreenEffectEnabled = false;
};

class ShaderEffectController final
{
public:
    bool Init(int width, int height)
    {
        return renderer_.Init(width, height);
    }

    void Release() noexcept
    {
        renderer_.Release();
    }

    void SetEnabled(bool enabled) noexcept
    {
        enabled_ = enabled;
    }

    bool IsEnabled() const noexcept
    {
        return enabled_;
    }

    ShaderEffectSettings& GetSettings() noexcept
    {
        return settings_;
    }

    const ShaderEffectSettings& GetSettings() const noexcept
    {
        return settings_;
    }

    template<typename SceneDraw, typename DepthDraw>
    void Render(SceneDraw&& sceneDraw, DepthDraw&& depthDraw) const
    {
        renderer_.SetOutlineEnabled(
            enabled_ && settings_.outlineEnabled);
      /*  renderer_.SetFullScreenEffectEnabled(
            enabled_ && settings_.fullScreenEffectEnabled);*/
		renderer_.SetFullScreenEffectEnabled(false); // 全画面エフェクトは無効化
        renderer_.Render(
            [&] { sceneDraw(enabled_ && settings_.toonEnabled); },
            static_cast<DepthDraw&&>(depthDraw));
    }

private:
 mutable OutlineRenderer renderer_{};
    ShaderEffectSettings settings_{};
    bool enabled_ = true;
};
