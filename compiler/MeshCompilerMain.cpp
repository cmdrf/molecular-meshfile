/*	MeshCompilerMain.cpp

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

#include "MeshCompiler.h"
#include "util/FileStreamStorage.h"
#include "util/StringUtils.h"
#include "PrecomputedRadianceTransfer.h"
#include "MeshUtils.h"
#include "ColladaFile.h"
#include "ColladaToMesh.h"
#include "triListOpt.h"
#include "CommandLineParser.h"

int main(int argc, char** argv)
{
	CommandLineParser cmd;
	CommandLineParser::PositionalArg<std::string> inFileName(cmd, "input file", "Input mesh to compile");
	CommandLineParser::PositionalArg<std::string> outFileName(cmd, "output file", "Output compiled mesh file");
	CommandLineParser::Flag prt(cmd, "prt", "Enable radiance transfer precomputation");
	CommandLineParser::Option<float> scale(cmd, "scale", "Mesh scale factor", 1.0);

	try
	{
		cmd.Parse(argc, argv);
		if(!inFileName || !outFileName)
		{
			std::cerr << "Both input and output file names are required\n";
			return EXIT_FAILURE;
		}

		FileWriteStorage outFile(*outFileName);
		MeshSet meshSet;
		if(StringUtils::EndsWith(*inFileName, ".obj"))
		{
			FileReadStorage inFile(*inFileName);
			TextReadStream<FileReadStorage> trs(inFile);
			ObjFile objFile(trs);
			meshSet = MeshCompiler::ObjFileToMeshSet(objFile);
		}
		else if(StringUtils::EndsWith(*inFileName, ".dae"))
		{
			ColladaFile file(inFileName->c_str());
			meshSet = ColladaToMesh::ToMesh(file);
		}
		else
		{
			std::cerr << *inFileName << ": Unknown format" << std::endl;
			return EXIT_FAILURE;
		}

		if(scale)
		{
			for(auto& mesh: meshSet)
				MeshUtils::Scale(mesh, *scale);
		}
		if(prt)
		{
			auto samples = SphericalHarmonics::SetupSphericalSamples<3>();
			for(auto& mesh: meshSet)
				PrecomputedRadianceTransfer::CalculateDiffuseShadowed(mesh, samples);
		}

		// Precision reduction:
		for(auto& mesh: meshSet)
			MeshUtils::ReducePrecision(mesh);

		{
			for(auto& mesh: meshSet)
			{
				uint32_t* indices = mesh.GetIndices().data();
				assert(indices);
				TriListOpt::OptimizeTriangleOrdering(mesh.GetNumVertices(), mesh.GetIndices().size(), indices, indices);
			}
		}
		MeshCompiler::Compile(meshSet, outFile);
	}
	catch(std::exception& e)
	{
		std::cerr << inFileName << ": " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
