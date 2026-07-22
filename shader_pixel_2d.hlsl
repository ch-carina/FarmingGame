cbuffer ColorBuffer : register(b0)
{
    float4 color;
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    //float4 color : COLOR0;
    float2 uv : TEXCOORD;
};

Texture2D major_texture : register(t0); // テクスチャ
SamplerState major_sampler : register(s0); // サンプラー

float4 main(PS_IN ps_in) : SV_TARGET
{

    
    float4 texColor = major_texture.Sample(major_sampler, ps_in.uv) * color;

    texColor.rgb = pow(texColor.rgb, 1.0f / 2.2f);

    return texColor;

    //return major_texture.Sample(major_sampler, ps_in.uv)* color;
}
