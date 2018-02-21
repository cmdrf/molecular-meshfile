# Mesh Compiler
add_executable(molecularmeshcompiler
	MeshCompilerMain.cpp

	CharacterAnimation.cpp
	CharacterAnimation.h
	ColladaFile.cpp
	ColladaFile.h
	ColladaToMesh.cpp
	ColladaToMesh.h
	CommandLineParser.cpp
	CommandLineParser.h
	Mesh.h
	MeshCompiler.cpp
	MeshCompiler.h
	MeshUtils.cpp
	MeshUtils.h
	ObjFile.cpp
	ObjFile.h
	PrecomputedRadianceTransfer.cpp
	PrecomputedRadianceTransfer.h

	util/AxisAlignedBox.h
	util/FileStreamStorage.cpp
	util/FileStreamStorage.h
	util/FloatToHalf.cpp
	util/FloatToHalf.h
	util/Hash.h
	util/Math.h
	util/Range.h
	util/SphericalHarmonics.cpp
	util/SphericalHarmonics.h
	util/StreamStorage.cpp
	util/StreamStorage.h
	util/StringUtils.cpp
	util/StringUtils.h
	util/TextStream.h
	util/Vector.h
	util/Vector3.h
	util/Vector4.h
)
target_include_directories(molecularmeshcompiler PRIVATE ../runtime)
target_link_libraries(molecularmeshcompiler pugixml opcode trilistopt)
