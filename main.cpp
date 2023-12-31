#include "gl_render.hpp"

using namespace Boundless;
int main()
{
	system("chcp 65001");
#ifdef _DEBUG
	std::cout << "<DEBUG>" << std::endl;
#endif
	if (!opengl_init())
	{
		exit(EXIT_FAILURE);
	}

	while (!glfwWindowShouldClose(window_ptr))
	{

		glfwSwapBuffers(window_ptr);
		glfwPollEvents();
	}

    glfwDestroyWindow(window_ptr);
	glfwTerminate();
	system("pause");
	exit(EXIT_SUCCESS);
}