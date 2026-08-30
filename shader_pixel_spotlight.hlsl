cbuffer SpotlightBuffer : register(b0)
{
    float2 lightCenter; // player position, screen pixels
    float innerRadius; // fully lit within this distance
    float outerRadius; // fully dark beyond this distance
    float darkAlpha; // max darkness (0..1)
    float2 screenSize; // SCREEN_WIDTH, SCREEN_HEIGHT
    float pad;
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PS_IN ps_in) : SV_TARGET
{
    float2 screenPos = ps_in.uv * screenSize;
    float dist = length(screenPos - lightCenter);
    float darkness = smoothstep(innerRadius, outerRadius, dist) * darkAlpha;

    return float4(0.0f, 0.0f, 0.0f, darkness);
}