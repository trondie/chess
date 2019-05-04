#include "texture_helper.h"
#include "TestGLSL.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mymath.h"
#include "opengl_helpers.h"
#define STBI_HEADER_FILE_ONLY
#include <stb_image.c>
#undef STBI_HEADER_FILE_ONLY
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#undef STB_IMAGE_WRITE_IMPLEMENTATION

bool writePNGFromDepthTexture(const char *file_name, int textureID, int width, int height)
{
#ifdef PLATFORM_WINDOWS
	glPixelStoref(GL_UNPACK_ALIGNMENT, 1);
	unsigned char *data_ubyte = (unsigned char*) malloc(width*height);
	float *data_float = (float*) malloc(width*height*4);
	glBindTexture( GL_TEXTURE_2D, textureID );
	CHECK_GL_ERROR;
	glGetTexImage( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data_float );
	CHECK_GL_ERROR;
	float min = data_float[0],max = data_float[0];
	for(int i = 1; i<width*height; i++) {
		if ( data_float[i] < min ) min = data_float[i];
		if ( data_float[i] > max ) max = data_float[i];
	}
	for(int i = 0; i<width*height; i++) {
		data_ubyte[i] = (max-min) == 0 ? 0 : (data_float[i] - min) * (255.f - 0.f) / (max-min) + 0;
	}
	if (!stbi_write_png(file_name, width, height, 1, data_ubyte, width)) 
	{
		Base::log("[write_png_file] File %s could not be opened for writing", file_name);
		free(data_ubyte);
		return false;
	}
	free(data_float);
	free(data_ubyte);
#endif
	return true;
}

bool writePNGFromTexture(const char *file_name, int textureID, int format, int type, int width, int height, int Bpp, int stride )
{
#ifdef PLATFORM_WINDOWS
	char *data = (char*) malloc(stride*height);
	glBindTexture( GL_TEXTURE_2D, textureID );
	CHECK_GL_ERROR;
	glGetTexImage( GL_TEXTURE_2D, 0, format, type, data );
	CHECK_GL_ERROR;
	if (!stbi_write_png(file_name, width, height, Bpp, data, stride)) 
	{
		Base::log("[write_png_file] File %s could not be opened for writing", file_name);
		free(data);
		return false;
	}
	free(data);
#endif
	return true;
}

bool writePNGImage(const char* file_name, MyTexture2D *texture )
{
	if (!stbi_write_png(file_name, texture->width, texture->height, texture->Bpp, texture->data, texture->Bpp*texture->width)) 
	{
		Base::log("[write_png_file] File %s could not be opened for writing", file_name);
		return false;
	}
	return true;
}

bool readPNGImage(const char* file_name, MyTexture2D *texture )
{
	int width;
	int height;
	int Bpp;

	Base::log("readPNGImage %s", file_name );

	texture->data = stbi_load( file_name, &width, &height, &Bpp, 0 );

	if ( NULL == texture->data ) return false;

	texture->width = width;
	texture->height = height;
	texture->Bpp = Bpp;

	return true;
}

bool readJPEGImage(const char* file_name, MyTexture2D *texture )
{
	int width;
	int height;
	int Bpp;

	Base::log("readJPEGImage %s", file_name );

	texture->data = stbi_load( file_name,  &width, &height, &Bpp, 0 );	

	if ( NULL == texture->data ) return false;

	texture->width = width;
	texture->height = height;
	texture->Bpp = Bpp;
	return true;
}


void destroyImage( MyTexture2D *texture )
{
	free( texture->data );
	texture->id = 0;
	texture->width = 0;
	texture->height = 0;
	texture->Bpp = 0;
}

void writeppm_fp16( const char *filename, fp16 *fp16_buffer, int w, int h )
{
	//Assuming RGBA-fp16
	FILE	*fp = NULL;
	int		i = 0;
	int max = 0;
	char *buffer = (char*)fp16_buffer;
		
	fp = fopen( filename, "wb" );
	if ( NULL == fp ) 
	{
		Base::log("failed to open file %s", filename );
		return;
	}
	fprintf( fp, "P6\n%d %d\n255\n", w, h );
	for (i=0; i<w*h; i++)
	{
		fp16 rf = *(fp16*)(buffer + 0 + i*8);
		fp16 gf = *(fp16*)(buffer + 2 + i*8);
		fp16 bf = *(fp16*)(buffer + 4 + i*8);
		float r = _fp16_to_float(rf);
		float g = _fp16_to_float(gf);
		float b = _fp16_to_float(bf);
		if ( r > max ) max = r;
		if ( g > max ) max = g;
		if ( b > max ) max = b;
		unsigned char rc = r;
		unsigned char bc = b;
		unsigned char gc = g;
		
		fwrite( &rc, 1, 1, fp );
		fwrite( &gc, 1, 1, fp );
		fwrite( &bc, 1, 1, fp );
	}
	/* Always close the file as we've finished with it */
	fclose( fp );
	Base::log("wrote %s max: %d", filename, max);
	return;	
}

