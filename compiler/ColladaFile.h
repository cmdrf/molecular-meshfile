/*	ColladaFile.h

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

#ifndef COLLADAFILE_H
#define COLLADAFILE_H

#include "util/Matrix4.h"
#include "util/Hash.h"
#include <pugixml.hpp>
#include <vector>
#include <stdexcept>

class ColladaFile
{
private:
	class Base;
public:
	class Accessor;
	class Animation;
	class Asset;
	class Channel;
	class Controller;
	class Geometry;
	class Input;
	class InstanceController;
	class InstanceGeometry;
	class Joints;
	class Material;
	class Mesh;
	class Node;
	class Param;
	class Perspective;
	class Point;
	class Polylist;
	class Sampler;
	class Scene;
	class Skin;
	class Source;
	class TechniqueCommon;
	class Triangles;
	class VertexWeights;
	class Vertices;
	class VisualScene;

	explicit ColladaFile(const char* filename);
	explicit ColladaFile(void* fileContents, size_t size);

	Asset GetAsset() const;
	std::vector<Animation> GetAnimations() const;
	Scene GetScene() const;
	Geometry GetGeometry(const char* id) const;
	Geometry GetGeometryByName(const char* name) const;
	VisualScene GetVisualScene(const char* id) const;
	Material GetMaterial(const char* id) const;
	Controller GetController(const char* id) const;
	std::vector<Controller> GetControllers() const;

private:
	pugi::xml_document mDocument;
	pugi::xml_node mCollada;
};

class ColladaFile::Base
{
protected:
	Base(pugi::xml_node xmlNode) : mXmlNode(xmlNode) {}

	template<class T>
	std::vector<T> GetChildren(const char* element) const
	{
		std::vector<T> out;
		for(auto& input: mXmlNode.children(element))
			out.push_back(T(input));
		return out;
	}

	template<class T>
	T GetChild(const char* element, const char* id) const
	{
		auto node = mXmlNode.find_child_by_attribute(element, "id", id);
		if(!node)
			throw std::runtime_error(std::string(element) + " with id \"" + id + "\" not found");
		return T(node);
	}

	template<class T>
	T GetChild(const char* element) const
	{
		auto child = mXmlNode.child(element);
		if(!child)
			throw std::runtime_error(std::string(element) + " not present");
		return T(child);
	}

	Matrix4 GetMatrix4(const char* element) const;

	pugi::xml_node mXmlNode;
};

/// accessor
/** Parent elements: source, technique_common

	Child elements: param */
class ColladaFile::Accessor : ColladaFile::Base
{
	friend class ColladaFile;
public:
	const char* GetSource() const {return mXmlNode.attribute("source").value();}
	int GetCount() const {return mXmlNode.attribute("count").as_int();}
	int GetStride() const {return mXmlNode.attribute("stride").as_int();}
	Param GetParam() const;

private:
	Accessor(pugi::xml_node accessor) : Base(accessor) {}
};

class ColladaFile::Animation : ColladaFile::Base
{
	friend class ColladaFile;
public:
	const char* GetId() const {return mXmlNode.attribute("id").value();}
	const char* GetName() const {return mXmlNode.attribute("name").value();}
	auto GetAnimations() const {return GetChildren<Animation>("animation");}
	auto GetAnimation() const {return GetChild<Animation>("animation");}
	Sampler GetSampler() const;
	Sampler GetSampler(const char* id) const;
	auto GetSources() const {return GetChildren<Source>("source");}
	Source GetSource(const char* id);
	Channel GetChannel() const;

private:
	Animation(pugi::xml_node animation) : Base(animation) {}
};

class ColladaFile::Asset : ColladaFile::Base
{
	friend class ColladaFile;
public:
	const char* GetCreated() const {return mXmlNode.child_value("created");}
	const char* GetUpAxis() const {return mXmlNode.child_value("up_axis");}

private:
	Asset(pugi::xml_node node) : Base(node) {}
};

