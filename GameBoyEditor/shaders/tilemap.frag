#version 460

layout(origin_upper_left) in vec4 gl_FragCoord;

out vec4 FragColor;

uniform highp sampler2D graphics;
uniform highp sampler2D tilemap;

uniform int gfxSize;
uniform int tilemapSize;
uniform int tilemapWidth;

uniform vec4 colors[4];

void main()
{
    const ivec2 pixelPosition = ivec2(gl_FragCoord.xy);

    const int tilemapIndex = pixelPosition.x / 8 + (pixelPosition.y / 8) * tilemapWidth;

    const float tilemapOffset = float(tilemapIndex) / float(tilemapSize - 1) - 0.0000001;
    const int tileId = int(texture(tilemap, vec2(tilemapOffset, 0)).r * 255);

    const int subPixelX = 7 - pixelPosition.x % 8;
    const int subPixelY = pixelPosition.y % 8;

    if (tileId >= gfxSize / 16)
    {
        const bool line0 = bool(subPixelX == subPixelY || 7 - subPixelX == subPixelY);
        FragColor = vec4(1, 0, 0, line0);
        return;
    }

    const float offset = float(tileId * 8 + subPixelY) / float(gfxSize - 1);
    const vec2 data = texture(graphics, vec2(offset, 0)).rg;

    const uint plane0 = (uint(data.r * 255) & (1 << subPixelX)) >> subPixelX;
    const uint plane1 = (uint(data.g * 255) & (1 << subPixelX)) >> subPixelX;
    const uint paletteId = plane0 | (plane1 << 1);

    FragColor = colors[paletteId];
}
