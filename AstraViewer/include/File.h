#pragma once

class File
{
public:
	static bool Exist(char* fileName)
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