/*
 * sprites merge command line tool
 * author: xu xiao cheng <xiaocheng.xu@snsplus.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "FreeImage.h"
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "BinPacker.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace rapidxml;

// http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
string itoa(int value, int base) {
    string buf;

    // check that the base if valid
    if (base < 2 || base > 16) return buf;

    enum { kMaxDigits = 35 };
    buf.reserve( kMaxDigits ); // Pre-allocate enough space.

    int quotient = value;

    // Translating number to string with base:
    do {
        buf += "0123456789abcdef"[ abs( quotient % base ) ];
        quotient /= base;
    } while ( quotient );

    // Append the negative sign
    if ( value < 0) buf += '-';

    std::reverse( buf.begin(), buf.end() );
    return buf;
}

/**
 * Double to ASCII
 */

static double PRECISION = 0.00000000000001;
static int MAX_NUMBER_STRING_SIZE = 32;

char * dtoa(char *s, double n) {
    // handle special cases
    if (isnan(n)) {
        strcpy(s, "nan");
    } else if (isinf(n)) {
        strcpy(s, "inf");
    } else if (n == 0.0) {
        strcpy(s, "0");
    } else {
        int digit, m, m1;
        char *c = s;
        int neg = (n < 0);
        if (neg)
            n = -n;
        // calculate magnitude
        m = log10(n);
        int useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
        if (neg)
            *(c++) = '-';
        // set up for scientific notation
        if (useExp) {
            if (m < 0)
               m -= 1.0;
            n = n / pow(10.0, m);
            m1 = m;
            m = 0;
        }
        if (m < 1.0) {
            m = 0;
        }
        // convert the number
        while (n > PRECISION || m >= 0) {
            double weight = pow(10.0, m);
            if (weight > 0 && !isinf(weight)) {
                digit = floor(n / weight);
                n -= (digit * weight);
                *(c++) = '0' + digit;
            }
            if (m == 0 && n > 0)
                *(c++) = '.';
            m--;
        }
        if (useExp) {
            // convert the exponent
            int i, j;
            *(c++) = 'e';
            if (m1 > 0) {
                *(c++) = '+';
            } else {
                *(c++) = '-';
                m1 = -m1;
            }
            m = 0;
            while (m1 > 0) {
                *(c++) = '0' + m1 % 10;
                m1 /= 10;
                m++;
            }
            c -= m;
            for (i = 0, j = m-1; i<j; i++, j--) {
                // swap without temporary
                c[i] ^= c[j];
                c[j] ^= c[i];
                c[i] ^= c[j];
            }
            c += m;
        }
        *(c) = '\0';
    }
    return s;
}

// try insert sprite to result image
int spritePacker (FIBITMAP* srcBitmap, int* sprX, int* sprY, int* sprWidth, int* sprHeight, 
                  FIBITMAP* dstBitmap, BinPacker& binPacker, 
                  int padding)
{
    BinPacker::Node node;
    if (binPacker.insert(*sprWidth + padding * 2, *sprHeight + padding * 2, 0, node)) {
        FIBITMAP* sprBitmap = FreeImage_Copy(srcBitmap, *sprX, *sprY, *sprX + *sprWidth, *sprY + *sprHeight);
        FreeImage_Paste(dstBitmap, sprBitmap, node.rect.x + padding, node.rect.y + padding, 256);
        FreeImage_Unload(sprBitmap);
        *sprX = node.rect.x + padding;
        *sprY = node.rect.y + padding;
        return 1;
    }
    else {
        return 0;
    }
}

