#ifndef _TEXTURE_HELPER_H_
#define _TEXTURE_HELPER_H_

#define JPEG_SUPPORT 1
#define PNG_SUPPORT 1

typedef struct
{
	void *data;
	unsigned int width;
	unsigned int height;
	unsigned int id;
	unsigned int Bpp;
	unsigned int wrap_s;
	unsigned int wrap_t;
	unsigned int wrap_u;
	unsigned int filter_min;
	unsigned int filter_mag;
	unsigned int target;
} MyTexture2D;

#include "mymath.h"

void writeppm_fp16( const char *filename, fp16 *fp16_buffer, int w, int h );
bool readPPMImage( const char *filename, MyTexture2D *texture );
bool readJPEGImage(const char* filename, MyTexture2D *texture );
bool readPNGImage(const char* file_name, MyTexture2D *texture );
bool writePNGImage(const char* file_name, MyTexture2D *texture );
void destroyImage( MyTexture2D *texture );
void setTexture( MyTexture2D *texture, int loc );
bool writePNGFromTexture(const char *file_name, int textureID, int format, int type, int width, int height, int Bpp, int stride );
bool writePNGFromDepthTexture(const char *file_name, int textureID, int width, int height);
void loadQuadVolumeTexPNG( const char *filename_stub, int nslices, MyTexture2D *texture );
void loadCubeTexPNG( const char *filename_stub, MyTexture2D *texture );
void loadQuadTexPNG( const char *filename, MyTexture2D *texture );

#endif /* _TEXTURE_HELPER_H_ */
