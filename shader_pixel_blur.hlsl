/*--------------------------------------------------
Typical Gausian Blur affect
--------------------------------------------------*/
cbuffer BlurBuffer : register(b0)
{
    float2 texelSize; // 1/screenWidth, 1/screenHeight
    float2 direction; // (1,0) for horizontal pass, (0,1) for vertical pass
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D major_texture : register(t0);
SamplerState major_sampler : register(s0);

static const float weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

float4 main(PS_IN ps_in) : SV_TARGET
{
    float2 offset = texelSize * direction;

    float3 result = major_texture.Sample(major_sampler, ps_in.uv).rgb * weights[0];

    for (int i = 1; i < 5; i++)
    {
        result += major_texture.Sample(major_sampler, ps_in.uv + offset * i).rgb * weights[i];
        result += major_texture.Sample(major_sampler, ps_in.uv - offset * i).rgb * weights[i];
    }

    return float4(result, 1.0f);
}