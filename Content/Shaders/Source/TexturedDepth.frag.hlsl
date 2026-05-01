Texture2D<float4> Texture : register(t0, space2);
SamplerState Sampler : register(s0, space2);

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

//Output main(float2 TexCoord : TEXCOORD0, float4 Position : SV_Position, float3 Normal : NORMAL0)
Output main(float2 TexCoord : TEXCOORD0, float4 Position : SV_Position)
{
    float3 lightDir = normalize(float3(-1, -1, -1));
    
    Output result;
    result.Color = Texture.Sample(Sampler, TexCoord);
    //result.Color *= dot(lightDir, Normal);
    //result.Color = float4(1, 0, 0, 1); // PRECHECKIN: remove
    result.Depth = LinearizeDepth(Position.z, NearPlane, FarPlane);
    return result;
}
