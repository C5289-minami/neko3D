// =============================
// Gameplay/Debug/Grid.h
// =============================
#pragma once

class Grid
{
public:
    Grid() = default;

    // accessors
    int GetHalfCount() const { return halfCount; }
    void SetHalfCount(int h) 
    {
		if (h < 1) h = 1;
        halfCount = h; 
    }

    float GetSpacing() const { return spacing; }
    void SetSpacing(float s) { spacing = s; }

    bool IsDrawXY() const { return drawXY; }
    void SetDrawXY(bool d) { drawXY = d; }

    bool IsDrawYZ() const { return drawYZ; }
    void SetDrawYZ(bool d) { drawYZ = d; }

    bool IsDrawZX() const { return drawZX; }
    void SetDrawZX(bool d) { drawZX = d; }

    // lifecycles
    void Draw() const;

private:
    int halfCount{ 5 };
    float spacing{ 100.0f };
    bool drawXY{ true };
    bool drawYZ{ true };
    bool drawZX{ true };
};
