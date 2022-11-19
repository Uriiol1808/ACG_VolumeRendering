#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"
#include "volume.h"

unsigned int SceneNode::lastNameId = 0;
unsigned int mesh_selected = 0;
unsigned int volumes = 0;
bool isosurface = false;

SceneNode::SceneNode()
{
	this->name = std::string("Node" + std::to_string(lastNameId++));
}

SceneNode::SceneNode(const char * name)
{
	this->name = name;
}

SceneNode::~SceneNode()
{

}

void SceneNode::render(Camera* camera)
{
	if (material)
		material->render(mesh, model, camera);
}

void SceneNode::renderWireframe(Camera* camera)
{
	WireframeMaterial mat = WireframeMaterial();
	mat.render(mesh, model, camera);
}

void SceneNode::renderInMenu()
{
	//Model edit
	if (ImGui::TreeNode("Model")) 
	{
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
		ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
		ImGui::DragFloat3("Scale", matrixScale, 0.1f);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);
		
		ImGui::TreePop();
	}

	//Material
	if (material && ImGui::TreeNode("Material"))
	{
		//Volumes
		ImGui::Text("Change volume");
		bool changed = false;
		changed |= ImGui::Combo("Volumes", (int*)&volumes, "ABDOMEN\0BONSAI\0TEAPOT");

		//Isosurfaces
		bool iso = false;
		ImGui::Text("Activate Isosurfaces");
		iso |= ImGui::Checkbox("Isosurface", (bool*)&isosurface);
		if (iso) {
			if (isosurface)
				material = new VolumeMaterialIso();
			else
				material = new VolumeMaterial();
		}

		ImGui::TreePop();
	}
	material->renderInMenu();

	//Geometry
	if (mesh && ImGui::TreeNode("Geometry"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Mesh", (int*)&mesh_selected, "SPHERE\0");

		ImGui::TreePop();
	}
}

//VolumeNode

VolumeNode::VolumeNode(const char* name) {

	this->name = name;

	// Mesh
	Mesh* mesh = new Mesh();
	mesh->createCube();
	this->mesh = mesh;

	// Volume
	Volume* volume = new Volume();
	volume->loadPVM("data/volumes/CT-Abdomen.pvm");
	//volume->loadPNG("data/volumes/teapot_16_16.png");
	//volume->loadPNG("data/volumes/bonsai_16_16.png");

	// Scale model
	int normalize = volume->width * volume->widthSpacing;
	this->model.scale(1,
		(volume->height * volume->heightSpacing) / normalize,
		(volume->depth * volume->depthSpacing) / normalize);

	// Texture
	Texture* texture = new Texture();
	texture->create3DFromVolume(volume);

	// Material
	VolumeMaterial* material = new VolumeMaterial();
	this->material = material;
	material->texture = texture;
	// Shaders en el constructor de VolumeMaterial

}

VolumeNode::~VolumeNode() {
	material->shader->~Shader();
	material->texture->~Texture();
	mesh->~Mesh();
}
