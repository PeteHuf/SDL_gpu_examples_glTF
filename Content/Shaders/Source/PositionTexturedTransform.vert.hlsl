cbuffer UBO : register(b0, space1)
{
    float4x4 world : packoffset(c0);
    float4x4 view : packoffset(c4);
    float4x4 projection : packoffset(c8);
};

struct Input
{
    float3 Position : TEXCOORD0;
    //float3 Normal : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
};

struct Output
{
    float2 TexCoord : TEXCOORD0;
    float4 Position : SV_Position;
    //float3 Normal : NORMAL0;
};

Output main(Input input)
{
    float4x4 transform = mul(projection, mul(view, world));
    
    Output output;
    output.TexCoord = input.TexCoord;
    output.Position = mul(transform, float4(input.Position, 1.0f));
    //output.Normal = mul((float3x3) world, input.Normal);
    return output;
}