class ColladaFile::Channel : ColladaFile::Base
{
	friend class ColladaFile;
public:
	const char* GetSource() const {return mXmlNode.attribute("source").value();}
	const char* GetTarget() const {return mXmlNode.attribute("target").value();}

private:
	Channel(pugi::xml_node channel) : Base(channel) {}
};

class ColladaFile::Controller : ColladaFile::Base
{
	friend class ColladaFile;
public:
	Skin GetSkin() const;
	bool HasSkin() const {return mXmlNode.child("skin");}

private:
	Controller(pugi::xml_node controller) : Base(controller) {}
};

class ColladaFile::Geometry : ColladaFile::Base
{
	friend class ColladaFile;
public:
	Mesh GetMesh() const;

private:
	Geometry(pugi::xml_node geometry) : Base(geometry) {}
};

class ColladaFile::Input : ColladaFile::Base
{
	friend class ColladaFile;
public:
	const char* GetSemantic() const {return mXmlNode.attribute("semantic").value();}
	const char* GetSource() const {return mXmlNode.attribute("source").value();}
	int GetOffset() const {return mXmlNode.attribute("offset").as_int();}

private:
	Input(pugi::xml_node input) : Base(input) {}
};

class ColladaFile::InstanceController : ColladaFile::Base
{
	friend class ColladaFile;
public:
	const char* GetUrl() const {return mXmlNode.attribute("url").value();}

private:
	InstanceController(pugi::xml_node node) : Base(node) {}
};

class ColladaFile::InstanceGeometry : ColladaFile::Base
{
	friend class ColladaFile;
public:
	const char* GetUrl() const {return mXmlNode.attribute("url").value();}
	const char* GetName() const {return mXmlNode.attribute("name").value();}

private:
	InstanceGeometry(pugi::xml_node instanceGeometry) : Base(instanceGeometry) {}
};

class ColladaFile::Joints : ColladaFile::Base
{
	friend class ColladaFile;
public:
	auto GetInputs() const {return GetChildren<Input>("input");}
private:
	Joints(pugi::xml_node node) : Base(node) {}
};

class ColladaFile::Material : ColladaFile::Base
{
	friend class ColladaFile;
public:
	const char* GetId() const {return mXmlNode.attribute("id").value();}
	const char* GetName() const {return mXmlNode.attribute("name").value();}

private:
	Material(pugi::xml_node material) : Base(material) {}
};

class ColladaFile::Mesh : ColladaFile::Base
{
	friend class ColladaFile;
public:
	bool HasPolylist() const {return mXmlNode.child("polylist");}
	Polylist GetPolylist() const;
	bool HasTriangles() const {return mXmlNode.child("triangles");}
	Triangles GetTriangles() const;
	Source GetSource(const char* id) const;
	Vertices GetVertices(const char* id) const;
	auto GetSources() const {return GetChildren<Source>("source");}

private:
	Mesh(pugi::xml_node mesh) : Base(mesh) {}
};

class ColladaFile::Node : ColladaFile::Base
{
	friend class ColladaFile;
public:
	Matrix4 GetMatrix() const {return GetMatrix4("matrix");}
	bool HasInstanceGeometry() const {return mXmlNode.child("instance_geometry");}
	InstanceGeometry GetInstanceGeometry() const;
	bool HasInstanceController() const {return mXmlNode.child("instance_controller");}
	InstanceController GetInstanceController() const {return GetChild<InstanceController>("instance_controller");}
	auto GetNodes() const {return GetChildren<Node>("node");}
	const char* GetName() const {return mXmlNode.attribute("name").value();}
	const char* GetType() const {return mXmlNode.attribute("type").value();}
	const char* GetSid() const {return mXmlNode.attribute("sid").value();}

private:
	Node(pugi::xml_node node) : Base(node) {}
};

class ColladaFile::Param : ColladaFile::Base
{
	friend class ColladaFile;
public:
	const char* GetType() const {return mXmlNode.attribute("type").value();}
	const char* GetName() const {return mXmlNode.attribute("name").value();}

private:
	Param(pugi::xml_node param) : Base(param) {}
};

