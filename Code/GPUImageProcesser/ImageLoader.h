#pragma once
#include<FreeImage.h>
#include"Base.h"



class ImageLoader {
	CInfoCallback * callback;
	unsigned int width, height,channel;
	FIBITMAP * C;
	ImageInfoNode info;

	size_t limitSize;
public :
	ImageLoader(CInfoCallback * Muster,size_t MaxSize):callback(Muster),limitSize(MaxSize) {
		FreeImage_Initialise(true);
	}
	~ImageLoader() {
		callback = NULL;
		if (C != NULL)FreeImage_Unload(C);
		FreeImage_DeInitialise();
	}

	void clear() {
		if (C != NULL) {
			FreeImage_Unload(C);
			C = NULL;
		}
		width = 0;
		height = 0;
		channel = 0;
	}

	void to32(FIBITMAP ** map) {
		if (FreeImage_GetBPP(*map) != 32) {
			FIBITMAP * t = FreeImage_ConvertTo32Bits(*map);
			FreeImage_Unload(*map);
			*map = t;
		}
	}

	bool loadGif(const char* path,FREE_IMAGE_FORMAT * fif ) {
			
			FIMULTIBITMAP * mmap=FreeImage_OpenMultiBitmap(*fif, path, false, true,true, GIF_PLAYBACK);
			if (mmap == NULL) {
				callback->OnDataError(-234, std::stringstream() << "Load Image Fail [" << path << "] UnsuppotFormat");
				return false;
			}

			int count=FreeImage_GetPageCount(mmap);

			FIBITMAP * b1map = FreeImage_LockPage(mmap,(int)(count*0.15));
			

			width = FreeImage_GetWidth(b1map);
			height = FreeImage_GetHeight(b1map);
			channel = FreeImage_GetBPP(b1map);

			if (count == 1) {
				C= FreeImage_Allocate(width, height, channel);
				BYTE * Dst = FreeImage_GetBits(C);
				BYTE * Src1 = FreeImage_GetBits(b1map);
				memcpy(Dst, Src1, width* height * channel / 8);
			}
			else{
				FIBITMAP * b2map = FreeImage_LockPage(mmap,(int)(count*0.85));

				C = FreeImage_Allocate(width, height * 2, channel);
				
				BYTE * Dst = FreeImage_GetBits(C);
				BYTE * Src1 = FreeImage_GetBits(b1map);
				BYTE * Src2 = FreeImage_GetBits(b2map);

				memcpy(Dst, Src2, width* height * channel / 8);
				memcpy(Dst + width * height * channel / 8, Src1, width* height * channel / 8);
				
				height *= 2;
				
				FreeImage_UnlockPage(mmap, b2map,false);
			}
			
			FreeImage_UnlockPage(mmap, b1map,false);
			
			FreeImage_CloseMultiBitmap(mmap);

			return true;

	}

