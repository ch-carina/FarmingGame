cbuffer WaterBuffer : register(b0)
{
    float time;
    float3 pad1;
    float2 worldOffset;
    float2 pad2;
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D water_texture : register(t0);
SamplerState water_sampler : register(s0);

static const float RIPPLE_SPEED = 1.5f;
static const float RIPPLE_STRENGTH = 0.015f;

float4 main(PS_IN ps_in) : SV_TARGET
{
    float2 uv = ps_in.uv;

    float2 worldUV = uv + worldOffset * 0.01f;

    uv.x += sin(worldUV.y * 20.0f + time * RIPPLE_SPEED) * RIPPLE_STRENGTH;
    uv.y += cos(worldUV.x * 18.0f + time * RIPPLE_SPEED * 0.8f) * RIPPLE_STRENGTH;

    float4 texColor = water_texture.Sample(water_sampler, uv);
    texColor.rgb = pow(texColor.rgb, 1.0f / 2.2f);
    return texColor;
}