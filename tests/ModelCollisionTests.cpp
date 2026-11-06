#include "ModelCollision.h"
#include "DrawableObject.h"
#include "ResourceManager.h"
#include "ActorBase.h"
#include "DxConv.h"
#include "Player.h"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>

// Link with the game's objects except main.obj. No keyboard input or rendering is needed.
bool g_raise_imgui_viewports = false;

namespace
{
    void Check(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
        std::cout << "PASS: " << message << std::endl;
    }

    bool Near(float actual, float expected, float tolerance = 0.02f)
    {
        return std::abs(actual - expected) <= tolerance;
    }

    void CenterModel(ModelObject& model, const Vec3& center)
    {
        model.SetPosition({});
        model.SetPosition(center - model.GetHitEllipsoid().center);
    }

    class Parent : public ActorBase
    {
    public:
        Parent() : ActorBase("CollisionTestParent") {}
        void Init() override {}
        void Reset() override {}
        void Update(float) override {}
        void Draw() const override {}
    };

    void TestStageRims()
    {
        ModelObject stage;
        stage.SetModelKey(ResourceKeys::Model_Stage);
        stage.SetScale({ 2, 2, 2 });
        ModelCollision collision;
        collision.AddFallRecoveryFloor(stage);
        Check(collision.Refresh(), "Load the real arena floor mesh");

        const int mesh = RM().GetModel(stage.GetModelKey());
        Check(MV1SetupCollInfo(mesh, -1, 8, 8, 8) == 0, "Prepare arena overlap checks");
        const auto overlapsFloor = [mesh](const Player& player)
        {
            const auto body = player.GetHitSphere();
            const auto result = MV1CollCheck_Sphere(mesh, -1, DxConv::ToVECTOR(body.center), body.radius - 0.02f);
            const bool overlaps = result.HitNum > 0;
            MV1CollResultPolyDimTerminate(result);
            return overlaps;
        };

        Physics::RayHit flat{};
        Check(collision.RaycastDown({ 1000, 1000, 0 }, 2000, flat), "Find flat arena floor");
        for (const float outward : { -1.0f, 1.0f })
        {
            Physics::RayHit slope{};
            bool foundSlope = false;
            for (float offset = 100; offset <= 460; offset += 2)
            {
                if (collision.RaycastDown({ 1000 + outward * offset, 1000, 0 }, 2000, slope) &&
                    slope.normal.y > 0.15f && slope.normal.y < 0.97f)
                {
                    foundSlope = true;
                    break;
                }
            }
            Check(foundSlope, outward < 0 ? "Find the inner slope" : "Find the outer slope");

            Player player;
            player.Init();
            player.SetControlLocked(true);
            player.SetStartPosition(flat.point);
            player.SetPosition(slope.point + Vec3::Up() * 50.0f);
            player.ApplyKnockback(-Vec3::Up(), 1500.0f, 0.1f);
            player.Update(1.0f / 30.0f, collision, {});
            Check(!overlapsFloor(player) && player.GetPosition().y > slope.point.y,
                "A fast landing on the rim keeps the player's body out of the mesh");

            player.SetStartPosition(flat.point);
            player.Update(1.0f / 60.0f, collision, {});
            Check(player.IsGrounded(), "Player can stand on the flat arena floor");
            player.ApplyKnockback({ outward, 0, 0 }, 1050.0f, 0.45f);
            bool recovered = false;
            bool overlapped = false;
            for (int frame = 0; frame < 60; ++frame)
            {
                player.Update(1.0f / 60.0f, collision, {});
                overlapped |= overlapsFloor(player);
                if (!player.IsGrounded() && player.GetVelocity().LengthSq() == 0.0f)
                {
                    recovered = true;
                    break;
                }
            }
            Check(!overlapped, "Moving over the rim keeps the player's body out of the mesh");
            Check(recovered, "Leaving the arena at the rim still triggers fall recovery");
        }
        MV1TerminateCollInfo(mesh, -1);
    }

