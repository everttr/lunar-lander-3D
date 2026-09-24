#include <stdio.h>

#define VERBOSE		false

unsigned char *	BmpToTexture( char *, int *, int * );
int		ReadInt( FILE * );
short		ReadShort( FILE * );

#define BMP_MAGIC_NUMBER	0x4d42
#ifndef BI_RGB
#define BI_RGB			0
#define BI_RLE8			1
#define BI_RLE4			2
#endif


// bmp file header:
struct bmfh
{
	short bfType;		// BMP_MAGIC_NUMBER = "BM"
	int bfSize;		// size of this file in bytes
	short bfReserved1;
	short bfReserved2;
	int bfOffBytes;		// # bytes to get to the start of the per-pixel data
} FileHeader;

// bmp info header:
struct bmih
{
	int biSize;		// info header size, should be 40
	int biWidth;		// image width
	int biHeight;		// image height
	short biPlanes;		// #color planes, should be 1
	short biBitCount;	// #bits/pixel, should be 1, 4, 8, 16, 24, 32
	int biCompression;	// BI_RGB, BI_RLE4, BI_RLE8
	int biSizeImage;
	int biXPixelsPerMeter;
	int biYPixelsPerMeter;
	int biClrUsed;		// # colors in the palette
	int biClrImportant;
} InfoHeader;



