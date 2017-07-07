#include "common.h"

std::string COMMON::getFilePathName(const string _filePath)
{
	QString filePath = QString::fromStdString(_filePath);
	QString result = "";
	for (size_t i = _filePath.size() - 1; i >= 0; i--)
	{
		if (_filePath.at(i) != '/')
		{
			result.push_front(_filePath.at(i));
		}
		else
		{
			break;
		}
	}

	result = result.remove(result.size() - 4, 4);
	return result.toStdString();
}
