// g++ create_tiff.cpp -ltiff -lz -o create_tiff
#include <iostream>
#include <math.h>
#include <tiffio.h>

// https://research.cs.wisc.edu/graphics/Courses/638-f1999/libtiff_tutorial.htm
int main(int argc, const char* argv[])
{
	int rc = 0 ;
	if ( argc < 4 || argc > 5 ) {
		std::cerr << "usage " << argv[0] << " width height path [open-option]" << std::endl;
	} else {
		int   width         = atoi(argv[1]);
		int   height        = atoi(argv[2]);
        const char*    path = argv[3] ;
        const char*    open = argc == 5 ? argv[4] : "w8" ;
        TIFF*          out  = NULL    ;
		unsigned char* buf  = NULL    ;
		 	
		int sampleperpixel  = 4;    // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon.
        
        if ( width <= 0 || height <= 0 ) 		{
			std::cerr << "illegal width or height" << std::endl;
			rc = 3;
		} else {
        	out= TIFFOpen(path, open); // "w8" = write BigTiff
        	if ( !out ) {
				std::cerr << "path did not open" << std::endl;
				rc = 4;
        	}
        }
        
        
        if ( rc == 0 ) {
			tsize_t linebytes = sampleperpixel * width;                   // length in memory of one row of pixel in the image.
			// Allocating memory to store the pixels of current row
			buf 	= TIFFScanlineSize(out) < linebytes 
					? (unsigned char *)_TIFFmalloc(linebytes)
					: (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out))
					;
			if ( ! buf ) {
				std::cerr << "unable to allocate scan buffer" << std::endl;
				rc = 4;
			}
		}
		
		if ( rc == 0 ) {

			TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width);                // set the width of the image
			TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);               // set the height of the image
			TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
			TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);                  // set the size of the channels
			TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);  // set the origin of the image.
			//   Some other essential fields to set that you do not have to understand for now.
			TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
			
			// TIFFSetField(tif, TIFFTAG_COMPRESSION    , COMPRESSION_LZW       );


			// We set the strip size of the file to be size of one row of pixels
			TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, width*sampleperpixel));

			//Now writing image to the file one strip at a time
			for (uint32 row = 0; rc == 0 && row < height ; row++)
			{
				if (TIFFWriteScanline(out, buf, row, 0) < 0) rc = 2 ;
			}
		}
		
		// Finally we close the output file, and destroy the buffer
		if ( out )  TIFFClose(out);
		if ( buf ) _TIFFfree (buf);
    }
    return rc;
}
