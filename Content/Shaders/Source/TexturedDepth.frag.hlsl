Texture2D<float4> ColorTexture : register(t0, space2);
SamplerState ColorSampler : register(s0, space2);

Texture2D<float4> NormalTexture : register(t1, space2);
SamplerState NormalSampler : register(s1, space2);

cbuffer UBO : register(b0, space3)
{
    float NearPlane;
    float FarPlane;
};

struct Output
{
    float4 Color : SV_Target0;
    float Depth : SV_Depth;
};

float LinearizeDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0;
    return ((2.0 * near * far) / (far + near - z * (far - near))) / far;
}

Output main(float4 Position : SV_Position, float4 Normal : NORMAL0, float2 TexCoord : TEXCOORD0)
{
    float3 lightDir = normalize(float3(1, 0.5, 1));
    
    float3 normalSample = normalize(NormalTexture.Sample(NormalSampler, TexCoord).xyz);
    
    Output result;
    result.Color = ColorTexture.Sample(ColorSampler, TexCoord);
    result.Color *= 0.5 + 0.5 * dot(lightDir, normalize(normalize(Normal.xyz) + normalSample));
    result.Depth = LinearizeDepth(Position.z, NearPlane, FarPlane);
    return result;
}
