#include "bl_log.hpp"
#include "bl_initialization.hpp"
#include "boundless.hpp"

#include "glad/glad.h"
#include "glfw/glfw3.h"

#include <iostream>
#include <cstdlib>

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
	ADSRender::ADSData adsd;
	adsd.materialindex = 0;
	adsd.vertexcolor = Vector4f(0.2f,0.3f,0.1f,1.0f);
	LightProp* ld = lightdata;
	ld[0].enable = true;
	ld[0].edited = true;
	ld[0].isSpot = false;
	ld[0].isLocal = true;
	ld[0].position = Vector3f(2.0f,0.0f,0.0f);
	ld[0].color = Vector3f(0.98f,0.98f,0.98f);
	ld[0].ambient = Vector3f(0.1f,0.1f,0.1f);
	ld[0].constantAttenuation = 0.2f;
	ld[0].linearAttenuation = 0.7f;
	ld[0].quadraticAttenuation = 0.01f;
	ld[1].edited = false;
	MaterialProp* mp = materialdata;
	mp[0].edited = true;
	mp[0].shininess = 8.0f;
	mp[0].ambient = Vector3f(0.1f,0.1f,0.1f);
	mp[0].diffuse = Vector3f(0.6f,0.6f,0.6f);
	mp[0].emission = Vector3f(0.0f,0.0f,0.0f);
	mp[0].specular = Vector3f(0.2f,0.2f,0.2f);
	ADSBase::Init();
	ADSBase::UpdateUniformBuffer();
	Renderer rend;
	Transform* node = rend.AddObject<ADSRender>(Vector3d(0.0,1.0,1.0),Vector3d(1.0,1.0,1.0),Quaterniond::Identity(), &adsd);
	ADSRender* ro = dynamic_cast<ADSRender*>(node->render_obj);
	Mesh& mesh = ro->mesh;
	Mesh::MakeSphere(mesh,1.0f,32,32,VertexData::POSITION);
	ro->base_transform->roenble = true;
	
	while (!glfwWindowShouldClose(Init::windowinfo.window_ptr))
	{
		Init::process_input(Init::windowinfo.window_ptr);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
		rend.DrawAll();

		glfwSwapBuffers(Init::windowinfo.window_ptr);
		glfwPollEvents();
	}

    glfwDestroyWindow(Init::windowinfo.window_ptr);
	glfwTerminate();
	system("pause");
	exit(EXIT_SUCCESS);
}