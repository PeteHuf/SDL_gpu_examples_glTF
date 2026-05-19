cbuffer UBO : register(b0, space1)
{
    float4x4 model : packoffset(c0);
    float4x4 view : packoffset(c4);
    float4x4 projection : packoffset(c8);
};

#define MAX_NUM_JOINTS 128

struct MeshShaderDataBlock
{
    float4x4 mat;
    float4x4 jointMatrix[MAX_NUM_JOINTS];
    uint jointCount;
};

cbuffer PushConstants : register(b1, space1) // PRECHECKIN: equivalent to push constants, mesh data should be moved to StorageBuffer, equiv to SSBO
{
    //MeshShaderDataBlock meshData;
    int meshIndex;
};


StructuredBuffer<MeshShaderDataBlock> MeshData : register(t0, space0);


struct Input
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    uint4 Joint : BLENDINDICES0;
    float4 Weight : BLENDWEIGHT0;
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
    
    
    
    if (MeshData[meshIndex].jointCount > 0)
    {
		// Mesh is skinned
        float4x4 skinMat =
			input.Weight.x * MeshData[meshIndex].jointMatrix[input.Joint.x] +
			input.Weight.y * MeshData[meshIndex].jointMatrix[input.Joint.y] +
			input.Weight.z * MeshData[meshIndex].jointMatrix[input.Joint.z] +
			input.Weight.w * MeshData[meshIndex].jointMatrix[input.Joint.w];

        //output.Position = output.Position = mul(mul(transform, skinMat), float4(input.Position, 1.0f));
        
        output.Position = output.Position = mul(mul(transform, mul(MeshData[meshIndex].mat, skinMat)), float4(input.Position, 1.0f));

        //output.Position = mul(mul(mul(model, meshData.mat), skinMat), float4(input.Position, 1.0));
        //outNormal = normalize(transpose(inverse(mat3(model * meshData.mat * skinMat))) * input.Normal);
    }
    else
    {
        output.Position = mul(mul(transform, MeshData[meshIndex].mat), float4(input.Position, 1.0));
        //locPos = model * meshData.mat * float4(input.Position, 1.0);
        
        //outNormal = normalize(transpose(inverse(mat3(model * meshData.mat))) * input.Normal);
    }
    
    
    
    return output;
}
