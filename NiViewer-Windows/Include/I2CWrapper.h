#include <OpenNI.h>

bool WriteDepthI2C(openni::Device& Device, int address, int value);
bool ReadDepthI2C(openni::Device& Device, int address, int& value);
