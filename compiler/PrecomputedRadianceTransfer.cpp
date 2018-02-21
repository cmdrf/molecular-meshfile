/*	PrecomputedRadianceTransfer.cpp

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

#include "PrecomputedRadianceTransfer.h"
#include "Mesh.h"
#include "util/Vector3.h"

#include <Opcode.h>
#undef for // WTF?

namespace PrecomputedRadianceTransfer
{

void CalculateDiffuseUnshadowed(Mesh& mesh, std::vector<SphericalHarmonics::Sample<3>> samples)
{
	const Vector3* normals = nullptr;
	try
	{
		auto& normalAttribute = mesh.GetAttribute(VertexAttributeInfo::kNormal);
		normals = normalAttribute.GetData<Vector3>();
	}
	catch(...)
	{
		throw std::runtime_error("PRT calculation needs vertex normals");
	}

	size_t numVertices = mesh.GetNumVertices();
	size_t numSamples = samples.size();
	std::vector<Vector3> outPrt0, outPrt1, outPrt2;
	outPrt0.reserve(numVertices);
	outPrt1.reserve(numVertices);
	outPrt2.reserve(numVertices);
	for(size_t iNormal = 0; iNormal < numVertices; ++iNormal)
	{
		Vector3d normal(normals[iNormal][0], normals[iNormal][1], normals[iNormal][2]);
		Vector<9, double> coeff;
		for(size_t iSample = 0; iSample < numSamples; ++iSample)
		{
			if(normal.DotProduct(samples[iSample].vec) > 0)
				coeff += samples[iSample].coeff;
		}
		const double factor = 4.0 * 3.1415926535897932384626433832795029 / numSamples;
		coeff *= factor;
		outPrt0.emplace_back(coeff[0], coeff[1], coeff[2]);
		outPrt1.emplace_back(coeff[3], coeff[4], coeff[5]);
		outPrt2.emplace_back(coeff[6], coeff[7], coeff[8]);
	}

	mesh.SetAttributeData(VertexAttributeInfo::kVertexPrt0, outPrt0.data(), numVertices);
	mesh.SetAttributeData(VertexAttributeInfo::kVertexPrt0, outPrt1.data(), numVertices);
	mesh.SetAttributeData(VertexAttributeInfo::kVertexPrt0, outPrt2.data(), numVertices);
	mesh.RemoveAttribute(VertexAttributeInfo::kNormal);
}

void CalculateDiffuseShadowed(Mesh& mesh, std::vector<SphericalHarmonics::Sample<3>> samples)
{
	const Vector3* normals = nullptr;
	try
	{
		auto& normalAttribute = mesh.GetAttribute(VertexAttributeInfo::kNormal);
		normals = normalAttribute.GetData<Vector3>();
	}
	catch(...)
	{
		throw std::runtime_error("PRT calculation needs vertex normals");
	}

	unsigned int numVertices = mesh.GetNumVertices();
	Opcode::MeshInterface meshInterface;
	meshInterface.SetNbTriangles(mesh.GetIndices().size() / 3);
	meshInterface.SetNbVertices(numVertices);

	static_assert(sizeof(IceMaths::Point) == 3 * sizeof(float), "Additional fields in IceMaths::Point");
	static_assert(sizeof(IceMaths::IndexedTriangle) == 3 * sizeof(uint32_t), "Additional fields in IceMaths::IndexedTriangle");
	const IceMaths::IndexedTriangle* tris = reinterpret_cast<const IceMaths::IndexedTriangle*>(mesh.GetIndices().data());
	const IceMaths::Point* vertices = static_cast<const IceMaths::Point*>(mesh.GetAttribute(VertexAttributeInfo::kPosition).GetRawData());
	if(!meshInterface.SetPointers(tris, vertices))
		throw std::runtime_error("Could not set mesh interface pointers");

	Opcode::OPCODECREATE create;
	create.mIMesh = &meshInterface;

	Opcode::Model model;
	if(!model.Build(create))
		throw std::runtime_error("Could not build model");

	Opcode::CollisionFaces collisionFaces;

	Opcode::RayCollider collider;
	collider.SetCulling(false);
	collider.SetClosestHit(false);
	collider.SetDestination(&collisionFaces);
	if(const char* error = collider.ValidateSettings())
		throw std::runtime_error(std::string("Invalid collider settings: ") + error);

	std::vector<Vector3> outPrt0, outPrt1, outPrt2;
	outPrt0.reserve(numVertices);
	outPrt1.reserve(numVertices);
	outPrt2.reserve(numVertices);
	const Vector3* positions = mesh.GetAttribute(VertexAttributeInfo::kPosition).GetData<Vector3>();
	for(unsigned int iVertex = 0; iVertex < numVertices; ++iVertex)
	{
		Vector3d normal(normals[iVertex][0], normals[iVertex][1], normals[iVertex][2]);
		Vector<9, double> coeff;
		for(auto& sample: samples)
		{
			if(normal.DotProduct(sample.vec) < 0)
				continue;
			IceMaths::Point origin(positions[iVertex]);
			IceMaths::Point direction(sample.vec[0], sample.vec[1], sample.vec[2]);
			IceMaths::Ray ray(origin, direction);
			collider.Collide(ray, model);

			auto faces = collisionFaces.GetFaces();
			bool hit = false;
			for(unsigned int iCols = 0; iCols < collisionFaces.GetNbFaces(); ++iCols)
			{
				if(faces[iCols].mDistance > 0.01)
				{
					hit = true;
					break;
				}
			}
			if(!hit)
				coeff += sample.coeff; // No hit found
			collisionFaces.Reset();
		}
		coeff *= 4.0 * 3.1415926535897932384626433832795029 / samples.size();
		outPrt0.emplace_back(coeff[0], coeff[1], coeff[2]);
		outPrt1.emplace_back(coeff[3], coeff[4], coeff[5]);
		outPrt2.emplace_back(coeff[6], coeff[7], coeff[8]);
	}
	mesh.SetAttributeData(VertexAttributeInfo::kVertexPrt0, outPrt0.data(), numVertices);
	mesh.SetAttributeData(VertexAttributeInfo::kVertexPrt1, outPrt1.data(), numVertices);
	mesh.SetAttributeData(VertexAttributeInfo::kVertexPrt2, outPrt2.data(), numVertices);
}

}
