//  
struct VS_OUT
{
	float4 Position : POSITION;
	float4 Diffuse : COLOR0;
	float2 TexCoord0 : TEXCOORD0;
	float4 ChannelSelector : COLOR1;
};

//  
sampler FontTexture : register(s0);

// 
float4 main( VS_OUT In ) : COLOR0
{
//      // Fetch a texel from the font texture
	float4 FontTexel = tex2D( FontTexture, In.TexCoord0 );
//      
	if( dot( In.ChannelSelector, float4(1,1,1,1) ) )
	{
//          // Select the color from the channel
		float value = dot( FontTexel, In.ChannelSelector );
//          
//          // For white pixels, the high bit is 1 and the low
//          // bits are luminance, so r0.a will be > 0.5. For the
//          // RGB channel, we want to lop off the msb and shift
//          // the lower bits up one bit. This is simple to do
//          // with the _bx2 modifier. Since these pixels are
//          // opaque, we emit a 1 for the alpha channel (which
//          // is 0.5 x2 ).
//          
//          // For black pixels, the high bit is 0 and the low
//          // bits are alpha, so r0.a will be < 0.5. For the RGB
//          // channel, we emit zero. For the alpha channel, we
//          // just use the x2 modifier to scale up the low bits
//          // of the alpha.
		float4 Color;
		Color.rgb = ( value > 0.5f ? 2*value-1 : 0.0f );
		Color.a = 2 * ( value > 0.5f ? 1.0f : value );
//          
//          // Return the texture color modulated with the vertex
//          // color
		return Color * In.Diffuse;
	}
	else
	{
		return FontTexel * In.Diffuse;
	}
};
