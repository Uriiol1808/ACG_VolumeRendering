#ifndef MATERIAL_H
#define MATERIAL_H

#include "framework.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "extra/hdre.h"

class Material {
public:

	Shader* shader = NULL;
	Texture* texture = NULL;
	vec4 color;

	virtual void setUniforms(Camera* camera, Matrix44 model) = 0;
	virtual void render(Mesh* mesh, Matrix44 model, Camera * camera) = 0;
	virtual void renderInMenu() = 0;
};

class StandardMaterial : public Material {
public:

	StandardMaterial();
	~StandardMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera * camera);
	void renderInMenu();
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

// TODO: Derived class VolumeMaterial

class VolumeMaterial : public StandardMaterial {
public:

	//Lab1
	float step_length;
	float brightness;
	float coloralpha;

	//Lab2 
	//Jittering
	bool check_jittering;
	float texture_width;
	bool check_jittering2;

	//Transfer Function
	bool check_transfer_function;
	Texture* transfer_function1;
	Texture* transfer_function2;
	Texture* transfer_function_texture;
	float threshold;

	//Clipping
	bool check_clipping;
	vec4 clipping_plane;

	VolumeMaterial();
	~VolumeMaterial();

	void renderInMenu();
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void setUniforms(Camera* camera, Matrix44 model);
};

class VolumeMaterialIso : public VolumeMaterial {
public:

	Vector3 ambient;
	Vector3 light_position;

	float h;
	bool normals;

	VolumeMaterialIso();
	~VolumeMaterialIso();

	void renderInMenu();
	void setUniforms(Camera* camera, Matrix44 model);
};

#endif