#include "bl_initialization.hpp"
#include "bl_render.hpp"
#include "bl_mesh.hpp"
#include "bl_log.hpp"
#include "bl_resource_load.hpp"

#include "glm/glm.hpp"
#include "glad/glad.h"
#include "glfw/glfw3.h"


using namespace Boundless;
int main()
{
	std::cout.sync_with_stdio(false);
	system("chcp 65001");
	system("cls");
#ifdef _DEBUG
	std::cout << "<DEBUG>" << std::endl;
#endif
	if (!Init::InitOpenGL())
	{
		exit(EXIT_FAILURE);
	}

	while (!glfwWindowShouldClose(Init::windowinfo.window_ptr))
	{
		glfwSwapBuffers(Init::windowinfo.window_ptr);
		glfwPollEvents();
	}

    glfwDestroyWindow(Init::windowinfo.window_ptr);
	glfwTerminate();
	system("pause");
	exit(EXIT_SUCCESS);
}