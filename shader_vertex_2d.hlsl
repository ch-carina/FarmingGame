cbuffer MatrixBuffer : register(b0)
{
    float4x4 mtx;
};

cbuffer UVMatrixBuffer : register(b1)
{
    float4x4 mtx_uv;
};

struct VS_IN
{
    float4 posL : POSITION0;
    //float4 color : COLOR0;
    float2 uv : TEXCOORD;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    //float4 color : COLOR0;
    float2 uv : TEXCOORD;
};

// :POSTION0 semantics
VS_OUT main(VS_IN vs_in)
{
    VS_OUT vs_out;
    
    vs_out.posH = mul(vs_in.posL, mtx);
    //vs_out.color = vs_in.color;
    vs_out.uv = mul(float4(vs_in.uv, 0, 1), mtx_uv).xy;
    
    return vs_out;
}
