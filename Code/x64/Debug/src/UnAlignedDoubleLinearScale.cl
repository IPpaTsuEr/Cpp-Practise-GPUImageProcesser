
typedef struct ImageInfoNode{
	int inx,iny;
	float sx, sy;
	int outx, outy;
	int inchannel,outchannel;
	}imageinfo;
	
void chechLimit(int2 * location,__global imageinfo * info){
	if(location->x<0)location->x=0;
	if(location->x>=info->inx)location->x=info->inx-1;
	if(location->y<0)location->y=0;
	if(location->y>=info->iny)location->y=info->iny-1;
}
__kernel void UnAlignedDoubleLinearScale(__global uchar * inputdata,__global imageinfo * ii , __global uchar * outputdata){
	
	int2 pt=(int2)(get_global_id(0),get_global_id(1));
	if(pt.x>ii->outx || pt.y > ii->outy)return;
	
	float2 spt=(float2)((float)pt.x/ii->sx,(float)pt.y/ii->sy);
	
	int2 tlp=convert_int2(spt);
	int2 trp=tlp+(int2)(1,0);
	int2 blp=tlp+(int2)(0,1);
	int2 brp=tlp+(int2)(1,1);
	
	chechLimit(&tlp,ii);
	chechLimit(&trp,ii);
	chechLimit(&blp,ii);
	chechLimit(&brp,ii);

	uchar4 tl=vload4(0,(inputdata+(tlp.y*ii->inchannel*ii->inx+tlp.x*ii->inchannel)));
	uchar4 tr=vload4(0,(inputdata+(trp.y*ii->inchannel*ii->inx+trp.x*ii->inchannel)));
	uchar4 bl=vload4(0,(inputdata+(blp.y*ii->inchannel*ii->inx+blp.x*ii->inchannel)));
	uchar4 br=vload4(0,(inputdata+(brp.y*ii->inchannel*ii->inx+brp.x*ii->inchannel)));
	
		float dx=spt.x-tlp.x;
		float dy=spt.y-tlp.y;
		float4 Qt=(float4)(dx*convert_float4(tl)+(1-dx)*convert_float4(tr));
		float4 Qb=(float4)(dx*convert_float4(bl)+(1-dx)*convert_float4(br));
		
	uchar4 result=convert_uchar4(dy*Qt+(1-dy)*Qb);
	if(ii->outchannel== 4)
	vstore4(result,0,(outputdata+(pt.y*ii->outchannel*ii->outx+pt.x*ii->outchannel)));
	if(ii->outchannel==3)
	vstore3(result.xyz,0,(outputdata+(pt.y*ii->outchannel*ii->outx+pt.x*ii->outchannel)));
	if(ii->outchannel==1)
	*(outputdata+(pt.y*ii->outchannel*ii->outx+pt.x*ii->outchannel))=convert_uchar(result.x*0.299f+result.y*0.587f+result.z*0.114f);
}