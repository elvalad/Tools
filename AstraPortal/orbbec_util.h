#ifndef ORBBEC_UTIL_H
#define ORBBEC_UTIL_H

#include <QString>
#include <OpenNI.h>

#include "orbbec_common.h"

using namespace openni;

class Util
{
public:
    static bool Exist(const char* fileName)
    {
        bool exist = false;

        if (_access(fileName, 0) != -1)
        {
            //cout << "found file " << fileName << endl;
            exist = true;
        }
        else
        {
            //cout << "can't find file " << fileName << endl;
        }

        return exist;
    }
};

#endif // ORBBEC_UTIL_H
