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
	// ���������Ƭ����Ϣ
	std::vector<PhotoInfo> photos; // ����ӡ����Ƭ����
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
	std::sort(photos.rbegin(), photos.rend()); // ������ӡ��Ƭ�Ӵ�СԤ������һ�£������ӿռ�������

	// ��ӡ�ϲ����д���
	LARGE_INTEGER nFreq;
    LARGE_INTEGER nLast;
    LARGE_INTEGER nNow;
    QueryPerformanceFrequency(&nFreq);
    QueryPerformanceCounter(&nLast);
	//FILETIME timeBefore;
	//GetSystemTimeAsFileTime(&timeBefore);

	float maxWidth = 76.2; // ��ӡֽ�������
	float maxHeight = 254; // ��ӡֽ�����߶�
	std::vector<RectBinPacker> printPapers; // �����ֽ������
	std::vector<PrintInfo> printInfos; // ������Ƭ�Ĵ�ӡ��Ϣ

	printPapers.push_back(RectBinPacker(maxWidth, maxHeight)); // ��һ�Ŵ�ӡֽ
	int photoNo = -1; // ��ǰ��Ƭ���
	for (std::vector<PhotoInfo>::iterator photoIter = photos.begin(); photoIter != photos.end(); photoIter++) { // ��һ����Ƭ
		photoNo++;

		int pageNo = -1; // ��ǰ��ӡֽ���

		if (photoIter->width > maxWidth || photoIter->height > maxHeight) {
			std::cout << "photo is too large" << std::endl;
			return 1;
		}

		bool isInserted = false;
		for (std::vector<RectBinPacker>::iterator paperIter = printPapers.begin(); paperIter != printPapers.end(); paperIter++) { // ����ÿ�Ŵ�ӡֽ�Խ�ʡ�ռ�
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

		if (isInserted == false) { // ��ӡ�ռ䲻�㣬���һҳ�µĴ�ӡֽ
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

	// ��������Ϣ
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

	// ���ɴ�ӡͼ��
	FreeImage_Initialise(TRUE);
	int imageNo = -1;
	for (std::vector<RectBinPacker>::iterator paperIter = printPapers.begin(); paperIter != printPapers.end(); paperIter++) {
		imageNo++;
		RGBQUAD c; c.rgbBlue = 255; c.rgbGreen = 255; c.rgbRed = 255; c.rgbReserved = 0;
		FIBITMAP* imageBitmap = FreeImage_AllocateExT(FIT_BITMAP, maxWidth, maxHeight, 32, &c);

		for (std::vector<PrintInfo>::iterator printIter = printInfos.begin(); printIter != printInfos.end(); printIter++) {
			if (printIter->pageNo == imageNo) {
				//RGBQUAD c; 
				//c.rgbBlue = ((printIter->photoInfo.albumNo / (2 * 2)) % 2)* (255 / 2); // 2���ưٷ�λֵ�ú�ɫ������ʾ
				//c.rgbGreen = ((printIter->photoInfo.albumNo / 2) % 2) * (255 / 2); // 2����ʮ��λֵ����ɫ������ʾ
				//c.rgbRed = (printIter->photoInfo.albumNo % 2) % (255 / 2); // 2���Ƹ���λֵ����ɫ������ʾ
				//c.rgbReserved = 255;
				RGBQUAD c; 
				c.rgbBlue = random_in_range(0, 255);
				c.rgbGreen = random_in_range(0, 255);
				c.rgbRed = random_in_range(0, 255);
				c.rgbReserved = 255;

				FIBITMAP* photoBitmap = NULL;
				if (printIter->isRotated == true) {
					//FreeImage������ʱ����ת
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
