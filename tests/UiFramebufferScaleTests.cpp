#include "../UiFramebufferScale.h"

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
    int checks = 0;

    void Check(bool condition, const char* message)
    {
        ++checks;
        if (!condition) throw std::runtime_error(message);
    }

    bool Near(float actual, float expected, float tolerance = 0.005f)
    {
        return std::isfinite(actual) && std::abs(actual - expected) <= tolerance;
    }

    struct Point { float x, y; };
    struct Rectangle { Point minimum, maximum; };
    struct Surface { float clientWidth, clientHeight, targetWidth, targetHeight; };

    // The renderer projects Win32/desktop UI coordinates into the bound RTV,
    // then DxLib presents that RTV stretched to the client. Hit testing still
    // uses the original Win32/desktop coordinates and must match this result.
    Point RenderAndPresent(Point logical, Point displayOrigin,
        const Surface& surface, UiFramebufferScale scale)
    {
        const Point framebuffer{
            (logical.x - displayOrigin.x) * scale.x,
            (logical.y - displayOrigin.y) * scale.y
        };
        return {
            displayOrigin.x + framebuffer.x * surface.clientWidth / surface.targetWidth,
            displayOrigin.y + framebuffer.y * surface.clientHeight / surface.targetHeight
        };
    }

    bool Contains(const Rectangle& rectangle, Point point)
    {
        return point.x >= rectangle.minimum.x && point.y >= rectangle.minimum.y &&
            point.x < rectangle.maximum.x && point.y < rectangle.maximum.y;
    }

    void TestResizeAndHitTesting()
    {
        const std::array<Surface, 8> surfaces{ {
            { 1920, 1080, 1920, 1080 }, // Native.
            { 1344, 756, 1920, 1080 },  // Debug 70%.
            { 1632, 918, 1920, 1080 },  // Presentation 85%.
            { 960, 540, 1920, 1080 },   // Half-size preset.
            { 2560, 1440, 1920, 1080 }, // Client larger than the game target.
            { 1400, 900, 1920, 1080 },  // Nonuniform resize: independent axes.
            { 2400, 1350, 3840, 2160 }, // DPI-like physical sizes.
            { 800, 900, 1600, 900 }     // Horizontal-only scaling.
        } };
        const std::array<Point, 2> origins{ { { 0, 0 }, { -1800, 75 } } };
        for (const auto& surface : surfaces)
        {
            const auto scale = CalculateUiFramebufferScale(surface.clientWidth,
                surface.clientHeight, surface.targetWidth, surface.targetHeight);
            Check(std::isfinite(scale.x) && scale.x > 0 &&
                std::isfinite(scale.y) && scale.y > 0, "Valid surfaces produce usable positive scales");
            for (Point origin : origins)
            {
                const std::array<Point, 8> points{ {
                    { origin.x, origin.y },
                    { origin.x + 1, origin.y + 1 },
                    { origin.x + surface.clientWidth, origin.y },
                    { origin.x, origin.y + surface.clientHeight },
                    { origin.x + surface.clientWidth, origin.y + surface.clientHeight },
                    { origin.x + surface.clientWidth - 1, origin.y + surface.clientHeight - 1 },
                    { origin.x + surface.clientWidth * 0.9f, origin.y + surface.clientHeight * 0.8f },
                    { origin.x + surface.clientWidth * 0.5f, origin.y + surface.clientHeight * 0.5f }
                } };
                for (Point logical : points)
                {
                    const Point visible = RenderAndPresent(logical, origin, surface, scale);
                    Check(Near(visible.x, logical.x) && Near(visible.y, logical.y),
                        "Rendered points match hit-test coordinates at all edges and desktop origins");
                }

                const Rectangle clip{
                    { origin.x + surface.clientWidth * 0.75f, origin.y + surface.clientHeight * 0.7f },
                    { origin.x + surface.clientWidth - 1, origin.y + surface.clientHeight - 1 }
                };
                const Rectangle visibleClip{
                    RenderAndPresent(clip.minimum, origin, surface, scale),
                    RenderAndPresent(clip.maximum, origin, surface, scale)
                };
                Check(Near(visibleClip.minimum.x, clip.minimum.x) && Near(visibleClip.minimum.y, clip.minimum.y),
                    "The clip rectangle starts at the same visible and logical position");
                Check(Near(visibleClip.maximum.x, clip.maximum.x) && Near(visibleClip.maximum.y, clip.maximum.y),
                    "Far-right and bottom clipping do not cut off visible controls");
                const Point inside{
                    (clip.minimum.x + clip.maximum.x) * 0.5f,
                    (clip.minimum.y + clip.maximum.y) * 0.5f
                };
                Check(Contains(visibleClip, inside) && Contains(clip, inside),
                    "A visible control's interior remains inside its hit rectangle");
                const Point outside{ clip.maximum.x + 1, clip.maximum.y + 1 };
                Check(!Contains(visibleClip, outside) && !Contains(clip, outside),
                    "Points past the visible right and bottom edges remain outside the hit rectangle");
            }
        }
    }

    void TestOldMismatchAndIndependentWindows()
    {
        const Surface presentation{ 1632, 918, 1920, 1080 };
        const Point control{ 800, 600 };
        const Point oldVisible = RenderAndPresent(control, {}, presentation, {});
        Check(Near(oldVisible.x, 680) && Near(oldVisible.y, 510),
            "The old unit-scale rendering reproduces the 85% click offset");
        Check(std::abs(control.x - oldVisible.x) > 100 && std::abs(control.y - oldVisible.y) > 80,
            "The reproduced visible control and original hit-test position are substantially separated");
        const auto scale = CalculateUiFramebufferScale(1632, 918, 1920, 1080);
        const Point corrected = RenderAndPresent(control, {}, presentation, scale);
        Check(Near(corrected.x, control.x) && Near(corrected.y, control.y),
            "Main-window correction removes the reproduced click offset");

        const Surface detached{ 640, 480, 640, 480 };
        const Point desktopOrigin{ 2100, 100 };
        const Point detachedControl{ 2600, 500 };
        const auto detachedScale = CalculateUiFramebufferScale(640, 480, 640, 480);
        const Point detachedVisible = RenderAndPresent(detachedControl, desktopOrigin, detached, detachedScale);
        Check(Near(detachedScale.x, 1) && Near(detachedScale.y, 1) &&
            Near(detachedVisible.x, detachedControl.x) && Near(detachedVisible.y, detachedControl.y),
            "A detached native-size swap chain retains independent one-to-one desktop coordinates");
        Check(!Near(RenderAndPresent(detachedControl, desktopOrigin, detached, scale).x, detachedControl.x),
            "A main-window scale must not leak into the detached window");
    }

    void TestInvalidDimensions()
    {
        const float nan = std::numeric_limits<float>::quiet_NaN();
        const float infinity = std::numeric_limits<float>::infinity();
        const float maximum = std::numeric_limits<float>::max();
        const float minimum = std::numeric_limits<float>::min();
        for (float invalid : { 0.0f, -1.0f, nan, infinity, -infinity })
        {
            const std::array<UiFramebufferScale, 4> scales{ {
                CalculateUiFramebufferScale(invalid, 1080, 1920, 1080),
                CalculateUiFramebufferScale(1920, invalid, 1920, 1080),
                CalculateUiFramebufferScale(1920, 1080, invalid, 1080),
                CalculateUiFramebufferScale(1920, 1080, 1920, invalid)
            } };
            for (const auto& scale : scales)
                Check(scale.x == 1 && scale.y == 1,
                    "Zero, minimized, negative and nonfinite dimensions safely preserve unit scale");
        }
        for (const auto& scale : {
            CalculateUiFramebufferScale(minimum, 1080, maximum, 1080),
            CalculateUiFramebufferScale(1920, minimum, 1920, maximum),
            CalculateUiFramebufferScale(minimum, minimum, maximum, maximum) })
            Check(scale.x == 1 && scale.y == 1, "Overflowing ratios cannot reach the renderer");
        const auto restored = CalculateUiFramebufferScale(1920, 1080, 1920, 1080);
        Check(restored.x == 1 && restored.y == 1,
            "A restored normal-size window uses unit scale after invalid-size frames");
    }
}

int main()
{
    try
    {
        TestResizeAndHitTesting();
        TestOldMismatchAndIndependentWindows();
        TestInvalidDimensions();
        std::cout << "PASS: " << checks << " UI framebuffer checks\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL after " << checks << " checks: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