// loop spr xml element
int spriteMerge (xml_document<>* doc, FIBITMAP* srcBitmap, xml_node<>* dirOrSprNode, 
                 FIBITMAP* dstBitmap, BinPacker& binPacker, 
                 int padding) {
    if (dirOrSprNode == NULL)
        return 1;

    if (strcmp(dirOrSprNode->name(), "spr") == 0) {
        int sprX = atoi(dirOrSprNode->first_attribute("x")->value());
        int sprY = atoi(dirOrSprNode->first_attribute("y")->value());
        int sprWidth = atoi(dirOrSprNode->first_attribute("w")->value());
        int sprHeight = atoi(dirOrSprNode->first_attribute("h")->value());
        if (spritePacker(srcBitmap, &sprX, &sprY, &sprWidth, &sprHeight, dstBitmap, binPacker, padding) == 1) {
            char intVal[33];
            memset(intVal, 0, sizeof(intVal));
            sprintf(intVal, "%i", sprX);
            dirOrSprNode->first_attribute("x")->value(doc->allocate_string(intVal));
            memset(intVal, 0, sizeof(intVal));
            sprintf(intVal, "%i", sprY);
            dirOrSprNode->first_attribute("y")->value(doc->allocate_string(intVal));
            memset(intVal, 0, sizeof(intVal));
            sprintf(intVal, "%i", sprWidth);
            dirOrSprNode->first_attribute("w")->value(doc->allocate_string(intVal));
            memset(intVal, 0, sizeof(intVal));
            sprintf(intVal, "%i", sprHeight);
            dirOrSprNode->first_attribute("h")->value(doc->allocate_string(intVal));
        }
        else {
            return 0;
        }
    }
    else if (strcmp(dirOrSprNode->name(), "dir") == 0) {
        return spriteMerge(doc, srcBitmap, dirOrSprNode->first_node(), dstBitmap, binPacker, padding);
    }

    return spriteMerge(doc, srcBitmap, dirOrSprNode->next_sibling(), dstBitmap, binPacker, padding);
}

// do a sprites file
int spritesMerge (const char* workingDirectory, xml_document<>* doc, 
                  FIBITMAP* dstBitmap, BinPacker& binPacker, const char* resultFile, int currentDstWidth, int currentDstHeight, 
                  int padding) {
    xml_node<>* imgNode = doc->first_node("img");
    string imgFileName = workingDirectory;
    imgFileName += imgNode->first_attribute("name")->value();
    FIBITMAP* srcBitmap = FreeImage_Load(FIF_PNG, imgFileName.c_str(), PNG_DEFAULT);
    if(!srcBitmap) {
        printf("image %s load failed\n", imgFileName.c_str());
        return -1;
    }

    xml_node<>* definitionsNode = imgNode->first_node("definitions");
    xml_node<>* dirOrSprNode = definitionsNode->first_node();
    if (spriteMerge(doc, srcBitmap, dirOrSprNode, dstBitmap, binPacker, padding)) {
        FreeImage_Unload(srcBitmap);
        imgNode->first_attribute("name")->value(doc->allocate_string(resultFile));
        char intVal[33];
        memset(intVal, 0, sizeof(intVal));
        sprintf(intVal, "%i", currentDstWidth);
        imgNode->first_attribute("w")->value(doc->allocate_string(intVal));
        memset(intVal, 0, sizeof(intVal));
        sprintf(intVal, "%i", currentDstHeight);
        imgNode->first_attribute("h")->value(doc->allocate_string(intVal));
        return 1;
    }
    else {
        FreeImage_Unload(srcBitmap);
        return 0;
    }
}

