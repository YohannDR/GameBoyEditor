#version 460

layout(origin_upper_left) in vec4 gl_FragCoord;

out vec4 FragColor;

uniform highp sampler2D graphics;
uniform int gfxSize;

void main()
{
    const ivec2 pixelPosition = ivec2(gl_FragCoord.xy);

    const int tileId = pixelPosition.x / 8 + (pixelPosition.y / 8) * 16;

    if (tileId >= gfxSize / 16)
    discard;

    const int subPixelX = 7 - pixelPosition.x % 8;
    const int subPixelY = pixelPosition.y % 8;

    vec4 colors[5] = vec4[](
        vec4(1, 1, 1, 1),
        vec4(.75, .75, .75, 1),
        vec4(.5, .5, .5, 1),
        vec4(0, 0, 0, 1),
        vec4(0, 0, 0, 0)
    );

    const float offset = float(tileId * 8 + subPixelY) / float(gfxSize - 1);
    const vec2 data = texture(graphics, vec2(offset, 0)).rg;

    const uint plane0 = (uint(data.r * 255) & (1 << subPixelX)) >> subPixelX;
    const uint plane1 = (uint(data.g * 255) & (1 << subPixelX)) >> subPixelX;
    const uint paletteId = plane0 | (plane1 << 1);

    FragColor = colors[paletteId];
}
