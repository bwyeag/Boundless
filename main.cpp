#include "resource_load.hpp"
#include <string>
#include <iostream>

using namespace Boundless::Resource;
int main()
{
	std::string path;
	std::cout << "Enter file path:";
	std::cin>>path;
	GenerateInitialize();
	GenerateTextureFile2D(path.c_str());
	std::cout<<"Finish.";
	std::cin.get();
}