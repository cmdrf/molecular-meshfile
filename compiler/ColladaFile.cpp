/*	ColladaFile.cpp

MIT License

Copyright (c) 2018 Fabian Herb

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ColladaFile.h"
#include "util/StringUtils.h"
#include <stdexcept>

ColladaFile::ColladaFile(const char* filename)
{
	pugi::xml_parse_result result = mDocument.load_file(filename);
	if(!result)
		throw std::runtime_error(result.description());
	mCollada = mDocument.child("COLLADA");
}

ColladaFile::ColladaFile(void* fileContents, size_t size)
{
	pugi::xml_parse_result result = mDocument.load_buffer_inplace(fileContents, size);
	if(!result)
		throw std::runtime_error(result.description());
	mCollada = mDocument.child("COLLADA");
}

ColladaFile::Asset ColladaFile::GetAsset() const
{
	auto asset = mCollada.child("asset");
	if(!asset)
		throw std::runtime_error("No asset");
	return Asset(asset);
}

std::vector<ColladaFile::Animation> ColladaFile::GetAnimations() const
{
	std::vector<Animation> out;
	for(auto& input: mCollada.child("library_animations").children("animation"))
		out.push_back(Animation(input));
	return out;
}

ColladaFile::Scene ColladaFile::GetScene() const
{
	auto scene = mCollada.child("scene");
	if(!scene)
		throw std::runtime_error("No scene");
	return Scene(scene);
}

ColladaFile::Geometry ColladaFile::GetGeometry(const char* id) const
{
	auto geometry = mCollada.child("library_geometries").find_child_by_attribute("geometry", "id", id);
	if(!geometry)
		throw std::runtime_error(std::string("Geometry ") + id + " not found");
	return Geometry(geometry);
}

ColladaFile::Geometry ColladaFile::GetGeometryByName(const char* name) const
{
	auto geometry = mCollada.child("library_geometries").find_child_by_attribute("geometry", "name", name);
	if(!geometry)
		throw std::runtime_error(std::string("Geometry ") + name + " not found");
	return Geometry(geometry);
}

ColladaFile::VisualScene ColladaFile::GetVisualScene(const char* id) const
{
	auto visualScene = mCollada.child("library_visual_scenes").find_child_by_attribute("visual_scene", "id", id);
	if(!visualScene)
		throw std::runtime_error(std::string("visual_scene \"") + id + "\" not found");
	return VisualScene(visualScene);
}

ColladaFile::Material ColladaFile::GetMaterial(const char* id) const
{
	auto material = mCollada.child("library_materials").find_child_by_attribute("material", "id", id);
	if(!material)
		throw std::runtime_error(std::string("Material \"") + id + "\" not found");
	return Material(material);
}

ColladaFile::Controller ColladaFile::GetController(const char* id) const
{
	auto controller = mCollada.child("library_controllers").find_child_by_attribute("controller", "id", id);
	if(!controller)
		throw std::runtime_error(std::string("Controller \"") + id + "\" not found");
	return Controller(controller);
}

std::vector<ColladaFile::Controller> ColladaFile::GetControllers() const
{
	std::vector<Controller> out;
	for(auto& input: mCollada.child("library_controllers").children("controller"))
		out.push_back(Controller(input));
	return out;
}

/******************************************************************************/

static std::vector<float> ReadFloatArray(const char* text)
{
	std::vector<float> out;
	const char* beginPtr = text;
	char* endPtr = nullptr;
	while(true)
	{
		float value = strtof(beginPtr, &endPtr);
		if(beginPtr == endPtr)
			break;
		out.push_back(value);
		beginPtr = endPtr;
	}
	return out;
}

static std::vector<int> ReadIntArray(const char* text)
{
	std::vector<int> out;
	const char* beginPtr = text;
	char* endPtr = nullptr;
	while(true)
	{
		int value = strtol(beginPtr, &endPtr, 0);
		if(beginPtr == endPtr)
			break;
		out.push_back(value);
		beginPtr = endPtr;
	}
	return out;
}

static std::vector<Hash> ReadNameArray(const char* text)
{
	std::vector<Hash> out;
	const char* begin = text;
	while(true)
	{
		while(isspace(*begin))
			begin++;
		if(*begin == '\0')
			return out;
		const char* end = begin;
		while(*end != '\0' && !isspace(*end))
			end++;
		out.push_back(HashUtils::MakeHash(begin, end));
		begin = end;
	}
}

/******************************************************************************/

Matrix4 ColladaFile::Base::GetMatrix4(const char* element) const
{
	auto matrix = mXmlNode.child(element);
	if(!matrix)
		return Matrix4::Identity();
	 auto values = ReadFloatArray(matrix.child_value());
	 if(values.size() != 16)
		 throw std::runtime_error("Invalid number of entries in transform matrix");
	 return Matrix4(values.data());
}

ColladaFile::Param ColladaFile::Accessor::GetParam() const
{
	return GetChild<Param>("param");
}