class ColladaFile::Polylist : ColladaFile::Base
{
	friend class ColladaFile;
public:
	std::vector<Input> GetInputs() const {return GetChildren<Input>("input");}
	std::vector<int> GetVertexCounts() const;
	std::vector<int> GetPrimitives() const;
	const char* GetMaterial() const {return mXmlNode.attribute("material").value();}
	int GetCount() const {return mXmlNode.attribute("count").as_int();}

private:
	Polylist(pugi::xml_node polylist) : Base(polylist) {}
};

class ColladaFile::Sampler : ColladaFile::Base
{
	friend class ColladaFile;
public:
	std::vector<Input> GetInputs() const {return GetChildren<Input>("input");}

private:
	Sampler(pugi::xml_node sampler) : Base(sampler) {}
};

class ColladaFile::Scene : ColladaFile::Base
{
	friend class ColladaFile;
public:
	const char* GetInstanceVisualSceneUrl() const;

private:
	Scene(pugi::xml_node scene) : Base(scene) {}
};

/// skin element, found in controller elements
class ColladaFile::Skin : ColladaFile::Base
{
	friend class ColladaFile;
public:
	Matrix4 GetBindShapeMatrix() const {return GetMatrix4("bind_shape_matrix");}

	/** @returns source children. */
	std::vector<Source> GetSources() const;

	/** @returns source child */
	Source GetSource(const char* id) const;

	/** @returns source attribute */
	const char* GetSource() const {return mXmlNode.attribute("source").value();}

	Joints GetJoints() const {return GetChild<Joints>("joints");}

	VertexWeights GetVertexWeights() const;

private:
	Skin(pugi::xml_node skin) : Base(skin) {}
};

class ColladaFile::Source : ColladaFile::Base
{
	friend class ColladaFile;
public:
	std::vector<float> GetFloatArray(const char* id) const;
	std::vector<Hash> GetNameArray(const char* id) const;
	std::vector<Matrix4> GetFloatArrayAsMatrices(const char* id) const;
	TechniqueCommon GetTechniqueCommon() const;

private:
	Source(pugi::xml_node source) : Base(source) {}
};

class ColladaFile::TechniqueCommon : ColladaFile::Base
{
	friend class ColladaFile;
public:
	Accessor GetAccessor() const;
	Point GetPoint() const;
	Perspective GetPerspective() const;

private:
	TechniqueCommon(pugi::xml_node techniqueCommon) : Base(techniqueCommon) {}
};

class ColladaFile::Triangles : ColladaFile::Base
{
	friend class ColladaFile;
public:
	std::vector<Input> GetInputs() const;
	std::vector<int> GetPrimitives() const;
	const char* GetMaterial() const {return mXmlNode.attribute("material").value();}
	int GetCount() const {return mXmlNode.attribute("count").as_int();}

private:
	Triangles(pugi::xml_node triangles) : Base(triangles) {}
};

class ColladaFile::VertexWeights : ColladaFile::Base
{
	friend class ColladaFile;
public:
	auto GetInputs() const {return GetChildren<Input>("input");}
	std::vector<int> GetVCount() const;
	std::vector<int> GetV() const;
	int GetCount() const {return mXmlNode.attribute("count").as_int();}

private:
	VertexWeights(pugi::xml_node node) : Base(node) {}
};

class ColladaFile::Vertices : ColladaFile::Base
{
	friend class ColladaFile;
public:
	auto GetInputs() const {return GetChildren<Input>("input");}

private:
	Vertices(pugi::xml_node vertices) : Base(vertices) {}
};

class ColladaFile::VisualScene : ColladaFile::Base
{
	friend class ColladaFile;
public:
	Node GetNode(const char* id) const {return GetChild<Node>("node", id);}
	auto GetNodes() const {return GetChildren<Node>("node");}

private:
	VisualScene(pugi::xml_node visualScene) : Base(visualScene) {}
};

#endif // COLLADAFILE_H
