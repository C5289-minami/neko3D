    #pragma once

    #include <string>
    #include <utility>
    #include "Matrix.h"

    class ActorBase
    {
    public:
        explicit ActorBase(std::string id) : actorID_(std::move(id)) {}
        virtual ~ActorBase() = default;

        virtual void Init() = 0;
        virtual void Reset() = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void Draw() const = 0;

        // 位置
        const Vec3& GetPosition() const { return position_; }
        void SetPosition(const Vec3& position) { position_ = position; }

        // 回転
        Vec3 GetRotation() const { return { pitch_, yaw_, 0.0f }; }
        void SetRotation(const Vec3& rotation) { pitch_ = rotation.x; yaw_ = rotation.y; }

        float GetYaw() const { return yaw_; }
        void SetYaw(float yaw) { yaw_ = yaw; }

        float GetPitch() const { return pitch_; }
        void SetPitch(float pitch) { pitch_ = pitch; }

        const std::string& GetIdentifier() const { return actorID_; }
	    void SetIdentifier(const std::string& id) { actorID_ = id; }

        // 親子関係
        void SetOwner(ActorBase& owner) { owner_ = &owner; }
        ActorBase* GetOwner() const { return owner_; }

        // ローカル行列
        Matrix4 GetLocalMatrix() const
        {
            return Matrix4::CreateRotationXYZ(GetRotation())
                * Matrix4::CreateTranslation(position_);
        }

        // ワールド行列
        Matrix4 GetWorldMatrix() const { 
            if (!owner_) return GetLocalMatrix();
            return GetLocalMatrix() * owner_->GetWorldMatrix();
	    }

        // ワールド座標・ワールド回転
        Vec3 GetWorldPosition() const;
        Vec3 GetWorldRotation() const;

    protected:
        std::string actorID_;
        Vec3 position_{};
        float yaw_{};
        float pitch_{};
        ActorBase* owner_{ nullptr };
    };