ColladaFile::Channel ColladaFile::Animation::GetChannel() const
{
	return GetChild<Channel>("channel");
}

ColladaFile::Sampler ColladaFile::Animation::GetSampler() const
{
	return GetChild<Sampler>("sampler");
}

ColladaFile::Sampler ColladaFile::Animation::GetSampler(const char* id) const
{
	return GetChild<Sampler>("sampler", id);
}

ColladaFile::Source ColladaFile::Animation::GetSource(const char* id)
{
	return GetChild<Source>("source", id);
}

ColladaFile::Skin ColladaFile::Controller::GetSkin() const
{
	return GetChild<Skin>("skin");
}

ColladaFile::Mesh ColladaFile::Geometry::GetMesh() const
{
	return GetChild<Mesh>("mesh");
}

ColladaFile::Polylist ColladaFile::Mesh::GetPolylist() const
{
	return GetChild<Polylist>("polylist");
}

ColladaFile::Triangles ColladaFile::Mesh::GetTriangles() const
{
	return GetChild<Triangles>("triangles");
}

ColladaFile::Source ColladaFile::Mesh::GetSource(const char* id) const
{
	return GetChild<Source>("source", id);
}

ColladaFile::Vertices ColladaFile::Mesh::GetVertices(const char* id) const
{
	return GetChild<Vertices>("vertices", id);
}

ColladaFile::InstanceGeometry ColladaFile::Node::GetInstanceGeometry() const
{
	return GetChild<InstanceGeometry>("instance_geometry");
}

std::vector<int> ColladaFile::Polylist::GetVertexCounts() const
{
	auto vcount = mXmlNode.child("vcount");
	if(!vcount)
		throw std::runtime_error("No vcount in polylist");
	return ReadIntArray(vcount.child_value());
}

std::vector<int> ColladaFile::Polylist::GetPrimitives() const
{
	auto p = mXmlNode.child("p");
	if(!p)
		throw std::runtime_error("No p in polylist");
	return ReadIntArray(p.child_value());
}

const char* ColladaFile::Scene::GetInstanceVisualSceneUrl() const
{
	auto sceneUrl = mXmlNode.child("instance_visual_scene").attribute("url");
	if(!sceneUrl)
		throw std::runtime_error("No scene URL");
	return sceneUrl.value();
}

std::vector<ColladaFile::Source> ColladaFile::Skin::GetSources() const
{
	return GetChildren<Source>("source");
}

ColladaFile::Source ColladaFile::Skin::GetSource(const char* id) const
{
	return GetChild<Source>("source", id);
}

ColladaFile::VertexWeights ColladaFile::Skin::GetVertexWeights() const
{
	return GetChild<VertexWeights>("vertex_weights");
}

std::vector<float> ColladaFile::Source::GetFloatArray(const char* id) const
{
	auto floatArray = mXmlNode.find_child_by_attribute("float_array", "id", id);
	if(!floatArray)
		throw std::runtime_error(std::string("No float_array ") + id + " in source");
	return ReadFloatArray(floatArray.child_value());
}

std::vector<Matrix4> ColladaFile::Source::GetFloatArrayAsMatrices(const char* id) const
{
	auto floatArray = GetFloatArray(id);
	const size_t arraySize = floatArray.size();
	if(arraySize % 16 != 0)
		throw std::runtime_error("Odd number of matrix entries");
	std::vector<Matrix4> out(arraySize / 16);
	for(size_t i = 0; i < arraySize; i++)
		out[i / 16](i % 16 / 4, i % 4) = floatArray[i];
	return out;
}

std::vector<Hash> ColladaFile::Source::GetNameArray(const char* id) const
{
	auto nameArray = mXmlNode.find_child_by_attribute("Name_array", "id", id);
	if(!nameArray)
		throw std::runtime_error(std::string("No Name_array ") + id + " in source");
	return ReadNameArray(nameArray.child_value());
}

ColladaFile::TechniqueCommon ColladaFile::Source::GetTechniqueCommon() const
{
	return GetChild<TechniqueCommon>("technique_common");
}

ColladaFile::Accessor ColladaFile::TechniqueCommon::GetAccessor() const
{
	return GetChild<Accessor>("accessor");
}

std::vector<ColladaFile::Input> ColladaFile::Triangles::GetInputs() const
{
	return GetChildren<Input>("input");
}

std::vector<int> ColladaFile::Triangles::GetPrimitives() const
{
	auto p = mXmlNode.child("p");
	if(!p)
		throw std::runtime_error("No p in triangles");
	return ReadIntArray(p.child_value());
}

std::vector<int> ColladaFile::VertexWeights::GetVCount() const
{
	auto vcount = mXmlNode.child("vcount");
	if(!vcount)
		throw std::runtime_error("No vcount in vertex_weights");
	return ReadIntArray(vcount.child_value());
}

std::vector<int> ColladaFile::VertexWeights::GetV() const
{
	auto v = mXmlNode.child("v");
	if(!v)
		throw std::runtime_error("No v in vertex_weights");
	return ReadIntArray(v.child_value());
}
