"""Generate a periodic ocean normal map: RGB encodes tangent XYZ, green is +V.

The texture is linear data, not sRGB. Decode with rgb * 2 - 1 and normalize.
Use repeat wrapping. Blue is the outward surface-normal component, NOT world Z.
Requires numpy and Pillow. No external input assets.
"""
from pathlib import Path
import numpy as np
from PIL import Image


def generate(size=1024):
    rng = np.random.default_rng(7351)
    v, u = np.mgrid[0:size, 0:size] / size
    du = np.zeros((size, size))
    dv = np.zeros_like(du)
    # Integer spatial frequencies make the continuous field tile periodically.
    for _ in range(96):
        kx, ky = rng.integers(-28, 29, size=2)
        frequency = np.hypot(kx, ky)
        if frequency < 3:
            continue
        amplitude = frequency ** -1.7
        phase = 2 * np.pi * (kx * u + ky * v) + rng.uniform(0, 2 * np.pi)
        gradient = amplitude * 2 * np.pi * np.cos(phase)
        du += kx * gradient
        dv += ky * gradient
    strength = 0.38 / np.sqrt(np.mean(du * du + dv * dv))
    normals = np.stack((-du * strength, -dv * strength, np.ones_like(du)), axis=-1)
    normals /= np.linalg.norm(normals, axis=-1, keepdims=True)
    pixels = np.rint((normals * 0.5 + 0.5) * 255).astype(np.uint8)
    decoded = pixels.astype(float) / 255 * 2 - 1
    assert np.max(np.abs(np.linalg.norm(decoded, axis=-1) - 1)) < 0.008
    # Wrap-neighbor differences should match ordinary neighboring differences.
    for axis in (0, 1):
        seam = np.mean(np.abs(np.take(decoded, 0, axis=axis) - np.take(decoded, -1, axis=axis)))
        interior = np.mean(np.abs(np.diff(decoded, axis=axis)))
        assert seam < interior * 2, (axis, seam, interior)
    return Image.fromarray(pixels)


if __name__ == "__main__":
    output = Path(__file__).resolve().parents[1] / "assets/textures/ocean-normal-placeholder.png"
    if output.exists():
        raise SystemExit(f"Refusing to overwrite existing asset: {output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    generate().save(output)
    print(output)
