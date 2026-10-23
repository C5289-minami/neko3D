#include "ModelCollision.h"
#include "DrawableObject.h"
#include "ResourceManager.h"
#include "ActorBase.h"
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

    void TestCollision()
    {
        using Surface = ModelCollision::Surface;
        ModelObject pillar;
        pillar.SetModelKey(ResourceKeys::Model_hashira_s);
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
        Check(touched && (stopped - oldPosition).Length() < radius * 0.02f,
            "Moving straight into wall stops without sideways acceleration");

        const Vec3 diagonal = inwardPosition + tangent * (radius * 0.5f) + Vec3::Up() * 0.1f;
        const Vec3 slid = collision.SlideAlongWalls(oldPosition, diagonal, modelOb, touched);
        Check(touched && std::abs(Vec3::Dot(slid - oldPosition, normal)) < radius * 0.02f &&
            Vec3::Dot(slid - oldPosition, tangent) > radius * 0.4f && Near(slid.y, diagonal.y),
            "Diagonal movement slides and preserves jump height");
        const Vec3 away = oldPosition + normal * radius;
        const Vec3 released = collision.SlideAlongWalls(oldPosition, away, modelOb, touched);
        Check(!touched && (released - away).Length() < 0.01f, "Movement away from wall is unrestricted");
        Check(collision.IsTouchingWalls(inwardPosition, modelOb), "Capsule contacts model mesh");

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

        Physics::RayHit ground{};
        Check(collision.RaycastDown({ 0, size.y * 2, 0 }, size.y * 4, ground),
            "Pillar supports landing on its top");

        collision.Clear();
        collision.AddModel(pillar, Surface::Wall);
        Check(collision.Refresh(), "Prepare Wall registration independently");
        const Vec3 wallStopped = collision.SlideAlongWalls(oldPosition, inwardPosition, modelOb, touched);
        Check(touched && (wallStopped - oldPosition).Length() < radius * 0.02f &&
            collision.IsTouchingWalls(inwardPosition, modelOb),
            "Wall blocks horizontal movement and reports contact");
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
        collision.AddModel(second, Surface::Pillar);
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
        collision.AddModel(second, Surface::Pillar);
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
    try { TestCollision(); }
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
