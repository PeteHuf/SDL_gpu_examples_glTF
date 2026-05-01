cbuffer UBO : register(b0, space1)
{
    float4x4 model : packoffset(c0);
    float4x4 view : packoffset(c4);
    float4x4 projection : packoffset(c8);
};

struct Input
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
};

struct Output
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
};

Output main(Input input)
{
    float4x4 transform = mul(projection, mul(view, model));
    
    Output output;
    output.TexCoord = input.TexCoord;
    output.Position = mul(transform, float4(input.Position, 1.0f));
    output.Normal = mul(model, float4(input.Normal, 1.0f));
    return output;
}
