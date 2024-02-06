#include "resource_load.hpp"
#include <string>
#include <iostream>

using namespace Boundless::Resource;
int main()
{
	std::string command,path;
	std::cout << "Boundless> ";
	std::cin >> command;
	GenerateInitialize();
	while (command != "exit")
	{
		size_t pos;
		if (pos = command.find("genmodel"),pos!=std::string::npos)
		{
			std::getline(std::cin,path);
			path.erase(0,path.find_first_not_of(' '));
			GenerateModelFile(path.c_str());
			std::cout<<"All Done.\n";
		}
		else if (pos = command.find("gentexture"),pos!=std::string::npos)
		{
			std::getline(std::cin,path);
			path.erase(0,path.find_first_not_of(' '));
			GenerateTextureFile2D(path.c_str());
			std::cout<<"All Done.\n";
		}
		else
		{
			std::cout << "error: illegal commands" << std::endl;
		}
		std::cout << "Boundless> ";
		std::cin >> command;
	}
	std::cout << "exit;\n";
}