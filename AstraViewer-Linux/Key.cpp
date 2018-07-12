

#include "Key.h"

#include <iostream>


using namespace std;


Key::Key()
{

}


Key::~Key()
{

}


int Key::Handle(Config& config, int key)
{
    switch (key)
    {
    case 27:
        cout << "Exit!" << endl;
        return -1;
        break;

    case 'L':
		if (config.alphaLabel < 100)
		{
			config.alphaLabel += 10;
		}
		break;

    case 'l':
		if (config.alphaLabel > 0)
		{
			config.alphaLabel -= 10;
		}
        break;

    case 'C':
		if (config.alphaColor < 100)
		{
			config.alphaColor += 10;
		}
		break;

    case 'c':
		if (config.alphaColor > 0)
		{
			config.alphaColor -= 10;
		}
        break;

    case 'O':
    case 'o':
        config.overlayDisplay = !config.overlayDisplay;
        break;

	case 'P':
	case 'p':
		config.savePointCloud = true;
		break;

    case '1':
		config.registrationType = 1;
        break;

    case '2':
        config.registrationType = 2;
        break;

    case '3':
		config.registrationType = 3;
        break;

    case '4':
		config.registrationType = 4;
        break;

    case '5':
		config.registrationType = 5;
        break;

	case '6':
		config.useDistortCoef = !config.useDistortCoef;
		break;

    default:
        return 0;
        break;
    }

    return 0;
}
