#pragma once

struct OutlineSettings
{
    float radius = 3.0f;

    float depthStrength = 20.0f;

    float normalStrength = 5.0f;

    float threshold = 0.1f;
};

inline OutlineSettings g_outlineSettings;