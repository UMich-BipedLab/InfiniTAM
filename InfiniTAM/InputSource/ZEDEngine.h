// Copyright 2018 UMich-BipedLab
// Author: Lu Gan, ganlu@umich.edu

#pragma once

#include "ImageSourceEngine.h"

namespace InputSource
{
	class ZEDEngine : public BaseImageSourceEngine
	{
	private:
		class PrivateData;
		PrivateData *data;
		Vector2i imageSize_rgb, imageSize_d;
		bool dataAvailable;

	public:
		ZEDEngine(const char *calibFilename);
		~ZEDEngine();

		bool hasMoreImages(void) const;
		void getImages(ITMUChar4Image *rgb, ITMShortImage *rawDepth);
		Vector2i getDepthImageSize(void) const;
		Vector2i getRGBImageSize(void) const;
	};
}