	BYTE * load(const char * path) {
		FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path);
		if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif)) {
			if (fif==FIF_GIF) {
				if (!loadGif(path, &fif))return NULL;
			}
			else {
				C=FreeImage_Load(fif, path);
				width=FreeImage_GetWidth(C);
				height=FreeImage_GetHeight(C);
				
			}
			to32(&C);
			channel = FreeImage_GetBPP(C) / 8;

			if (width* height* channel > limitSize) {
				callback->OnDataError(-235, std::stringstream() << "Get File [" << path << "] With:" << width << "*" << height << "*" << channel << ", Big Than Limit:"<<limitSize);
				return NULL;
			}
			if (width <= 1 || height <= 1) {
				callback->OnDataError(-235, std::stringstream() << "Get File [" << path << "] With:" << width << "*" << height << "*" << channel);
				return NULL;
			}
			
			return FreeImage_GetBits(C);
		}
		else {
			callback->OnDataError(-234, std::stringstream() << "Load Image Fail [" << path << "] UnsuppotFormat Or It Not A Image");
			return NULL;
		}

	}
	unsigned int getWidth() { return width; }
	unsigned int getHeight() { return height; }
	unsigned int getChannel() { return channel; }

	void getAdjustSize(ImageInfoNode * info, cl_uint heightLimit = 1024) {
		float scale = (float)info->inx / info->iny;
		unsigned int lasetSize = 0, selectSize = 0;
		int x = 1, y = 1, tmp = 0;

		while (x < info->inx) {
			tmp = (int)(x / scale);
			lasetSize = x * tmp;
			if (lasetSize <= heightLimit && lasetSize >= selectSize) {
				selectSize = lasetSize;
				y = tmp;
			}
			x++;
		}
		x = (int)(y * scale);
		info->outx = x;
		info->outy = y;
		info->sx = info->outx /(float)info->inx;
		info->sy = info->outy/(float)info->iny;
		this->info = *info;
	}
	std::stringstream getInfo() {
		std::stringstream ss;
		ss << "Image Size : " << info.inx << " * " << info.iny;
		ss << "   channel : " << channel;
		ss << "    Scale : " << info.sx << "/" << info.sy;
		ss << "   Out Size : " << info.outx << " * " << info.outy;
		return ss;
	}


	
	FIBITMAP * create(int width,int height,BYTE* data,int channel = 4) {
		FIBITMAP * map=FreeImage_Allocate(width, height, channel*8);

		BYTE * ptr = FreeImage_GetBits(map);
		int pitch = FreeImage_GetPitch(map);
		//无压缩图像会自动对齐，pitch代表了图像中对齐后每行的数据宽度
		//原始数据则是连续的，需按行写入数据才不会出现图像错乱的情况
		for (int y = 0; y < height; y++) {
			memcpy(ptr + (y * pitch),
				data + (y * width * channel),
				width * channel);
		}
		GetLastError();
		return map;
	}
	void getRawData(FIBITMAP * map,BYTE * outPut) {
		BYTE * ptr = FreeImage_GetBits(map);
		unsigned int  pitch = FreeImage_GetPitch(map);
		unsigned int  width = FreeImage_GetWidth(map);
		unsigned int  height= FreeImage_GetHeight(map);
		unsigned int  channel=FreeImage_GetBPP(map) / 8;
		outPut = (BYTE*)malloc(width*height*channel);
		for (unsigned int y = 0; y < height; y++) {
			memcpy(outPut +(y * width * channel),
				ptr + (y * pitch),
				width * channel);
		}
	}

	bool save(FIBITMAP * map,const char *path,FREE_IMAGE_FORMAT type=FIF_PNG){
		return FreeImage_Save(type, map, path);
	}
	bool saveToGray(FIBITMAP * map, const char *path) {
		bool result;
		if (FreeImage_GetBPP(map) > 8) {
			FIBITMAP * gmap = FreeImage_ConvertToGreyscale(map);
			result = FreeImage_Save(FIF_JPEG, gmap, path);
			FreeImage_Unload(gmap);
			FreeImage_Unload(map);
			return result;
		}
		else {
			result = FreeImage_Save(FIF_JPEG, map, path);
			FreeImage_Unload(map);
			return result;
		}
	}
	bool saveToJPG(FIBITMAP * map, const char *path) {
		bool result;
		if (FreeImage_GetBPP(map) > 24) {
			FIBITMAP * jmap = FreeImage_ConvertTo24Bits(map);
			result= FreeImage_Save(FIF_JPEG, jmap, path);
			FreeImage_Unload(jmap);
			FreeImage_Unload(map);
			return result;
		}
		else {
			
			result=FreeImage_Save(FIF_JPEG, map, path);
			FreeImage_Unload(map);
			return result;
		}
	}
	bool saveToPNG(FIBITMAP * map, const char *path) {
		bool result;
		if (FreeImage_GetBPP(map)  < 32) {
			FIBITMAP * jmap = FreeImage_ConvertTo32Bits(map);
			result = FreeImage_Save(FIF_PNG, jmap, path);
			FreeImage_Unload(jmap);
			FreeImage_Unload(map);
			return result;
		}
		else {
			result = FreeImage_Save(FIF_PNG, map, path);
			FreeImage_Unload(map);
			return result;
		}
	}

	 void getImageHead(cl_image_format * format, cl_image_desc * desc,float scale=1.0f) {
		format->image_channel_data_type = CL_UNSIGNED_INT8;
		format->image_channel_order = CL_RGBA;

		memset(desc, 0, sizeof(*desc));
		desc->image_width =(size_t)(width*scale);
		desc->image_height = (size_t)(height*scale);
		desc->image_type = CL_MEM_OBJECT_IMAGE2D;

	}
	 void getImageInfo(ImageInfoNode * info) {
		 info->inx = width;
		 info->iny = height;
		 info->inchannel = channel;
	 }
};