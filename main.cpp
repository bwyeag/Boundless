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
	system("chcp 65001");
	system("cls");
#ifdef _DEBUG
	std::cout << "<DEBUG>" << std::endl;
#endif
	if (!Init::InitOpenGL())
	{
		exit(EXIT_FAILURE);
	}
	
	Render::Camera cam;
	cam.ModificateView();
	cam.position = Render::Vector3(0.0f,3.0f,0.0f);
	cam.dir_front = Render::Vector3(0.0f,-1.0f,0.0f);
	cam.dir_up = Render::Vector3(0.0f,0.0f,1.0f);
	cam.ModificateProj();
	cam.type = Render::ViewType::PERSPECTIVE;
	cam.pers_fov = glm::radian(45.0f);
	cam.pers_k = Init::windowinfo.screen_height/Init::windowinfo.screen_height;
	cam.znear = 0.1f;
	cam.zfar = 10.0f;
	Render::Renderer ren(&cam,Init::windowinfo.window_ptr);
	Render::TransformDataPack tdp = {
		Render::Vector3(0.0f,0.0f,0.0f),
		Render::Vector3(1.0f,1.0f,1.0f);
		Render::Quaternion()
	};
	Render::RenderObject *rendobj = ren.AddRenderObject();
	rendobj.GetTransform().Initialize(tdp);
	Render::Mesh mesh;
	mesh.Initialize(0,Resource::IndexStatus::ONLY_INDEX);
	glBindVertexArray()

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