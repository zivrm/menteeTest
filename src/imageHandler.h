#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#pragma once
#include "jpeglib.h"
#include "jerror.h"
#include <bits/stdc++.h>
namespace jpegHandle{

 void saveAsJPEG(std::vector<uint8_t> pixelData14Bit,  const char* filename) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE* outfile;
    JSAMPROW row_pointer[1];
    int row_stride;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    if ((outfile = fopen(filename, "wb")) == NULL) {
        fprintf(stderr, "Can't open %s\n", filename);
        return;
    }
    std::vector<uint8_t> pixelData8Bit;
    uint64_t _7byteHolder;
    float ratio = 255.0/16383.0;
    uint32_t counter =0;
    while (pixelData8Bit.size()<1310720){
        
        _7byteHolder  = (pixelData14Bit[counter+0]      );  // this will hold 7 bytes or 56 bits which are 4 x 14bits pixels. this is done outside a loop to increase speed.
        _7byteHolder += (pixelData14Bit[counter+1] << 8 ); 
        _7byteHolder += (pixelData14Bit[counter+2] << 16); 
        _7byteHolder += (pixelData14Bit[counter+3] << 24); 
        // _7byteHolder += (pixelData14Bit[counter+4] << 32); 
        // _7byteHolder += (pixelData14Bit[counter+5] << 40); 
        // _7byteHolder += (pixelData14Bit[counter+6] << 48); 
        
        for (uint8_t i =0; i< 4; ++i){
            uint16_t holder14bit =(_7byteHolder >> 14*i)& 0x3fff;
            pixelData8Bit.push_back( (uint8_t)(holder14bit*ratio)); //now push the truncked 8bit grey level bit.
        }
        
        counter += 7;

    }
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = 1280;
    cinfo.image_height = 1024;
    cinfo.input_components = 1; // For grayscale images
    cinfo.in_color_space = JCS_GRAYSCALE; // Use JCS_GRAYSCALE for grayscale images

    jpeg_set_defaults(&cinfo);

    cinfo.data_precision = 8;
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = 1280; // Number of bytes in a row
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = const_cast<JSAMPROW>(&pixelData8Bit[cinfo.next_scanline * row_stride]);
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
}




}


#endif