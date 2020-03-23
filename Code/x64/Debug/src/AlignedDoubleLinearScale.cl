__constant sampler_t sampler=
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP|
	CLK_FILTER_NEAREST;
	
typedef struct ImageScaleNode{
	float X;
	float Y;
	}ImageScale;
	
__kernel void AlignedDoubleLinearScale(__read_only image2d_t original ,__global ImageScale * scale, __write_only image2d_t output){
	int2 outputcoord = (int2){get_global_id(0),get_global_id(1)};
	float2 Qcoord = (float2){outputcoord.x/scale->X,outputcoord.y/scale->Y};
	int srcH= get_image_height(original);
	int srcW= get_image_width(original);
	uint4 P;
	int2 tlp=convert_int2(Qcoord);/*(int2){outputcoord.x/(*scale),outputcoord.y/(*scale)};*/
	if(Qcoord.x<=0||Qcoord.y<=0||Qcoord.x>=srcW||Qcoord.y>=srcH){
		if(tlp.x<0 ){
			tlp.x=0;
		}
		if( tlp.y<0 ){
			tlp.y=0;
		}
		if( tlp.x>=srcW ){
			tlp.x=srcW-1;
		}
		if(tlp.y >=srcH){
			tlp.y=srcH-1;	
		}
		P=read_imageui(original,tlp);
	}
	else{
		
		uint4 tl=read_imageui(original,tlp);
		uint4 tr=read_imageui(original,tlp+(int2)(-1,0));
		
		uint4 bl=read_imageui(original,tlp+(int2)(0,-1));
		uint4 br=read_imageui(original,tlp+(int2)(-1,-1));
		
		float dx=Qcoord.x-tlp.x;
		float dy=Qcoord.y-tlp.y;
		
		float4 Qt=(float4)(dx*convert_float4(tl)+(1-dx)*convert_float4(tr));
		
		float4 Qb=(float4)(dx*convert_float4(bl)+(1-dx)*convert_float4(br));
		
		P=convert_uint4(dy*Qt+(1-dy)*Qb);
	}
	write_imageui(output,outputcoord,P);
	
}