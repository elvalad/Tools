#pragma once

#include "Config.h"


class Key
{
public:
    Key();
    ~Key();

	int Handle(Config& config, int key);

};