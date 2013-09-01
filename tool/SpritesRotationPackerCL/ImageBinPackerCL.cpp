#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include "FreeImage.h"
#include "RectBinPacker.h"
#include "CVS.h"

struct PhotoInfo
{
	int albumNo;
	float width;
	float height;

	// std::sort(rects.rbegin(), rects.rend()); // Sort from greatest to least area
    bool operator< (const PhotoInfo& rhs) {
		return this->width * this->height < rhs.width * rhs.height;
    }
};

struct PrintInfo
{
	int pageNo;
	float printLocationX;
	float printLocationY;
	bool isRotated;
	PhotoInfo photoInfo;
};

int random_in_range (unsigned int min, unsigned int max)
{
  int base_random = rand(); /* in [0, RAND_MAX] */
  if (RAND_MAX == base_random) return random_in_range(min, max);
  /* now guaranteed to be in [0, RAND_MAX) */
  int range       = max - min,
      remainder   = RAND_MAX % range,
      bucket      = RAND_MAX / range;
  /* There are range buckets, plus one smaller interval
     within remainder of RAND_MAX */
  if (base_random < RAND_MAX - remainder) {
    return min + base_random/bucket;
  } else {
    return random_in_range (min, max);
  }
}

int _tmain(int argc, _TCHAR* argv[])
{
	// 获得所有照片的信息
	std::vector<PhotoInfo> photos; // 被打印的照片数组
	std::ifstream csv("photo.csv");
	for(CVSIterator loop(csv); loop != CVSIterator(); ++loop)
    {
		PhotoInfo photo;
		photo.albumNo = atoi((*loop)[0].c_str());
		photo.width = atof((*loop)[1].c_str());
		photo.height = atof((*loop)[2].c_str());

		//std::cout << photo.albumNo << ":" << photo.width << "," << photo.height << std::endl;
		photos.push_back(photo);
    }
	std::sort(photos.rbegin(), photos.rend()); // 将被打印照片从大到小预先排列一下，能增加空间利用率

	// 打印合并排列处理
	LARGE_INTEGER nFreq;
    LARGE_INTEGER nLast;
    LARGE_INTEGER nNow;
    QueryPerformanceFrequency(&nFreq);
    QueryPerformanceCounter(&nLast);
	//FILETIME timeBefore;
	//GetSystemTimeAsFileTime(&timeBefore);

	float maxWidth = 76.2; // 打印纸张最大宽度
	float maxHeight = 254; // 打印纸张最大高度
	std::vector<RectBinPacker> printPapers; // 所需的纸张数组
	std::vector<PrintInfo> printInfos; // 所有照片的打印信息

	printPapers.push_back(RectBinPacker(maxWidth, maxHeight)); // 第一张打印纸
	int photoNo = -1; // 当前照片序号
	for (std::vector<PhotoInfo>::iterator photoIter = photos.begin(); photoIter != photos.end(); photoIter++) { // 下一张照片
		photoNo++;

		int pageNo = -1; // 当前打印纸序号

		if (photoIter->width > maxWidth || photoIter->height > maxHeight) {
			std::cout << "photo is too large" << std::endl;
			return 1;
		}

		bool isInserted = false;
		for (std::vector<RectBinPacker>::iterator paperIter = printPapers.begin(); paperIter != printPapers.end(); paperIter++) { // 迭代每张打印纸以节省空间
			pageNo++;

			if (paperIter->tryInsert(photoIter->width, photoIter->height, true)) {
				RectBinPacker::Node node;
				paperIter->insert(photoIter->width, photoIter->height, photoNo, true, node);

				PrintInfo printInfo;
				printInfo.pageNo = pageNo;
				printInfo.printLocationX = node.rect.x;
				printInfo.printLocationY = node.rect.y;
				printInfo.isRotated = node.rotated;
				printInfo.photoInfo = *photoIter;
				printInfos.push_back(printInfo);

				isInserted = true;
				break;
			}
		}

		if (isInserted == false) { // 打印空间不足，添加一页新的打印纸
			printPapers.push_back(RectBinPacker(maxWidth, maxHeight));
			std::vector<RectBinPacker>::iterator paperIter = printPapers.end();
			pageNo++;

			RectBinPacker::Node node;
			(--paperIter)->insert(photoIter->width, photoIter->height, photoNo, true, node);

			PrintInfo printInfo;
			printInfo.pageNo = pageNo;
			printInfo.printLocationX = node.rect.x;
			printInfo.printLocationY = node.rect.y;
			printInfo.isRotated = node.rotated;
			printInfo.photoInfo = *photoIter;
			printInfos.push_back(printInfo);
		}
	}

	QueryPerformanceCounter(&nNow);
	//FILETIME timeAfter;
	//GetSystemTimeAsFileTime(&timeAfter);

	// 输出结果信息
	float sumArea = 0;
	for (std::vector<PrintInfo>::iterator printIter = printInfos.begin(); printIter != printInfos.end(); printIter++) {
		//std::cout << printIter->pageNo << ":" << printIter->printLocationX << "," << printIter->printLocationY << ";" << printIter->photoInfo.width << "," << printIter->photoInfo.height << std::endl;
		sumArea += printIter->photoInfo.width * printIter->photoInfo.height;
	}
	std::cout << "use time " << (nNow.QuadPart - nLast.QuadPart) / (double)nFreq.QuadPart * 1000.0 << " milliseconds" << std::endl;
	//ULARGE_INTEGER time_before; time_before.LowPart = timeBefore.dwLowDateTime; time_before.HighPart = timeBefore.dwHighDateTime;
	//ULARGE_INTEGER time_after; time_after.LowPart = timeAfter.dwLowDateTime; time_after.HighPart = timeAfter.dwHighDateTime;
	//std::cout << "use time " << (time_after.QuadPart - time_before.QuadPart) / 10000.0 << " milliseconds" << std::endl;
	std::cout << "paper count " << printPapers.size() << std::endl;
	std::cout << "utilization " << sumArea / ((maxWidth * maxHeight) * printPapers.size()) * 100 << "%" << std::endl;

	// 生成打印图像
	FreeImage_Initialise(TRUE);
	int imageNo = -1;
	for (std::vector<RectBinPacker>::iterator paperIter = printPapers.begin(); paperIter != printPapers.end(); paperIter++) {
		imageNo++;
		RGBQUAD c; c.rgbBlue = 255; c.rgbGreen = 255; c.rgbRed = 255; c.rgbReserved = 0;
		FIBITMAP* imageBitmap = FreeImage_AllocateExT(FIT_BITMAP, maxWidth, maxHeight, 32, &c);

		for (std::vector<PrintInfo>::iterator printIter = printInfos.begin(); printIter != printInfos.end(); printIter++) {
			if (printIter->pageNo == imageNo) {
				//RGBQUAD c; 
				//c.rgbBlue = ((printIter->photoInfo.albumNo / (2 * 2)) % 2)* (255 / 2); // 2进制百分位值用红色分量表示
				//c.rgbGreen = ((printIter->photoInfo.albumNo / 2) % 2) * (255 / 2); // 2进制十分位值用绿色分量表示
				//c.rgbRed = (printIter->photoInfo.albumNo % 2) % (255 / 2); // 2进制个分位值用蓝色分量表示
				//c.rgbReserved = 255;
				RGBQUAD c; 
				c.rgbBlue = random_in_range(0, 255);
				c.rgbGreen = random_in_range(0, 255);
				c.rgbRed = random_in_range(0, 255);
				c.rgbReserved = 255;

				FIBITMAP* photoBitmap = NULL;
				if (printIter->isRotated == true) {
					//FreeImage采用逆时针旋转
					photoBitmap = FreeImage_AllocateExT(FIT_BITMAP, printIter->photoInfo.height, printIter->photoInfo.width, 32, &c);
				}
				else {
					photoBitmap = FreeImage_AllocateExT(FIT_BITMAP, printIter->photoInfo.width, printIter->photoInfo.height, 32, &c);
				}

				FreeImage_Paste(imageBitmap, photoBitmap, printIter->printLocationX, printIter->printLocationY, 255);
				FreeImage_Unload(photoBitmap);
			}
		}

		std::stringstream imagePathStream;
		imagePathStream << "image" << imageNo << ".png";
		std::string imagePath = imagePathStream.str();
		FreeImage_Save(FIF_PNG, imageBitmap, imagePath.c_str(), PNG_DEFAULT);
		FreeImage_Unload(imageBitmap);
	}
	FreeImage_DeInitialise();

	system("pause");
	return 0;
}
