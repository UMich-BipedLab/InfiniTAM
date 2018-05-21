// Copyright 2018 Umich-BipedLab
// Author: Lu Gan, ganlu@umich.edu

#include "ZEDEngine.h"

#include "../ORUtils/FileUtils.h"

#include <cstdio>
#include <stdexcept>


#ifdef COMPILE_WITH_ZED
#include <sl/Camera.hpp>

using namespace InputSource;
using namespace ITMLib;


class ZEDEngine::PrivateData
{
	public:
	PrivateData(void): zed(new sl::Camera) {}
	sl::Camera* zed = nullptr;
	sl::Resolution res;
	sl::RuntimeParameters runtime_parameters;
	sl::Mat* image_zed = nullptr;
	sl::Mat* depth_image_zed = nullptr;
};

ZEDEngine::ZEDEngine(const char *calibFilename) :
	BaseImageSourceEngine(calibFilename) {
	
	// Set configuration parameters for the ZED
	sl::InitParameters initParameters;
	initParameters.camera_resolution = sl::RESOLUTION_HD720;  // RESOLUTION_HD1080
	//initParameters.camera_fps = 20;  // 30-60
	initParameters.depth_mode = sl::DEPTH_MODE_MEDIUM;  // ULTRA, QUALITY, MEDIUM, PERFORMANCE
	initParameters.coordinate_system = sl::COORDINATE_SYSTEM_RIGHT_HANDED_Y_UP; // OpenGL's coordinate system is right_handed
	initParameters.coordinate_units = sl::UNIT_METER;  // UNIT_METER
	//initParameters.depth_minimum_distance = 300;  // default:0.7-20m

	data = new PrivateData;

	// Open the camera
	sl::ERROR_CODE err = data->zed->open(initParameters);
	if (err != sl::SUCCESS) {
		std::cout << toString(err) << std::endl;
		data->zed->close();
		std::cout << "Failed to open ZED device." << std::endl;
		dataAvailable = false;
                delete data; data = nullptr;
                return; // Quit if an error occurred
	} else {
		data->res = data->zed->getResolution();
		data->runtime_parameters.sensing_mode = sl::SENSING_MODE_FILL;  // SENSING_MODE_FILL
		data->image_zed = new sl::Mat(data->res.width, data->res.height, sl::MAT_TYPE_8U_C4);
		data->depth_image_zed = new sl::Mat(data->res.width, data->res.height, sl::MAT_TYPE_8U_C4);
		this->imageSize_rgb = Vector2i(data->res.width, data->res.height);
		this->imageSize_d = Vector2i(data->res.width, data->res.height);
		std::cout << "ZED camera opened with resolution (rgb, depth): " << imageSize_rgb << " " << imageSize_d <<std::endl;
	}
}

ZEDEngine::~ZEDEngine() {
	if (data != nullptr) {
		data->zed->close();
		delete data; data = nullptr;
	}
}

void ZEDEngine::getImages(ITMUChar4Image *rgbImage, ITMShortImage *rawDepthImage) {
	dataAvailable = false;
	
	// setup infinitam frames
	Vector4u *rgb = rgbImage->GetData(MEMORYDEVICE_CPU);
	short *depth = rawDepthImage->GetData(MEMORYDEVICE_CPU);
	rgbImage->Clear();
	rawDepthImage->Clear();
	
	// get frames
	if (data->zed->grab(data->runtime_parameters) == sl::SUCCESS) {
		// Retrieve the left image, depth image
		data->zed->retrieveImage(*(data->image_zed), sl::VIEW_LEFT);
		data->zed->retrieveImage(*(data->depth_image_zed), sl::VIEW_DEPTH);
		// copy the rgb and depth value
		int i = 0;
		for (size_t r = 0; r < data->res.height; r++) {
			for (size_t c = 0; c < data->res.width; c++) {
				sl::uchar4 value;
				data->image_zed->getValue(c, r, &value);
				Vector4u newPix;
				newPix.x = value[2]; newPix.y = value[1]; newPix.z = value[0]; newPix.w = value[3];  // bgr2rgb
				data->depth_image_zed->getValue(c, r, &value);
				rgb[i] = newPix; depth[i++] = int(value[0]);  // uchar2short
			}
		}
		dataAvailable = true;
	} else std::cout << "Failed to read data from stream." << std::endl;
}

bool ZEDEngine::hasMoreImages(void) const { return (data!=nullptr); }
Vector2i ZEDEngine::getDepthImageSize(void) const { return (data!=nullptr)?imageSize_d:Vector2i(0,0); }
Vector2i ZEDEngine::getRGBImageSize(void) const { return (data!=nullptr)?imageSize_rgb:Vector2i(0,0); }

#else

using namespace InputSource;

ZEDEngine::ZEDEngine(const char *calibFilename)
: BaseImageSourceEngine(calibFilename)
{
	printf("compiled without ZED device support\n");
}
ZEDEngine::~ZEDEngine()
{}
void ZEDEngine::getImages(ITMUChar4Image *rgbImage, ITMShortImage *rawDepthImage)
{ return; }
bool ZEDEngine::hasMoreImages(void) const
{ return false; }
Vector2i ZEDEngine::getDepthImageSize(void) const
{ return Vector2i(0,0); }
Vector2i ZEDEngine::getRGBImageSize(void) const
{ return Vector2i(0,0); }

#endif
