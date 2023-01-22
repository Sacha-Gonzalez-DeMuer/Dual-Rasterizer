
//----------------------------------
//	Globals
//----------------------------------
float gPI = 3.14159265359f;
float gLightIntensity = 7.f;
float gShininess = 25.f;
float3 gLightDir = float3(0.577f, -0.577f, 0.577f);

float3 gCamForward : CamForward;
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gInvViewMatrix : InvViewMatrix;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : Glossiness;

RasterizerState gRasterizerState
{
    CullMode = back;
    FrontCounterClockwise = false;
};

BlendState gBlendState
{
	BlendEnable[0] = false;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = all;
	StencilEnable = true;
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
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : WORLD;
    float2 TextureUV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

//----------------------------------
//	BRDF Functions
//----------------------------------

float3 Phong(float3 specularColor, float gloss, float3 lightDir, float3 viewDir, float3 normal)
{
    float3 r = reflect(-gLightDir, normal);
    float specularStrength = saturate(dot(r, viewDir));
    float3 phongSpecular = 1.0f * pow(specularStrength, gShininess);
    float3 phong = specularColor * phongSpecular;

    return phong;
}

float3 Lambert(float reflectance, float3 diffuseColor)
{
    return (reflectance * diffuseColor) / gPI;
}

float3 PixelShading(VS_OUTPUT input, SamplerState sampleState)
{
    bool useSampledNormal = true;
    
	// Calculate view direction using OBN
    float3 viewDirection = normalize(input.WorldPosition.xyz - gInvViewMatrix[3].xyz);
    float3 normal = input.Normal;
    
	// Calculate observedArea
    float observedArea = saturate(dot(normal, -gLightDir));
    
    /// SAMPLED NORMAL
    if (useSampledNormal)
    {
        // Define tangent space matrix
        float3 binormal = cross(input.Normal, input.Tangent);
        float3x3 tangentSpace = float3x3(input.Tangent, binormal, input.Normal);
        
        // Sample normal
        float3 normalSample = gNormalMap.Sample(sampleState, input.TextureUV);
        
        normalSample = (2.0f * normalSample) - float3(1.0f, 1.0f, 1.0f);
        
        // Transform normal to local tangent space
        normal = normalize(mul(normalSample, tangentSpace));
    }


	// === BRDFs ===
	// diffuse
    float3 diffuse = gDiffuseMap.Sample(gSamplePoint, input.TextureUV);
    float3 lambertDiffuse = Lambert(1, diffuse) * gLightIntensity;
	
	// phong
    float specularColor = gSpecularMap.Sample(sampleState, input.TextureUV);
    float sampledGloss = gGlossinessMap.Sample(sampleState, input.TextureUV) * gShininess;
    

    float3 r = reflect(-gLightDir, normal);
    float specularStrength = saturate(dot(r, viewDirection));
    float3 phongSpecularReflect = 1.0f * pow(specularStrength, gShininess);
    
    float3 specular = specularColor * phongSpecularReflect;
    
    float3 phong = Phong(specularColor, sampledGloss, gLightDir, viewDirection, normal);
    
    float3 ambient = float3(.025f, .025f, .025f);
    
    return (lambertDiffuse * observedArea) + phong + ambient;
}

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
    output.Tangent = normalize(mul(input.Tangent, (float3x3) gWorldMatrix));
    return output;
}

//----------------------------------
//	Pixel Shader
//----------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	//gDiffuseMap.Sample(gSamplePoint, input.TextureUV);
    float3 pixelShade = PixelShading(input, gSamplePoint);
    return float4(pixelShade, 1);
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