int main(int argc, char** argv)
{
    int maxDstWidth = 64, maxDstHeight = 64;
    int currentDstWidth = 64, currentDstHeight = 64;
    bool expandStep = 0;

    FIBITMAP* srcBitmap = NULL;
    FIBITMAP* dstBitmap = NULL;
    BinPacker* binPacker = NULL;

    map<string, string> spriteXmls;
    int start;

    int padding = 5;
    vector<string> spritesFileNames;
    char resultFile[255];
    memset(resultFile, 0, sizeof(resultFile));

    // get sprites file names from arg
    bool isInit = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            for (int j = i + 1; j < argc; j++) {
                if (strcmp(argv[j], "-o") == 0) {
                    break;
                }
                spritesFileNames.push_back(argv[j]);
                isInit = true;
            }
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            strcpy(resultFile, argv[i + 1]);
        }
        else if (strcmp(argv[i], "-padding") == 0 && i + 1 < argc) {
            padding = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-maxw") == 0 && i + 1 < argc) {
            maxDstWidth = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-maxh") == 0 && i + 1 < argc) {
            maxDstHeight = atoi(argv[i + 1]);
        }
    }

    if (!isInit) {
        printf("example: SpritesPackerMergeCL -padding 5 -maxw 1024 -maxh 1024 -i input1.sprites input2.sprites input3.sprites -o output.png\n");
        return 1; // error args
    }

    // init freeimage
    FreeImage_Initialise(TRUE);

    // start = clock();
    bool isFailed = true;
    while (currentDstWidth <= maxDstWidth && currentDstHeight <= maxDstHeight) {
        dstBitmap = FreeImage_AllocateT(FIT_BITMAP, currentDstWidth, currentDstHeight, 32);
        binPacker = new BinPacker(currentDstWidth, currentDstHeight);

        // do
        vector<string>::iterator spritesFileNameIter;
        for (spritesFileNameIter = spritesFileNames.begin(); 
            spritesFileNameIter != spritesFileNames.end(); 
            spritesFileNameIter++) {
            // load sprites file to XML
            ifstream spritesFile;
            spritesFile.open(spritesFileNameIter->c_str(), ios::in);
            if (!spritesFile.is_open()) {
                printf("sprites file %s load failed\n", spritesFileNameIter->c_str());
                FreeImage_Unload(dstBitmap);
                delete binPacker;
                return 2; // sprites file load failed
            }
            string xml;
            spritesFile.seekg(0, ios::end);
            xml.reserve(spritesFile.tellg());
            spritesFile.seekg(0, ios::beg);
            xml.assign(istreambuf_iterator<char>(spritesFile),
                    istreambuf_iterator<char>());
            spritesFile.close();

            // parse sprites file
            vector<char> xmlCopy(xml.begin(), xml.end());
            xmlCopy.push_back('\0');
            xml_document<> doc;
            doc.parse<parse_full>(&xmlCopy[0]);

            // actual do sprites merge
            size_t found = spritesFileNameIter->find_last_of("/\\");
            string workingDirectory = spritesFileNameIter->substr(0, found + 1);

            string resultFileName = resultFile;
            found = spritesFileNameIter->find_last_of("/\\");
            resultFileName = resultFileName.substr(found + 1);

            int result = spritesMerge(workingDirectory.c_str(), &doc, dstBitmap, *binPacker, resultFileName.c_str(), currentDstWidth, currentDstHeight, padding);

            if (result == -1) {
                FreeImage_Unload(dstBitmap);
                delete binPacker;
                return 3; // sprites image file load failed
            }
            else if (result == 1) {
                // merge success then do next sprites file
                string outXml;
                print(back_inserter(outXml), doc, 0);
                spriteXmls.insert(pair<string, string>(*spritesFileNameIter, outXml));
                isFailed = false;
            }
            else if (result == 0) {
                // need more result image area to merge
                FreeImage_Unload(dstBitmap);
                delete binPacker;
                spriteXmls.clear();
                isFailed = true;
                break;
            }
        }

        if (isFailed) {
            // expand result image area and retry merge
            if (expandStep == 0) {
                currentDstWidth *= 2;
                expandStep = 1;
            }
            else if (expandStep = 1) {
                currentDstHeight *= 2;
                expandStep = 0;
            }
        }
        else {
            // merge success
            break;
        }
    }
    // printf("time elapsed: %i ms\n", (clock() - start));

    if (!isFailed) {
        // overwrite sprites files
        map<string, string>::iterator spriteXmlsIter;
        for (spriteXmlsIter = spriteXmls.begin(); spriteXmlsIter != spriteXmls.end(); spriteXmlsIter++) {
            ofstream spritesFile;
            spritesFile.open(spriteXmlsIter->first.c_str(), ios::out | ios::trunc);
            spritesFile << spriteXmlsIter->second;
            spritesFile.close();
        }

        // save result bitmap to image
        FreeImage_Save(FIF_PNG, dstBitmap, resultFile, PNG_DEFAULT);
        printf("save result to file %s with size width:%i height:%i\n", 
            resultFile, currentDstWidth, currentDstHeight);
        FreeImage_Unload(dstBitmap);
        delete binPacker;
    }
    else {
        printf("need more area to save result image\n");
        return 4; // need more area to save result image
    }

    // release resource
    FreeImage_DeInitialise();
    return 0;
}