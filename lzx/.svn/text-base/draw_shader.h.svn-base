static const char g_strDrawVertexhader[] =
    "												"
    " struct VS_IN									"
    "												"
    " {												"
    "     float2 ObjPos   : POSITION;				"  // Object space position 
	"     float2 Tex	  : TEXCOORD0;				"
    "     float4 Color    : COLOR0;					"  // Vertex color        
    " };											"
    "												"
    " struct VS_OUT									"
    " {												"
    "     float4 ObjPos   : POSITION;				"  // Projected space position 
	"     float2 Tex	  : TEXCOORD0;				"
	"     float4 Color    : COLOR0;					"
	" };											"
    "												"
    " VS_OUT main( VS_IN In )						"
    " {												"
    "	VS_OUT Out;									"
    "	Out.ObjPos.xy = In.ObjPos.xy;						"
	"	Out.ObjPos.z = 0.0;							"
	"	Out.ObjPos.w = 1.0;							"
	"	Out.Tex = In.Tex;							"
    "	Out.Color = In.Color;						"  // Projected space and 
    "	return Out;									"  // Transfer color
    " }												";


//--------------------------------------------------------------------------------------
// Pixel shader
//--------------------------------------------------------------------------------------
static const char *         g_strDrawPixelShader =
	"struct PS_IN"
"{"
"float2 Tex   : TEXCOORD0;"
"float4 Color : COLOR0;"
"};"
"sampler s : register(s0);"
"float4 main( PS_IN In ) : COLOR0"
"{"
//"return In.Color * tex2D( s, In.Tex );"
//" float4 Color = In.Color * tex2D( s, In.Tex );"
" float4 Color = In.Color;"
" Color.a = 1.0f; "
"return Color;"
"}";

//--------------------------------------------------------------------------------------
// Pixel shader
//--------------------------------------------------------------------------------------
static const char *         g_strDrawPixelTexturedShader =
	"struct PS_IN"
"{"
"float2 Tex   : TEXCOORD0;"
"float4 Color : COLOR0;"
"};"
"sampler s : register(s0);"
"float4 main( PS_IN In ) : COLOR0"
"{"
"return tex2D( s, In.Tex );"
"}";