unsigned char *
BmpToTexture( char *filename, int *width, int *height )
{
	FILE* fp;
#ifdef _WIN32
        errno_t err = fopen_s( &fp, filename, "rb" );
        if( err != 0 )
        {
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
        }
#else
	fp = fopen( filename, "rb" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
	}
#endif

	FileHeader.bfType = ReadShort( fp );


	// if bfType is not BMP_MAGIC_NUMBER, the file is not a bmp:

	if( VERBOSE ) fprintf( stderr, "FileHeader.bfType = 0x%0x = \"%c%c\"\n",
			FileHeader.bfType, FileHeader.bfType&0xff, (FileHeader.bfType>>8)&0xff );

	if( FileHeader.bfType != BMP_MAGIC_NUMBER )
	{
		fprintf( stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType );
		fclose( fp );
		return NULL;
	}


	FileHeader.bfSize = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "FileHeader.bfSize = %d\n", FileHeader.bfSize );

	FileHeader.bfReserved1 = ReadShort( fp );
	FileHeader.bfReserved2 = ReadShort( fp );

	FileHeader.bfOffBytes = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "FileHeader.bfOffBytes = %d\n", FileHeader.bfOffBytes );


	InfoHeader.biSize = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biSize = %d\n", InfoHeader.biSize );
	InfoHeader.biWidth = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biWidth = %d\n", InfoHeader.biWidth );
	InfoHeader.biHeight = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biHeight = %d\n", InfoHeader.biHeight );

	// the horizontal and vertical dimensions:
	int nums = InfoHeader.biWidth;
	int numt = InfoHeader.biHeight;

	// the horizontal dimension as the next lower multiple of 4:
	int nums4 = 4 * (nums / 4);

	InfoHeader.biPlanes = ReadShort( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biPlanes = %d\n", InfoHeader.biPlanes );

	InfoHeader.biBitCount = ReadShort( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biBitCount = %d\n", InfoHeader.biBitCount );

	fprintf(stderr, "Image file '%s' has pixel dimensions %dx%d and %d bits/pixel\n", filename, nums, numt, InfoHeader.biBitCount);

	InfoHeader.biCompression = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biCompression = %d\n", InfoHeader.biCompression );

	InfoHeader.biSizeImage = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biSizeImage = %d\n", InfoHeader.biSizeImage );

	InfoHeader.biXPixelsPerMeter = ReadInt( fp );
	InfoHeader.biYPixelsPerMeter = ReadInt( fp );

	InfoHeader.biClrUsed = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biClrUsed = %d\n", InfoHeader.biClrUsed );

	InfoHeader.biClrImportant = ReadInt( fp );

	// pixels will be stored bottom-to-top, left-to-right:
	int numcomps = InfoHeader.biBitCount / 8;
	if( VERBOSE )	fprintf( stderr, "numcomps = %d\n", numcomps );

	unsigned char *texture = new unsigned char[ 3 * nums * numt ];
	if( texture == NULL )
	{
		fprintf( stderr, "Cannot allocate the texture array!\n" );
		return NULL;
	}


	// extra padding bytes:

	//fprintf(stderr, "BitCount = %d ; Width = %d\n", InfoHeader.biBitCount, InfoHeader.biWidth);
	//int requiredRowSizeInBytes = 4 * ( ( InfoHeader.biBitCount*InfoHeader.biWidth + 31 ) / 32 );
	//float num = InfoHeader.biBitCount*InfoHeader.biWidth + 31.;
	//float denom =  32.;
	//int requiredRowSizeInWords = (int)(InfoHeader.biBitCount * InfoHeader.biWidth + 31) / 32;	// next highest integer
	//int requiredRowSizeInBytes = 4 * requiredRowSizeInWords;

	//requiredRowSizeInBytes = 4 * ifloor( num / denom );
	//fprintf( stderr, "requiredRowSizeInBytes using floor = %d\n", requiredRowSizeInBytes );

	//num = InfoHeader.biBitCount*InfoHeader.biWidth;
	//requiredRowSizeInBytes = 4 * iceil( num / denom );
	//fprintf( stderr, "requiredRowSizeInBytes using ceil = %d\n", requiredRowSizeInBytes );

	//if( VERBOSE )	fprintf( stderr, "requiredRowSizeInBytes = %d\n", requiredRowSizeInBytes );


	int rowSizeInBytes = numcomps * nums;
	int numExtra = 4 - ( rowSizeInBytes % 4 );
	if( numExtra == 4 )
		numExtra = 0;
	if( VERBOSE )	fprintf( stderr, "NumExtra padding = %d\n", numExtra );
	//fprintf( stderr, "rowSizeInBytes = %d\n", rowSizeInBytes );
	//fprintf(stderr, "NumExtra padding = %d\n", numExtra);


	// we can handle 24 bits of direct color:
	if( InfoHeader.biBitCount == 24 )
	{
		// 24-bit does not want to see the compression bits set:

		if (InfoHeader.biCompression != 0)
		{
			fprintf(stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression);
			fclose(fp);
			return NULL;
		}

		rewind(fp);
		fseek(fp, FileHeader.bfOffBytes, SEEK_SET);

		if( numcomps == 3  ||  numcomps == 4 )
		{
			unsigned char *tp = &texture[0];	// at the start
			for (int t = 0; t < numt; t++)
			{
				for( int s = 0; s < nums4; s++ )
				{
					if( numcomps == 4 )
						(void)fgetc( fp );					// a
					unsigned char b = fgetc( fp );			// b
					unsigned char g = fgetc( fp );			// g
					unsigned char r = fgetc( fp );			// r
					*tp++ = r;
					*tp++ = g;
					*tp++ = b;
				}
				for (int s = nums4; s < nums; s++)
				{
					if (numcomps == 4)
						(void)fgetc(fp);					// a
					unsigned char b = fgetc(fp);			// b
					unsigned char g = fgetc(fp);			// g
					unsigned char r = fgetc(fp);			// r
				}
				for( int e = 0; e < numExtra; e++ )
				{
					unsigned char uc = fgetc( fp );
					//if( uc != 0 )
						//fprintf( stderr, "%4d: %3d ", t, uc );
				}
			}
		}
	}

	// we can also handle 8 bits of indirect color:
	if (InfoHeader.biBitCount == 8 && InfoHeader.biClrUsed == 256)
	{
		// 8-bit does not want to see the compression bits set:

		if (InfoHeader.biCompression != 0)
		{
			fprintf(stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression);
			fclose(fp);
			return NULL;
		}
		struct rgba32
		{
			unsigned char r, g, b, a;
		};
		struct rgba32* colorTable = new struct rgba32[InfoHeader.biClrUsed];

		rewind(fp);
		fseek(fp, sizeof(struct bmfh) + InfoHeader.biSize - 2, SEEK_SET);
		for (int c = 0; c < InfoHeader.biClrUsed; c++)
		{
			colorTable[c].b = fgetc(fp);
			colorTable[c].g = fgetc(fp);
			colorTable[c].r = fgetc(fp);
			colorTable[c].a = fgetc(fp);
			if (VERBOSE)	fprintf(stderr, "%4d:\t0x%02x\t0x%02x\t0x%02x\t0x%02x\n",
				c, colorTable[c].r, colorTable[c].g, colorTable[c].b, colorTable[c].a);
		}

		rewind(fp);
		fseek(fp, FileHeader.bfOffBytes, SEEK_SET);

		if( numcomps == 1 )
		{
			unsigned char *tp = &texture[0];	// at the start
			for (int t = 0; t < numt; t++)
			{
				for (int s = 0; s < nums4; s++)
				{
					int index = fgetc(fp);
					unsigned char r = colorTable[index].r;	// r
					unsigned char g = colorTable[index].g;	// g
					unsigned char b = colorTable[index].b;	// b
					*tp++ = r;
					*tp++ = g;
					*tp++ = b;
				}
				for (int s = nums4; s < nums; s++)
				{
					int index = fgetc(fp);
				}
				for (int e = 0; e < numExtra; e++)
				{
					unsigned char uc = fgetc(fp);
				}
			}
		}
		delete[] colorTable;
	}

	// we can handle 32 bits of direct color:
	if (InfoHeader.biBitCount == 32)
	{
		// 32-bit doesn't mind if the compression bits are set -- we just ignore them:
		rewind( fp );
		fseek( fp, FileHeader.bfOffBytes, SEEK_SET );

		if( numcomps == 4 )
		{
			unsigned char *tp = &texture[0];	// at the start
			for (int t = 0; t < numt; t++)
			{
				for (int s = 0; s < nums4; s++)
				{
					unsigned char b = fgetc(fp);			// b
					unsigned char g = fgetc(fp);			// g
					unsigned char r = fgetc(fp);			// r
					unsigned char a = fgetc(fp);				// a
					*tp++ = r;
					*tp++ = g;
					*tp++ = b;
				}
				for (int s = nums4; s < nums; s++)
				{
					unsigned char b = fgetc(fp);			// b
					unsigned char g = fgetc(fp);			// g
					unsigned char r = fgetc(fp);			// r
					unsigned char a = fgetc(fp);			// a
				}
				for (int e = 0; e < numExtra; e++)
				{
					unsigned char a = fgetc(fp);
				}
			}
		}
	}

	*width  = nums4;
	*height = numt;

	fclose( fp );

	return texture;
}



int
ReadInt( FILE *fp )
{
	const unsigned char b0 = fgetc( fp );
	const unsigned char b1 = fgetc( fp );
	const unsigned char b2 = fgetc( fp );
	const unsigned char b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}


short
ReadShort( FILE *fp )
{
	const unsigned char b0 = fgetc( fp );
	const unsigned char b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}
