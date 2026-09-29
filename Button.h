#pragma once
#include "DxPlus/DxPlus.h"
#include <functional>

class Button {
public:
    Button(const DxPlus::Vec2& pos, float w, float h, std::function<void()> callback)
        : position(pos), onClick(callback) {
        alpha = 128; // 初期状態は半透明
        center.x = w / 2.0f;
        center.y = h / 2.0f;
    }
    Button(const DxPlus::Vec2& pos, int key, std::function<void()> callback)
        : position(pos), onClick(callback) {
        DxLib::GetGraphSizeF(key, &center.x, &center.y);
        center /= 2.0f;
        alpha = 128; // 初期状態は半透明    
        sprite = key;
    }
    enum class TriggerMode {
        Down, // 押した瞬間 (GetButtonDown)
        Up,   // 離した瞬間 (GetButtonUp)
        Hold  // 長押しOK (GetButton)
    };
    void SetTriggerMode(TriggerMode mode) { triggerMode = mode; }

    // 毎フレーム呼ぶ：判定と状態更新をセットで行う
    void Update(const DxPlus::Vec2& mousePos, bool mousePush) {
        if (!isDraw) return; // 描画されていないときは更新しない
        float scaledCenterX = center.x * scale;
        float scaledCenterY = center.y * scale;

        isHovered = (mousePos.x >= position.x - scaledCenterX && mousePos.x <= position.x + scaledCenterX &&
            mousePos.y >= position.y - scaledCenterY && mousePos.y <= position.y + scaledCenterY);

        // 触ったときに描画する画像があるか？
        if (hoverSprite != -1) {
            alpha = 255;
        }
        else {
            alpha = isHovered ? 128 : 255;
        }
        bool isTriggered = false;

        // ボタンの上でクリックを「開始」したかを記録する（Up判定で誤爆を防ぐため）
        if (isHovered && mousePush && !prevMousePush) {
            isPressedOnButton = true;
        }
        // マウスを離したらリセット
        if (!mousePush) {
            isPressedOnButton = false;
        }




        // 指定されたモードに合わせて判定
        switch (triggerMode) {
        case TriggerMode::Down:
            // ホバー中で、今回押されて、前回押されていなかったら（押した瞬間）
            if (isHovered && mousePush && !prevMousePush) {
                isTriggered = true;
            }
            break;
        case TriggerMode::Up:
            // ホバー中で、このボタン上で押し始めていて、今回離されたら（離した瞬間）
            if (isHovered && isPressedOnButton && !mousePush && prevMousePush) {
                isTriggered = true;
            }
            break;
        case TriggerMode::Hold:bool isTriggered = false;

        // ボタンの上でクリックを「開始」したかを記録する（Up判定で誤爆を防ぐため）
        if (isHovered && mousePush && !prevMousePush) {
            isPressedOnButton = true;
        }
        // マウスを離したらリセット
        if (!mousePush) {
            isPressedOnButton = false;
        }

        // 指定されたモードに合わせて判定
        switch (triggerMode) {
        case TriggerMode::Down:
            // ホバー中で、今回押されて、前回押されていなかったら（押した瞬間）
            if (isHovered && mousePush && !prevMousePush) {
                isTriggered = true;
            }
            break;
        case TriggerMode::Up:
            // ホバー中で、このボタン上で押し始めていて、今回離されたら（離した瞬間）
            if (isHovered && isPressedOnButton && !mousePush && prevMousePush) {
                isTriggered = true;
            }
            break;
        case TriggerMode::Hold:
            // ホバー中で、押されていたら常に（長押し）
            if (isHovered && mousePush) {
                isTriggered = true;
            }
            break;
        }

        // 条件を満たしていたらコールバック実行
        if (isTriggered) {
            if (onClick) onClick();
        }

        // 次のフレームのために今の入力状態を保存
        prevMousePush = mousePush;
            // ホバー中で、押されていたら常に（長押し）
            if (isHovered && mousePush) {
                isTriggered = true;
            }
            break;
        }

        // 条件を満たしていたらコールバック実行
        if (isTriggered) {
            if (onClick) onClick();
        }

        // 次のフレームのために今の入力状態を保存
        prevMousePush = mousePush;
    }

    void Draw() const {
        if (!isDraw) return; // 描画されていないときは描画しない 

        int drawSprite = sprite;
        if (isHovered && hoverSprite != -1) {
            drawSprite = hoverSprite;
        }

        DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DxPlus::Sprite::Draw(drawSprite,
            position, DxPlus::Vec2(scale, scale),
            center);
        DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    }
    void DebugDraw() const {
        float scaledCenterX = center.x * scale;
        float scaledCenterY = center.y * scale;

        DxLib::DrawBoxAA(position.x - center.x,
            position.y - center.y,
            position.x + center.x,
            position.y + center.y,
            DxLib::GetColor(255, 0, 0), true, 2.0f);
    }
    void SetDraw(bool draw) { isDraw = draw; }
    void SetAlpha(int a) { alpha = a; }
    void SetHoverSprite(int s) { hoverSprite = s; }
    void SetScale(float s) { scale = s; }
    void SetPosition(const DxPlus::Vec2& pos) { position = pos; }

private:
    DxPlus::Vec2 position{};
    float scale{ 1.0f };
    int alpha{}, sprite{ -1 };
    int hoverSprite{ -1 };// ホバーされたときのスプライトID
    DxPlus::Vec2 center{};
    bool isHovered{ false };
    std::function<void()> onClick{};
    bool isDraw{ true };

    TriggerMode triggerMode{ TriggerMode::Down }; // デフォルトは「押した瞬間」
    bool prevMousePush{ false };      // 前回のフレームでマウスが押されていたか
    bool isPressedOnButton{ false };  // このボタンの上で押し始めたか
};