bool readPPMImage( const char *filename, MyTexture2D *texture )
{
#define RGB_COMPONENT_COLOR 255
	char buff[16];
	MyTexture2D img;
	FILE *fp;
	int c, rgb_comp_color;


	//open PPM file for reading
	fp = fopen(filename, "rb");
	if (!fp) 
	{
		Base::log("Unable to open file '%s'\n", filename);
		return false;
	}

	//read image format
	if (!fgets(buff, sizeof(buff), fp)) 
	{
		Base::log( "can't open file %s\n", filename);
		return false;
	}

	//check the image format
	if (buff[0] != 'P' || buff[1] != '6') 
	{
		Base::log( "Invalid image format (must be 'P6')\n");
		return false;
	}

	//check for comments
	c = getc(fp);
	while (c == '#') {
	while (getc(fp) != '\n') ;
		c = getc(fp);
	}

	ungetc(c, fp);
	//read image size information
	if (fscanf(fp, "%d %d", &texture->width, &texture->height) != 2) 
	{
		Base::log( "Invalid image size (error loading '%s')\n", filename) ;
		return false;
	}
	Base::log( "Read size %dx%d from '%s'\n", texture->width, texture->height, filename) ;

	//read rgb component
	if (fscanf(fp, "%d", &rgb_comp_color) != 1) 
	{
		Base::log("Invalid rgb component (error loading '%s')\n", filename);
		return false;
	}

	//check rgb component depth
	if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
		Base::log( "'%s' does not have 8-bits components\n", filename );
		return false;
	}

	while (fgetc(fp) != '\n') ;
	//memory allocation for pixel data
	texture->data = malloc(texture->width * texture->height * sizeof(unsigned char)*3 );

	if (!texture->data) 
	{
		Base::log( "Unable to allocate memory\n");
		return false;
	}

	//read pixel data from file
	if (fread(texture->data, 3 * texture->width, texture->height, fp) != texture->height ) 
	{
		Base::log( "Error loading image '%s'\n", filename );
		return false;
	}

	fclose(fp);

	texture->Bpp = sizeof(unsigned char)*3;

	return true;
#undef RGB_COMPONENT_COLOR
}

void setTexture( MyTexture2D *texture, int loc )
{
	glActiveTexture( GL_TEXTURE0 + loc );

	/*glEnable( texture->target );
	CHECK_GL_ERROR;*/
	glBindTexture( texture->target, texture->id );
	CHECK_GL_ERROR;
	glTexParameteri( texture->target, GL_TEXTURE_WRAP_S, texture->wrap_s );
	CHECK_GL_ERROR;
	glTexParameteri( texture->target, GL_TEXTURE_WRAP_T, texture->wrap_t );
	CHECK_GL_ERROR;
	if ( texture->wrap_u )
	{
#if PLATFORM_WINDOWS
		glTexParameteri( texture->target, GL_TEXTURE_WRAP_R, texture->wrap_u );
#else
		//glTexParameteri( texture->target, GL_TEXTURE_WRAP_R_OES, texture->wrap_u );
#endif

		CHECK_GL_ERROR;
	}
	glTexParameteri( texture->target, GL_TEXTURE_MIN_FILTER, texture->filter_min );
	CHECK_GL_ERROR;
	glTexParameteri( texture->target, GL_TEXTURE_MAG_FILTER, texture->filter_mag );
	CHECK_GL_ERROR;
}

void loadQuadTexPNG( const char *filename, MyTexture2D *texture )
{
	glGenTextures( 1, &texture->id );
	CHECK_GL_ERROR;
	glBindTexture( GL_TEXTURE_2D, texture->id );
	CHECK_GL_ERROR;

	readPNGImage( filename, texture );
	CHECK_GL_ERROR;

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	CHECK_GL_ERROR;

	if ( texture->Bpp == 4 )
	{
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,  texture->width, texture->height, GL_FALSE, GL_RGBA, GL_UNSIGNED_BYTE, texture->data );
		CHECK_GL_ERROR;
	}
	else if ( texture->Bpp == 3 )
	{
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,  texture->width, texture->height, GL_FALSE, GL_RGB, GL_UNSIGNED_BYTE, texture->data );
		CHECK_GL_ERROR;
	}
	else if ( texture->Bpp == 2 )
	{
		glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA,  texture->width, texture->height, GL_FALSE, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texture->data );
		CHECK_GL_ERROR;
	}
	else if ( texture->Bpp == 1 )
	{
		glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE,  texture->width, texture->height, GL_FALSE, GL_LUMINANCE, GL_UNSIGNED_BYTE, texture->data );
		CHECK_GL_ERROR;
	}
	else assert( 0 );

	texture->wrap_s = GL_CLAMP_TO_EDGE;
	texture->wrap_t = GL_CLAMP_TO_EDGE;
	texture->filter_min = GL_NEAREST;
	texture->filter_mag = GL_NEAREST;
	texture->target = GL_TEXTURE_2D;
}

