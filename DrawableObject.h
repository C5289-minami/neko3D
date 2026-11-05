    #pragma once

    #include <algorithm>
    #include <string>

    #include "DxPlus/Vector2.h"
    #include "Vector3.h"
    #include "Matrix.h"
    #include "Collision.h"
    #include "ModelType.h"

    class ActorBase;

    struct ImageObject
    {
        std::wstring imageKey;
        DxPlus::Vec2 position{};
        DxPlus::Vec2 scale{ 1.0f, 1.0f };
        DxPlus::Vec2 center{ 0.5f, 0.5f };
        float rotation{};

        DxPlus::Vec2 GetSize() const;
        void Draw(DxPlus::Vec2 masterPos = { 0.0f, 0.0f }) const;
    };

    struct ModelObject
    {
        bool collisionHighlighted{ false };
        const ActorBase* owner_{ nullptr };

        void Init();
        void Draw(Vec3 masterPos = {}) const;

        // JSON
        bool Load(const std::string& filePath = "./Data/Config/model_objects.json");
        bool Save(const std::string& filePath = "./Data/Config/model_objects.json") const;

        // 変換
        void ApplyTransform(Vec3 masterPos = {}) const;
        void ApplyCollisionColor() const;

        // 当たり判定
        Collision::Ellipsoid GetHitEllipsoid() const;
        Vec3 GetSize() const { return GetHitEllipsoid().radii * 2.0f; }

        // 親子関係
        void SetOwner(const ActorBase& owner) { owner_ = &owner; }
        ModelObject WithParentTransform(const Vec3& parentPosition, const Vec3& parentRotation) const;

        // アルファ値
        void SetAlpha(float value) { alpha = (std::max)(0.0f, (std::min)(1.0f, value)); }
        void GetAlpha(float& outValue) const { outValue = alpha; }
        float GetAlpha() const { return alpha; }
        void FadeTo(const ActorBase& player, float speed, float deltaTime);

        // モデルキー
        const std::wstring& GetModelKey() const { return modelKey; }
        void SetModelKey(const std::wstring& value) { modelKey = value; }

        // 変換行列は必要なときに生成
        Matrix4 GetTransformMatrix() const
        {
            return Matrix4::CreateScale(scale)
                * Matrix4::CreateRotationXYZ(rotation)
                * Matrix4::CreateTranslation(position);
        }

        Matrix4 GetWorldMatrix() const;

        // 位置
        Vec3 GetPosition() const { return position; }
        void SetPosition(const Vec3& value) { position = value; }

        // スケール
        Vec3 GetScale() const { return scale; }
        void SetScale(const Vec3& value) { scale = value; }

        // 回転
        Vec3 GetRotation() const { return rotation; }
        void SetRotation(const Vec3& value) { rotation = value; }

        // 回転中心
        Vec3 GetCenter() const { return center; }
        void SetCenter(const Vec3& value) { center = value; }

        // オブジェクト識別情報
        const std::string& GetObjectName() const { return objectName; }
        void SetObjectName(const std::string& value) { objectName = value; }
        const std::string& GetObjectId() const { return objectId; }
        void SetObjectId(const std::string& value) { objectId = value; }

        // モデルの種類
        ModelType GetModelType() const { return modelType_; }
        void SetModelType(ModelType type) { modelType_ = type; }

    private:
        // 内部管理
        std::string objectName;
        std::string objectId;
        std::wstring modelKey;

        // 変換情報
        Vec3 position{};
        Vec3 scale{ 1.0f, 1.0f, 1.0f };
        Vec3 rotation{};

        // 回転中心
        Vec3 center{};

        // 描画設定
        float alpha{ 1.0f };
        ModelType modelType_ = ModelType::Model;
    };