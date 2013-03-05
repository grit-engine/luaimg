/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstdlib>
//#include <cmath>

#include <iostream>

#include <volpack.h>

#include "VoxelImage.h"
#include "Image.h"

unsigned short rgb_4_4_4 (unsigned char r, unsigned char g, unsigned char b)
{
    unsigned short c = 0;
    c |= (r >> 4) << 0;
    c |= (g >> 4) << 4;
    c |= (b >> 4) << 8;
    return c;
}

float clamp (float v, float min=0, float max=1)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

VoxelImage::VoxelImage (float *raw_data, chan_t channels, imglen_t width, imglen_t height, imglen_t depth, bool axes)
  : width(width), height(height), depth(depth)
{
    vpc = vpCreateContext();

    // DESCRIBE VOXEL DATA

    if (VP_OK != vpSetVolumeSize(vpc, width, height, depth)) {
        std::cerr << "VolPack error1: " << vpGetErrorString(vpGetError(vpc)) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (VP_OK != vpSetVoxelSize(vpc, sizeof(Voxel), 3, 2, 1)) {
        std::cerr << "VolPack error2: " << vpGetErrorString(vpGetError(vpc)) << std::endl;
        exit(EXIT_FAILURE);
    }

    Voxel *dummy_voxel;
    vpSetVoxelField(vpc, fieldRgb16,    2, vpFieldOffset(dummy_voxel, rgb16), VP_NORM_MAX);
    vpSetVoxelField(vpc, fieldScalar,   1, vpFieldOffset(dummy_voxel, scalar), 255);
    vpSetVoxelField(vpc, fieldGradient, 1, vpFieldOffset(dummy_voxel, gradient), VP_GRAD_MAX);

    unsigned char *scalar_data = new unsigned char[width*height*depth];
    voxelData = new Voxel[width*height*depth];

    switch (channels) {
        case 4:
        for (imglen_t z=0 ; z<depth ; ++z) {
            for (imglen_t y=0 ; y<height ; ++y) {
                for (imglen_t x=0 ; x<width ; ++x) {
                    unsigned char r = clamp(raw_data[channels*(z*height*width + y*width + x) + 0]) * 255;
                    unsigned char g = clamp(raw_data[channels*(z*height*width + y*width + x) + 1]) * 255;
                    unsigned char b = clamp(raw_data[channels*(z*height*width + y*width + x) + 2]) * 255;
                    voxelData[z*height*width + y*width + x].rgb16 = rgb_4_4_4(r,g,b);
                    scalar_data[z*height*width + y*width + x] = clamp(raw_data[channels*(z*height*width + y*width + x) + 3]) * 255;
                }
            }
        }
        break;

        case 3:
        for (imglen_t z=0 ; z<depth ; ++z) {
            for (imglen_t y=0 ; y<height ; ++y) {
                for (imglen_t x=0 ; x<width ; ++x) {
                    unsigned char r = clamp(raw_data[channels*(z*height*width + y*width + x) + 0]) * 255;
                    unsigned char g = clamp(raw_data[channels*(z*height*width + y*width + x) + 1]) * 255;
                    unsigned char b = clamp(raw_data[channels*(z*height*width + y*width + x) + 2]) * 255;
                    unsigned short c = rgb_4_4_4(r,g,b);
                    voxelData[z*height*width + y*width + x].rgb16 = c;
                    scalar_data[z*height*width + y*width + x] = c == 0 ? 0 : 255;
                }
            }
        }
        break;

        case 2:
        for (imglen_t z=0 ; z<depth ; ++z) {
            for (imglen_t y=0 ; y<height ; ++y) {
                for (imglen_t x=0 ; x<width ; ++x) {
                    unsigned char r = clamp(raw_data[channels*(z*height*width + y*width + x) + 0]) * 255;
                    unsigned char g = clamp(raw_data[channels*(z*height*width + y*width + x) + 1]) * 255;
                    unsigned char b = 0;
                    // convert 8:8:8 to 4:4:4
                    unsigned short c = rgb_4_4_4(r,g,b);
                    voxelData[z*height*width + y*width + x].rgb16 = c;
                    scalar_data[z*height*width + y*width + x] = c == 0 ? 0 : 255;
                }
            }
        }
        break;

        case 1:
        for (imglen_t z=0 ; z<depth ; ++z) {
            for (imglen_t y=0 ; y<height ; ++y) {
                for (imglen_t x=0 ; x<width ; ++x) {
                    unsigned char r = 255;
                    unsigned char g = 255;
                    unsigned char b = 255;
                    // convert 8:8:8 to 4:4:4
                    voxelData[z*height*width + y*width + x].rgb16 = rgb_4_4_4(r,g,b);
                    scalar_data[z*height*width + y*width + x] = clamp(raw_data[channels*(z*height*width + y*width + x) + 0]) * 255;
                }
            }
        }
        break;
    }

    if (axes) {
        for (unsigned z=0 ; z<depth ; ++z) {
            for (unsigned x=0 ; x<width ; x+=width-1) {
                for (unsigned y=0 ; y<height ; y+=height-1) {
                    unsigned r = float(x) / width * 255;
                    unsigned g = float(y) / width * 255;
                    unsigned b = float(z) / width * 255;
                    voxelData[z*height*width + y*width + x].rgb16 = rgb_4_4_4(r,g,b);
                    scalar_data[z*height*width + y*width + x] = 255;
                }
            }
        }
        for (unsigned x=0 ; x<width ; ++x) {
            for (unsigned z=0 ; z<depth ; z+=depth-1) {
                for (unsigned y=0 ; y<height ; y+=height-1) {
                    unsigned r = float(x) / width * 255;
                    unsigned g = float(y) / width * 255;
                    unsigned b = float(z) / width * 255;
                    voxelData[z*height*width + y*width + x].rgb16 = rgb_4_4_4(r,g,b);
                    scalar_data[z*height*width + y*width + x] = 255;
                }
            }
        }
        for (unsigned y=0 ; y<height ; ++y) {
            for (unsigned x=0 ; x<width ; x+=width-1) {
                for (unsigned z=0 ; z<depth ; z+=depth-1) {
                    unsigned r = float(x) / width * 255;
                    unsigned g = float(y) / width * 255;
                    unsigned b = float(z) / width * 255;
                    voxelData[z*height*width + y*width + x].rgb16 = rgb_4_4_4(r,g,b);
                    scalar_data[z*height*width + y*width + x] = 255;
                }
            }
        }
        unsigned the_max = std::max(std::max(width,height),depth);
        for (float i=0 ; i<the_max ; ++i) {
            unsigned char x = i/the_max * width;
            unsigned char y = i/the_max * height;
            unsigned char z = i/the_max * depth;
            voxelData[z*height*width + y*width + x].rgb16 = rgb_4_4_4(255,255,255);
            scalar_data[z*height*width + y*width + x] = 255;
        }
    }

    // convert 4:4:4 to float3
    for (unsigned c=0 ; c<=VP_NORM_MAX ; ++c) {
        colourTable[3*c + 0] = ((c>> 0) & ((1<<4)-1)) << 4;
        colourTable[3*c + 1] = ((c>> 4) & ((1<<4)-1)) << 4;
        colourTable[3*c + 2] = ((c>> 8) & ((1<<4)-1)) << 4;
    }   

    if (VP_OK != vpSetRawVoxels(vpc, voxelData, width*height*depth * sizeof(Voxel), sizeof(Voxel), width*sizeof(Voxel), width*height*sizeof(Voxel))) {
        std::cerr << "VolPack error3: " << vpGetErrorString(vpGetError(vpc)) << std::endl;
        exit(EXIT_FAILURE);
    }   
    
    if (VP_OK != vpVolumeNormals(vpc, scalar_data, width*height*depth, fieldScalar, fieldGradient, VP_SKIP_FIELD)) {
        std::cerr << "VolPack error4: " << vpGetErrorString(vpGetError(vpc)) << std::endl; 
        exit(EXIT_FAILURE);
    }   

    delete [] scalar_data;

    /* set the classification function */

    int DensityRampX[]   = {0, 255};
    float DensityRampY[] = {0.0f, 1.0f};
    vpRamp(densityRamp, sizeof(float), 2, DensityRampX, DensityRampY);
    if (VP_OK != vpSetClassifierTable(vpc, 0, fieldScalar, densityRamp, sizeof(densityRamp))) {
        std::cerr << "VolPack error5: " << vpGetErrorString(vpGetError(vpc)) << std::endl;
        exit(EXIT_FAILURE);
    }   
        
    // SHADING
    if (VP_OK != vpSetLookupShader(vpc, 3, 1, fieldRgb16, colourTable, sizeof(colourTable), 0, NULL, 0)) {
        std::cerr << "VolPack error6: " << vpGetErrorString(vpGetError(vpc)) << std::endl;
        exit(EXIT_FAILURE);
    }   

}

void VoxelImage::render (Image<3> *image, float euler_x, float euler_y, float euler_z)
{
    // FOG
    //vpSetDepthCueing(vpc, 1.0, 1.1);
    //vpEnable(vpc, VP_DEPTH_CUE, 1);

    // CAMERA
    vpSeti(vpc, VP_CONCAT_MODE, VP_CONCAT_LEFT);

    vpCurrentMatrix(vpc, VP_PROJECT);
    vpIdentityMatrix(vpc);

    vpWindow(vpc, VP_PARALLEL, -0.85, 0.85, -0.85, 0.85, -0.85, 0.85);

    vpCurrentMatrix(vpc, VP_MODEL);
    vpIdentityMatrix(vpc);
    vpRotate(vpc, VP_Z_AXIS, euler_z);
    vpRotate(vpc, VP_Y_AXIS, euler_y);
    vpRotate(vpc, VP_X_AXIS, euler_x);

    // set the image buffer 
    unsigned char *raw_image = new unsigned char[image->width * image->height * 3];
    vpSetImage(vpc, raw_image, image->width, image->height, image->width*3, VP_RGB);

    vpSetd(vpc, VP_MIN_VOXEL_OPACITY, 0.05);
    vpSetd(vpc, VP_MAX_RAY_OPACITY, 0.95);

    if (vpRenderRawVolume(vpc) != VP_OK) {
        std::cerr << "VolPack error7: " << vpGetErrorString(vpGetError(vpc)) << std::endl;
        exit(EXIT_FAILURE);
    }

    for (imglen_t y=0 ; y<image->height ; ++y) {
        for (imglen_t x=0 ; x<image->width ; ++x) {
            image->pixel(x,y)[0] = raw_image[3*(y*image->width + x) + 0] / 255.0f;
            image->pixel(x,y)[1] = raw_image[3*(y*image->width + x) + 1] / 255.0f;
            image->pixel(x,y)[2] = raw_image[3*(y*image->width + x) + 2] / 255.0f;
        }
    }

    delete [] raw_image;
}

VoxelImage::~VoxelImage (void)
{
    delete [] voxelData;
    vpDestroyContext(vpc);
}

#if 0
#include <cmath>

#include <iostream>
#include <sstream>
#include <iomanip>

float normally_distributed_float (void)
{
    float x = 0;
    for (int i=0 ; i<5 ; ++i) x += random()/(float(RAND_MAX)+1);
    return x / 5;
}

int main (int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <num_frames>" << std::endl;
        exit(EXIT_FAILURE);
    }

    int num_frames = strtol(argv[1], NULL, 10);

    std::cout << "Generating data..." << std::endl;
    float *raw_data = new float[256*256*256*4];
    //for (unsigned z=0 ; z<256 ; ++z) {
    //    for (unsigned y=0 ; y<256 ; ++y) {
    //        for (unsigned x=0 ; x<256 ; ++x) {
    //            float X=(x-128.0f)/100, Y=(y-128.0f)/100, Z=(z-128.0f)/100;
    //            unsigned char v = (X*X + Y*Y + Z*Z) > 1 ? 0 : 4;
    //            raw_data[256*256*z + 256*y + x] = v;
    //            data[256*256*z + 256*y + x].rgb16 = rgb16(x,y,z);
    //        }
    //    }
    //}
    for (unsigned i=0 ; i<100000 ; ++i) {
        float X = normally_distributed_float();
        float Y = normally_distributed_float();
        float Z = normally_distributed_float();
        unsigned char x = X * 256;
        unsigned char y = Y * 256;
        unsigned char z = Z * 256;
        //X = X*2 - 1;
        //Y = Y*2 - 1;
        //Z = Z*2 - 1;
        //float rad = sqrtf(X*X+Y*Y+Z*Z);
        //rad = rad > 1 ? 1 : rad;
        float H=0, S=0, L=0;
        RGBtoHSL(X,Y,Z, H,S,L);
        float R=0, G=0, B=0;
        HSLtoRGB(H,S,L, R,G,B);

        raw_data[4*(256*256*z + 256*y + x) + 0] = R;
        raw_data[4*(256*256*z + 256*y + x) + 1] = G;
        raw_data[3*(256*256*z + 256*y + x) + 2] = B;
        raw_data[4*(256*256*z + 256*y + x) + 3] = 1.0f;
    }
    for (unsigned i=0 ; i<256 ; ++i) {
        for (unsigned x=0 ; x<256 ; x+=255) {
            for (unsigned y=0 ; y<256 ; y+=255) {
                unsigned char z = i;
                raw_data[4*(256*256*z + 256*y + x) + 0] = x/255.0f;
                raw_data[4*(256*256*z + 256*y + x) + 1] = y/255.0f;
                raw_data[4*(256*256*z + 256*y + x) + 2] = z/255.0f;
                raw_data[4*(256*256*z + 256*y + x) + 3] = 1.0f;
            }
        }
    }
    for (unsigned i=0 ; i<256 ; ++i) {
        for (unsigned z=0 ; z<256 ; z+=255) {
            for (unsigned y=0 ; y<256 ; y+=255) {
                unsigned char x = i;
                raw_data[4*(256*256*z + 256*y + x) + 0] = x/255.0f;
                raw_data[4*(256*256*z + 256*y + x) + 1] = y/255.0f;
                raw_data[4*(256*256*z + 256*y + x) + 2] = z/255.0f;
                raw_data[4*(256*256*z + 256*y + x) + 3] = 1.0f;
            }
        }
    }
    for (unsigned i=0 ; i<256 ; ++i) {
        for (unsigned x=0 ; x<256 ; x+=255) {
            for (unsigned z=0 ; z<256 ; z+=255) {
                unsigned char y = i;
                raw_data[4*(256*256*z + 256*y + x) + 0] = x/255.0f;
                raw_data[4*(256*256*z + 256*y + x) + 1] = y/255.0f;
                raw_data[4*(256*256*z + 256*y + x) + 2] = z/255.0f;
                raw_data[4*(256*256*z + 256*y + x) + 3] = 1.0f;
            }
        }
    }

    std::cout << "Creating voxel image..." << std::endl;
    VoxelImage *voxel_image = new VoxelImage(raw_data, 4, 256, 256, 256);

    Image<3> *image = new Image<3>(512, 512);

    // render and store the images 
    for (int n=0 ; n<num_frames ; n++) {

        std::cout << "Rendering frame: " << n << std::endl;
        voxel_image->render(image, 15, -n*360.0/num_frames, 0);

        // store image
        std::stringstream ss;
        ss << "output_" << std::setfill('0') << std::setw(4) << n << ".ppm";
        if (!image_save(image, ss.str())) {
            std::cerr << "Could not save image." << std::endl;
            break;
        }
    }

    delete [] raw_data;

    return EXIT_SUCCESS;
}
#endif

