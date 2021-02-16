/*	MeshDecompilerMain.cpp

MIT License

Copyright (c) 2021 Fabian Herb

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

#include <molecular/meshfile/MeshFile.h>
#include <molecular/util/Blob.h>
#include <molecular/util/CommandLineParser.h>
#include <molecular/util/FileStreamStorage.h>
#include <molecular/util/StringUtils.h>

using namespace molecular;
using namespace molecular::util;
using namespace molecular::meshfile;

template<typename T>
void PrintTriangleIndices(const void* data, uint32_t count)
{
	const T* indexData = static_cast<const T*>(data);
	for(uint32_t i = 0; i < count; i += 3)
	{
		std::cout << "f";
		for(int iv = 0; iv < 3; ++iv)
			std::cout << " " << indexData[i + iv] + 1;
		std::cout << "\n";
	}

}

int main(int argc, char** argv)
{
	CommandLineParser cmd;
	CommandLineParser::PositionalArg<std::string> inFileName(cmd, "input file", "Input mesh to decompile");
	CommandLineParser::HelpFlag help(cmd);

	try
	{
		cmd.Parse(argc, argv);

		FileReadStorage inFile(*inFileName);
		Blob inData(inFile.GetSize());
		inFile.Read(inData.GetData(), inData.GetSize());
		const MeshFile* inMesh = static_cast<const MeshFile*>(inData.GetData());
		if(inMesh->magic != MeshFile::kMagic)
			throw std::runtime_error("Unrecognized input file type");

		std::cout << "# Created by molecularmeshdecompiler\n";
		std::cout << "o " << *inFileName << "\n";

		const MeshFile::VertexDataSet& dataset = inMesh->GetVertexDataSet(0);
		for(uint32_t v = 0; v < dataset.numVertexSpecs; ++v)
		{
			const VertexAttributeInfo& info = inMesh->GetVertexSpec(0, v);
			const MeshFile::Buffer& buffer = inMesh->GetBuffer(info.buffer);

			std::vector<float> toFloatData;
			if(info.type == VertexAttributeInfo::kHalf)
			{
				toFloatData.resize(buffer.size / 2); // TODO: Interleaved buffer, strides etc.
				// TODO proper conversion
			}
			else
			{
				toFloatData.resize(buffer.size / 4); // TODO: Interleaved buffer, strides etc.
				memcpy(toFloatData.data(), inMesh->GetBufferData(info.buffer), buffer.size);
			}

			for(int pos = 0; pos < toFloatData.size(); pos += info.components)
			{
				if(info.semantic == VertexAttributeInfo::kTextureCoords)
					std::cout << "vt";
				else if(info.semantic == VertexAttributeInfo::kPosition)
					std::cout << "v";
				else if(info.semantic == VertexAttributeInfo::kNormal)
					std::cout << "vn";

				for(int i = 0; i < info.components; ++i)
					std::cout << " " << toFloatData[pos + i];
				std::cout << "\n";
			}
		}

		std::cout << "s off\n";

		const IndexBufferInfo& indexInfo = inMesh->GetIndexSpec(0);
		const void* indexData = inMesh->GetBufferData(indexInfo.buffer);
		if(indexInfo.type == IndexBufferInfo::Type::kUInt8)
			PrintTriangleIndices<uint8_t>(indexData, indexInfo.count);
		else if(indexInfo.type == IndexBufferInfo::Type::kUInt16)
			PrintTriangleIndices<uint16_t>(indexData, indexInfo.count);
		else if(indexInfo.type == IndexBufferInfo::Type::kUInt32)
			PrintTriangleIndices<uint32_t>(indexData, indexInfo.count);
	}
	catch(std::exception& e)
	{
		std::cerr << "molecularmeshdecompiler: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