void loadCubeTexPNG( const char *filename_stub, MyTexture2D *texture )
{
	const char *stubs[] = { "_BK.png", "_DN.png", "_FT.png", "_LT.png", "_RT.png", "_UP.png" };
	const GLint gl_face[] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y };

	glGenTextures( 1, &texture->id );
	CHECK_GL_ERROR;
	glBindTexture( GL_TEXTURE_CUBE_MAP, texture->id );
	CHECK_GL_ERROR;

	char *filename = (char *)malloc( strlen(filename_stub) + 8 );
	strcpy( filename, filename_stub );
	char *data = NULL;
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	for ( int i = 0; i < 6; i++ )
	{
		strcpy( &filename[strlen(filename_stub)], stubs[i] );
		readPNGImage( filename, texture );
		CHECK_GL_ERROR;
		switch( texture->Bpp )
		{
		case 4:
			glTexImage2D( gl_face[i], 0, GL_RGBA,  texture->width, texture->height, GL_FALSE, GL_RGBA, GL_UNSIGNED_BYTE, texture->data );
			break;
		case 3:
			glTexImage2D( gl_face[i], 0, GL_RGB,  texture->width, texture->height, GL_FALSE, GL_RGB, GL_UNSIGNED_BYTE, texture->data );
			break;
		case 2:
			glTexImage2D( gl_face[i], 0, GL_LUMINANCE_ALPHA,  texture->width, texture->height, GL_FALSE, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texture->data );
			break;
		case 1:
			glTexImage2D( gl_face[i], 0, GL_LUMINANCE,  texture->width, texture->height, GL_FALSE, GL_LUMINANCE, GL_UNSIGNED_BYTE, texture->data );
			break;
		default:
			assert( 0 );
		}
		free( texture->data );
		texture->data = NULL;
		CHECK_GL_ERROR;
	}

	texture->wrap_s = GL_CLAMP_TO_EDGE;
	texture->wrap_t = GL_CLAMP_TO_EDGE;
	texture->wrap_u = GL_CLAMP_TO_EDGE;
	texture->filter_min = GL_LINEAR;
	texture->filter_mag = GL_LINEAR;
	texture->target = GL_TEXTURE_CUBE_MAP;
}

/* will concatenate filename_stub with %d.png requiring filename_stub1.png ... filename_stubN.png */
void loadQuadVolumeTexPNG( const char *filename_stub, int nslices, MyTexture2D *texture )
{
#if PLATFORM_WINDOWS
	PFNGLTEXIMAGE3DPROC glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
#else
	PFNGLTEXIMAGE3DOESPROC glTexImage3D = (PFNGLTEXIMAGE3DOESPROC) eglGetProcAddress("glTexImage3DOES");
#endif
	char *all_data = NULL;
	assert(glTexImage3D);

	int stublen = strlen( filename_stub );
	char *filename = (char*)malloc( stublen + 8 );
	strncpy(filename, filename_stub, stublen);

	int offset = 0;
	for ( int i = 1; i <= nslices; i++ )
	{
		sprintf( &filename[stublen], "%d.png\0", i );
		readPNGImage( filename, texture );
		int texture_size = texture->width*texture->height*texture->Bpp;
		if (NULL == all_data)
		{
			all_data = (char*)malloc( texture_size*nslices );
		}
		memcpy(all_data + offset, texture->data, texture_size);
		offset += texture_size;
		free(texture->data);
	}
	texture->data = all_data;

	glGenTextures( 1, &texture->id );
#if PLATFORM_WINDOWS
	texture->target = GL_TEXTURE_3D;
#else
	texture->target = GL_TEXTURE_3D_OES;
#endif

	glBindTexture( texture->target, texture->id );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	if ( texture->Bpp == 4 )
	{
		glTexImage3D(texture->target, 0, GL_RGBA,  texture->width, texture->height, nslices, GL_FALSE, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
	}
	else if ( texture->Bpp == 3 )
	{
		glTexImage3D(texture->target, 0, GL_RGB,  texture->width, texture->height, nslices, GL_FALSE, GL_RGB, GL_UNSIGNED_BYTE, texture->data);
	}
	else if ( texture->Bpp == 2 )
	{
		glTexImage3D(texture->target, 0, GL_LUMINANCE_ALPHA, texture->width, texture->height, nslices, GL_FALSE, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texture->data);
	}
	else if ( texture->Bpp == 1 )
	{
		glTexImage3D(texture->target, 0, GL_LUMINANCE, texture->width, texture->height, nslices, GL_FALSE, GL_LUMINANCE, GL_UNSIGNED_BYTE, texture->data);
	}
	else assert( 0 );

	texture->wrap_s = GL_CLAMP_TO_EDGE;
	texture->wrap_t = GL_CLAMP_TO_EDGE;
	texture->wrap_u = GL_CLAMP_TO_EDGE;
	texture->filter_min = GL_NEAREST;
	texture->filter_mag = GL_NEAREST;
	setTexture(texture, 0);
	CHECK_GL_ERROR;
#if !PLATFORM_WINDOWS
	glGenerateMipmap(GL_TEXTURE_3D_OES);
	CHECK_GL_ERROR;
#endif
}
