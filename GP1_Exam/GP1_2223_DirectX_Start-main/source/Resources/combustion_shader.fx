
//----------------------------------
//	Globals
//----------------------------------
float gPI = 3.14159265359f;
float gLightIntensity = 7.f;
float gShininess = 25.f;
float3 gLightDir = float3(0.577f, -0.577f, 0.577f);

float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gInvViewMatrix : InvViewMatrix;

Texture2D gDiffuseMap : DiffuseMap;

RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockwise = true;
};

BlendState gBlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;

    //StencilReadMask = 0x0F;
    //StencilWriteMask = 0x0F;

    //FrontFaceStencilFunc = always;
    //BackFaceStencilFunc = always;

    //FrontFaceStencilDepthFail = keep;
    //BackFaceStencilDepthFail = keep;

    //FrontFaceStencilPass = keep;
    //BackFaceStencilPass = keep;

    //FrontFaceStencilFail = keep;
    //BackFaceStencilFail = keep;
};

SamplerState gSamplePoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState gSampleLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState gSampleAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

//----------------------------------
//	Input/Output Structs
//----------------------------------

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 WorldPosition : WORLD;
    float2 TextureUV : TEXCOORD;
    float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : WORLD;
    float2 TextureUV : TEXCOORD;
    float3 Normal : NORMAL;
};

//----------------------------------
//	Vertex Shader
//----------------------------------

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
    output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
    output.TextureUV = input.TextureUV;
    output.Normal = normalize(mul(input.Normal, (float3x3) gWorldMatrix));
    return output;
}

//----------------------------------
//	Pixel Shader
//----------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(gSamplePoint, input.TextureUV);
}

//----------------------------------
//	Technique
//----------------------------------

technique11 DefaultTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}