    void TestCollision()
    {
        using Surface = ModelCollision::Surface;
        ModelObject pillar;
        pillar.SetModelKey(ResourceKeys::Model_PillarS);
        CenterModel(pillar, {});
        ModelCollision collision;
        collision.AddModel(pillar, Surface::Pillar);
        Check(collision.Refresh(), "Prepare real pillar mesh before drawing");
        Physics::RayHit initial{};
        Check(collision.Raycast({ 10000, 0, 0 }, {}, initial), "Initial ray before size extraction");
        const Vec3 size = collision.GetSize(pillar);
        Check(size.x > 0 && size.y > 0 && size.z > 0, "Extract model width, height and depth");
        std::cout << "Pillar size: " << size.x << ", " << size.y << ", " << size.z << std::endl;

        Physics::RayHit side{};
        Check(collision.Raycast({ size.x * 2, 0, 0 }, {}, side), "Ray hits pillar side");
        Vec3 normal = Vec3(side.normal.x, 0, side.normal.z).Normalized();
        if (normal.x < 0) normal = -normal;
        const Vec3 tangent{ -normal.z, 0, normal.x };
        const float radius = (std::min)(size.x, size.y) * 0.02f;
        const ModelCollision::ModelOb modelOb{ radius, radius * 3, radius * 0.01f };
        const Vec3 oldPosition = side.point + normal * (radius * 1.2f) - Vec3::Up() * (modelOb.height * 0.5f);
        const Vec3 inwardPosition = oldPosition - normal * (radius * 0.8f);
        bool touched = false;
        const Vec3 stopped = collision.SlideAlongWalls(oldPosition, inwardPosition, modelOb, touched);

        // Check(touched && (stopped - oldPosition).Length() < radius * 0.02f,
        //     "Moving straight into wall stops without sideways acceleration");
 
        Check(!touched && (stopped - inwardPosition).Length() < 0.01f,
            "Pillar does not stop horizontal movement");

        const Vec3 diagonal = inwardPosition + tangent * (radius * 0.5f) + Vec3::Up() * 0.1f;
        const Vec3 slid = collision.SlideAlongWalls(oldPosition, diagonal, modelOb, touched);
        // Check(touched && std::abs(Vec3::Dot(slid - oldPosition, normal)) < radius * 0.02f &&
        //     Vec3::Dot(slid - oldPosition, tangent) > radius * 0.4f && Near(slid.y, diagonal.y),
        //     "Diagonal movement slides and preserves jump height");

        Check(!touched && (slid - diagonal).Length() < 0.01f,
            "Pillar leaves diagonal movement and jump height unchanged");
        const Vec3 away = oldPosition + normal * radius;
        const Vec3 released = collision.SlideAlongWalls(oldPosition, away, modelOb, touched);
        Check(!touched && (released - away).Length() < 0.01f, "Movement away from wall is unrestricted");
        Check(collision.IsTouchingWalls(inwardPosition, modelOb), "Capsule contacts model mesh");
        Check(!collision.IsTouchingWalls(away, modelOb), "Contact becomes false after leaving pillar");

        Vec3 wallNormal{};
        float top{};
        Check(collision.FindWall(oldPosition, modelOb, wallNormal, -normal, &top, radius),
            "Find climbable side and top height");
        Check(Vec3::Dot(wallNormal, normal) > 0.95f && top > oldPosition.y,
            "Climb normal points toward moving body");
        const Vec3 atTop{ inwardPosition.x, size.y * 0.5f, inwardPosition.z };
        Check(!collision.FindWall(atTop, modelOb, wallNormal, -normal, &top, radius),
            "Standing on top is not treated as climbing a side");
        Check(collision.FindWall(atTop, modelOb, wallNormal, -normal, &top, radius, modelOb.height),
            "Climbing can keep contact at top edge");

        // Use the real player dimensions, including the sphere limit height == 2 * radius.
        const ModelCollision::ModelOb playerBody{ Const::PLAYER_RADIUS, Const::PLAYER_HEIGHT, Const::PLAYER_SKIN };
        Check(playerBody.IsValid(), "Short player dimensions remain valid");
        const Vec3 playerOld = side.point + normal * (playerBody.radius * 1.2f) - Vec3::Up() * (playerBody.height * 0.5f);
        const Vec3 playerInward = playerOld - normal * (playerBody.radius * 0.8f);
        const Vec3 playerStopped = collision.SlideAlongWalls(playerOld, playerInward, playerBody, touched);
        // Check(touched && (playerStopped - playerOld).Length() < 0.02f,
        //     "Short player still stops at pillar side");
        Check(!touched && (playerStopped - playerInward).Length() < 0.01f,
            "Player passes through pillar while contact remains detectable");
        Check(collision.IsTouchingWalls(playerInward, playerBody), "Short player reports pillar contact");
        Check(collision.FindWall(playerOld, playerBody, wallNormal, -normal, &top, Const::PLAYER_CLIMB_DETECTION_MARGIN),
            "Short player can detect climbable pillar side");

        Physics::RayHit ground{};
        // Check(collision.RaycastDown({ 0, size.y * 2, 0 }, size.y * 4, ground),
        //     "Pillar supports landing on its top");
        Check(!collision.RaycastDown({ 0, size.y * 2, 0 }, size.y * 4, ground),
            "Pillar no longer corrects landing height");
        Check(!collision.RaycastUp({ 0, -size.y, 0 }, size.y * 2, ground),
            "Pillar no longer blocks jumps from below");

        Player contactPlayer;
        contactPlayer.Init();
        contactPlayer.SetPosition(playerOld);
        contactPlayer.Update(0.0f, collision);
        Check(contactPlayer.IsTouchingWall() && contactPlayer.GetCurrentState() != PlayerStateType::Climbing,
            "Player contact stays true without movement input or auto-climbing");
        contactPlayer.SetGround(true);
        contactPlayer.JumpAction();
        contactPlayer.Update(0.01f, collision);
        Check(Near(contactPlayer.GetPosition().x, playerOld.x) &&
            contactPlayer.GetPosition().y > playerOld.y &&
            contactPlayer.GetCurrentState() != PlayerStateType::Climbing,
            "Jump remains a normal jump without attraction or automatic ascent");
        contactPlayer.SetPosition(playerOld + normal * playerBody.radius * 4);
        contactPlayer.Update(0.0f, collision);
        Check(!contactPlayer.IsTouchingWall(), "Player contact returns to false away from pillar");

        collision.Clear();
        collision.AddModel(pillar, Surface::Wall);
        Check(collision.Refresh(), "Prepare Wall registration independently");
        const Vec3 wallStopped = collision.SlideAlongWalls(oldPosition, inwardPosition, modelOb, touched);
        Check(touched && (wallStopped - oldPosition).Length() < radius * 0.02f &&
            collision.IsTouchingWalls(inwardPosition, modelOb),
            "Wall blocks horizontal movement and reports contact");
        Physics::RayHit ceiling{};
        const float ceilingY = -size.y * 0.5f;
        Check(collision.RaycastUp({ 0, ceilingY - 100, 0 }, 200, ceiling) && Near(ceiling.point.y, ceilingY, 0.1f),
            "Upward rays detect underside of Wall models");
        Check(!collision.RaycastUp({ 0, ceilingY - 100, 0 }, 0, ceiling) &&
            !collision.RaycastUp({ 0, ceilingY - 100, 0 }, -1, ceiling),
            "Upward rays reject non-positive distances");

        Player player;
        player.Init();
        player.SetGravityEnabled(false);
        // The head moves through the underside during this frame.
        player.SetPosition({ 0, ceilingY - Const::PLAYER_HEIGHT - 10, 0 });
        player.SetVerticalSpeed(200);
        player.Update(0.1f, collision);
        Check(Near(player.GetPosition().y + Const::PLAYER_HEIGHT, ceilingY - Const::PLAYER_SKIN, 0.1f),
            "Player stops below ceiling even after head crosses it in one frame");
        const float stoppedY = player.GetPosition().y;
        player.Update(0.1f, collision);
        Check(Near(player.GetPosition().y, stoppedY), "Ceiling collision cancels upward velocity");
        // A ceiling beyond the next head position must not stop the jump early.
        player.SetPosition({ 0, ceilingY - Const::PLAYER_HEIGHT - 30, 0 });
        player.SetVerticalSpeed(200);
        player.Update(0.1f, collision);
        Check(Near(player.GetPosition().y + Const::PLAYER_HEIGHT, ceilingY - 10, 0.1f),
            "Player continues upward when next head position is still below ceiling");
        player.Update(0.1f, collision);
        Check(Near(player.GetPosition().y + Const::PLAYER_HEIGHT, ceilingY - Const::PLAYER_SKIN, 0.1f),
            "Player hits ceiling on the following frame");

        Check(!collision.RaycastDown({ 0, size.y * 2, 0 }, size.y * 4, ground),
            "Wall does not support landing on its top");
        Check(collision.Raycast({ 0, size.y * 2, 0 }, { 0, -size.y * 2, 0 }, ground),
            "General rays still detect Wall models");
        Check(!collision.FindWall(oldPosition, modelOb, wallNormal, -normal, &top, radius),
            "Wall cannot be climbed");

        collision.Clear();
        collision.AddModel(pillar, Surface::Floor);
        Check(collision.Refresh(), "Prepare Floor registration independently");
        const Vec3 unrestricted = collision.SlideAlongWalls(oldPosition, inwardPosition, modelOb, touched);
        Check(!touched && (unrestricted - inwardPosition).Length() < 0.01f,
            "Floor does not block horizontal movement");
        Check(!collision.IsTouchingWalls(inwardPosition, modelOb) &&
            !collision.FindWall(oldPosition, modelOb, wallNormal, -normal, &top, radius),
            "Floor does not report walls or climbing surfaces");
        Check(collision.RaycastDown({ 0, size.y * 2, 0 }, size.y * 4, ground),
            "Floor supports landing");
        Check(Near(ground.point.y, size.y * 0.5f, 0.1f), "Landing uses real mesh top");

        ModelObject second;
        second.SetModelKey(pillar.GetModelKey());
        second.SetScale({ 2, 2, 2 });
        const float spacing = size.x * 5;
        CenterModel(second, { spacing, size.y * 2, 0 });
        // collision.AddModel(second, Surface::Pillar);
        collision.AddModel(second, Surface::Floor);
        Check(collision.Refresh(), "Register another instance of the same source model");
        const Vec3 doubled = collision.GetSize(second);
        Check(Near(doubled.x, size.x * 2, 0.1f) && Near(doubled.y, size.y * 2, 0.1f),
            "Per-instance scale changes collision size");
        Check(collision.RaycastDown({ 0, size.y * 5, 0 }, size.y * 10, ground) &&
            Near(ground.point.y, size.y * 0.5f, 0.1f), "First instance retains its own placement");
        Check(collision.RaycastDown({ spacing, size.y * 5, 0 }, size.y * 10, ground) &&
            Near(ground.point.y, size.y * 3, 0.1f), "Second instance uses its independent placement");

        CenterModel(second, { 0, size.y * 2, 0 });
        ModelCollision wallAboveFloor;
        wallAboveFloor.AddModel(pillar, Surface::Floor);
        wallAboveFloor.AddModel(second, Surface::Wall);
        Check(wallAboveFloor.Refresh() && wallAboveFloor.RaycastDown({ 0, size.y * 5, 0 }, size.y * 10, ground) &&
            Near(ground.point.y, size.y * 0.5f, 0.1f),
            "Landing skips Wall above Floor and finds Floor below");
        Check(wallAboveFloor.Raycast({ 0, size.y * 5, 0 }, { 0, -size.y * 5, 0 }, ground) &&
            Near(ground.point.y, size.y * 3, 0.1f),
            "General rays select the nearer Wall model above Floor");
        // collision.AddModel(second, Surface::Pillar);
        collision.AddModel(second, Surface::Floor);
        Check(collision.Refresh() && collision.RaycastDown({ 0, size.y * 5, 0 }, size.y * 10, ground) &&
            Near(ground.point.y, size.y * 3, 0.1f), "Refresh moves collision and ray selects nearest model");
        Check(!collision.RaycastDown({ spacing, size.y * 5, 0 }, size.y * 10, ground),
            "Moving a model removes collision from its old placement");

        second.SetModelKey(L"MissingCollisionTestModel");
        Check(!collision.Refresh(), "Missing source is reported without using a stale collision");
        Check(collision.RaycastDown({ 0, size.y * 5, 0 }, size.y * 10, ground) &&
            Near(ground.point.y, size.y * 0.5f, 0.1f), "Other registered models remain usable after failure");
        second.SetModelKey(pillar.GetModelKey());
        Check(collision.Refresh(), "Source replacement recovers on the next refresh");

        Parent parent;
        pillar.SetOwner(parent);
        parent.SetPosition({ spacing * 2, 100, 200 });
        parent.SetYaw(0.5f);
        ModelCollision anotherScene;
        anotherScene.AddModel(pillar, Surface::Floor);
        Check(anotherScene.Refresh(), "Another scene can register the same model independently");
        const Vec3 center = pillar.GetHitEllipsoid().center;
        Check(anotherScene.RaycastDown(center + Vec3::Up() * size.y * 2, size.y * 4, ground) &&
            Near(ground.point.y, center.y + size.y * 0.5f, 0.2f), "Parent translation and rotation are reflected");

        bool invalidTouched = true;
        const Vec3 invalidResult = collision.SlideAlongWalls({}, { 1, 2, 3 }, {}, invalidTouched);
        Check(!invalidTouched && Near(invalidResult.y, 2), "Invalid body dimensions safely skip collision");
        collision.Clear();
        Check(!collision.RaycastDown({ 0, size.y * 5, 0 }, size.y * 10, ground), "Clear removes registered collision");
        Check(collision.Refresh(), "Empty collision list is usable");

        ModelObject room;
        room.SetModelKey(ResourceKeys::Model_Room);
        CenterModel(room, {});
        ModelCollision tutorial;
        tutorial.AddModel(room, Surface::Floor);
        Check(tutorial.Refresh(), "Prepare tutorial room as Floor");
        const Vec3 roomSize = tutorial.GetSize(room);
        Physics::RayHit roomSide{};
        Check(tutorial.Raycast({}, { roomSize.x * 2, 0, 0 }, roomSide), "Ray reaches room wall from inside");
        Vec3 insideNormal = Vec3(roomSide.normal.x, 0, roomSide.normal.z).Normalized();
        if (Vec3::Dot(-roomSide.point, insideNormal) < 0) insideNormal = -insideNormal;
        const Vec3 roomOld = roomSide.point + insideNormal * (modelOb.radius * 1.2f) - Vec3::Up() * modelOb.height * 0.5f;
        const Vec3 roomDesired = roomOld - insideNormal * (modelOb.radius * 0.8f);
        const Vec3 roomUnrestricted = tutorial.SlideAlongWalls(roomOld, roomDesired, modelOb, touched);
        Check(!touched && (roomUnrestricted - roomDesired).Length() < 0.01f,
            "Registering room as Floor does not enable Wall collision");
        tutorial.AddModel(room, Surface::Wall);
        tutorial.AddModel(room, Surface::Wall);
        Check(tutorial.Refresh(), "Register room as Wall separately without duplicate entries");
        Check(tutorial.RaycastDown({}, roomSize.y * 2, ground),
            "Separate Wall registration leaves existing Floor registration usable");
        Check(!tutorial.FindWall(roomOld, modelOb, wallNormal, -insideNormal),
            "Separate Floor and Wall registrations do not enable Pillar climbing");
        const Vec3 roomStopped = tutorial.SlideAlongWalls(roomOld, roomDesired, modelOb, touched);
        Check(touched && (roomStopped - roomOld).Length() < modelOb.radius * 0.02f,
            "Room walls block movement from inside regardless of mesh normal orientation");
        room.SetScale({ 2, 2, 2 });
        Check(tutorial.Refresh() && Near(tutorial.GetSize(room).x, roomSize.x * 2, 0.2f),
            "Changing scale after registration updates mesh and size");
    }
}

int main()
{
    const auto assetDirectory = std::filesystem::current_path();
    SetUnhandledExceptionFilter([](EXCEPTION_POINTERS* error) -> LONG
    {
        std::cerr << "Native exception: " << std::hex << error->ExceptionRecord->ExceptionCode << std::endl;
        return EXCEPTION_EXECUTE_HANDLER;
    });
    SetDoubleStartValidFlag(TRUE);
    SetApplicationLogSaveDirectory((assetDirectory / "obj/CollisionTests").c_str());
    SetWindowVisibleFlag(FALSE);
    ss::SSPlatformInit();
    if (DxPlus::Initialize(640, 480, true) < 0) return 2;
    std::filesystem::current_path(assetDirectory);
    RM().LoadAll();
    int result = 0;
    try
    {
        TestStageRims();
        TestCollision();
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << std::endl;
        result = 1;
    }
    RM().UnloadAll();
    DxPlus::Shutdown();
    ss::SSPlatformRelease();
    return result;
}
