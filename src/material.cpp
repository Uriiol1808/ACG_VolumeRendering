#include "material.h"
#include "texture.h"
#include "application.h"
#include "extra/hdre.h"
#include "volume.h"

unsigned int tf_texture = 1;

StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

StandardMaterial::~StandardMaterial()
{

}

void StandardMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_color", color);

	if (texture)
		shader->setUniform("u_texture", texture);
}

void StandardMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
}

WireframeMaterial::WireframeMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial()
{

}

void WireframeMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//enable shader
		shader->enable();

		//upload material specific uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

//VolumeMaterial

VolumeMaterial::VolumeMaterial()
{
	//Volume shader
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/volumetric.fs");

	//Lab1
	step_length = 0.01f;
	brightness = 1.6f;
	coloralpha = 0.01f;

	//Lab2
	//Jittering
	check_jittering = false;
	texture_width = Application::instance->noise_texture->width;
	check_jittering2 = false;

	//Transfer Function
	check_transfer_function = false;
	transfer_function1 = Application::instance->transfer_function1;
	transfer_function2 = Application::instance->transfer_function2;
	transfer_function_texture = transfer_function1;
	threshold = 0.2f;

	//Clipping
	check_clipping = false;
	clipping_plane = Vector4(0.5, 0.5, 0.5, -1.5);
}

VolumeMaterial::~VolumeMaterial()
{
	
}

void VolumeMaterial::renderInMenu()
{
	ImGui::Text("VOLUME RENDERING");

	//Lab1
	ImGui::SliderFloat("Step Length", (float*)&step_length, 0.001f, 0.2f);
	ImGui::SliderFloat("Brightness", (float*)&brightness, 0.0f, 20.0f);
	ImGui::ColorEdit4("Color", (float*)&color);
	ImGui::SliderFloat("Alpha", (float*)&coloralpha, 0.0f, 0.2f);

	//Lab2
	//Jittering
	ImGui::Checkbox("Jittering 1st", &check_jittering);
	ImGui::Checkbox("Jiterring 2nd", &check_jittering2);

	//Transfer Function
	ImGui::Checkbox("Transfer Function", &check_transfer_function);
	if (check_transfer_function) {

		//Change transfer function texture
		bool texture = false;
		ImGui::Text("Change Isosurface texture");
		texture |= ImGui::Combo("Texture", (int*)&tf_texture, "TF1\0TF2\0", 2);
		if (texture) {
			if (tf_texture == 0)
				transfer_function_texture = transfer_function2;
			else
				transfer_function_texture = transfer_function1;

		}
		ImGui::SliderFloat("Threshold", (float*)&threshold, (int)0, int(1));
	}

	//Clipping
	ImGui::Checkbox("Clipping", &check_clipping);
	if (check_clipping) {
		ImGui::DragFloat4("Clipping plane", (float*)&clipping_plane, 0.01f);
	}
}

void VolumeMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//Upload standard material uniforms
	StandardMaterial::setUniforms(camera, model); 

	//Upload uniforms
	//Lab1
	shader->setUniform("u_brightness", brightness);
	shader->setUniform("stepLength", step_length);
	shader->setUniform("alpha_color", coloralpha);

	//Upload inverse model
	Matrix44 inv_model = model;
	inv_model.inverse();
	shader->setUniform("u_inv_model", inv_model);
	
	//Lab2
	//Jittering
	shader->setUniform("u_jittering", check_jittering); 
	shader->setUniform("u_noise_texture", Application::instance->noise_texture, 1);
	shader->setUniform("u_texture_width", texture_width);
	shader->setUniform("u_jittering2", check_jittering2);

	//Transfer Function
	shader->setUniform("u_check_transfer_function", check_transfer_function);
	shader->setUniform("u_transfer_function_texture", transfer_function_texture, 2);
	shader->setUniform("u_threshold", threshold);

	//Clipping
	shader->setUniform("u_clipping_check", check_clipping);
	shader->setUniform("u_clipping_plane", clipping_plane);
}

void VolumeMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

//VolumeMaterialIsosurfaces

VolumeMaterialIso::VolumeMaterialIso() {

	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/volumetric_isosurfaces.fs");
	light_position = vec3(10.0f, 5.0f, 10.0f);

	h = 0.01;
	normals = false;
}

VolumeMaterialIso::~VolumeMaterialIso() {

}

void VolumeMaterialIso::renderInMenu() {

	VolumeMaterial::renderInMenu();

	ImGui::Text("Isosurfaces");
	ImGui::Checkbox("Normals", &normals);
	ImGui::SliderFloat("H", (float*)&h, 0.01f, 0.05f);
	ImGui::DragFloat3("Light position", (float*)&light_position, 0.1f);
}

void VolumeMaterialIso::setUniforms(Camera* camera, Matrix44 model) {

	VolumeMaterial::setUniforms(camera, model);

	shader->setUniform("u_h", h);
	shader->setUniform("u_normals", normals);
	shader->setUniform("u_light_position", light_position);
}