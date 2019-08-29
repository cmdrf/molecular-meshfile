# Mesh Compiler
add_executable(molecularmeshcompiler
	MeshCompilerMain.cpp

	CharacterAnimation.cpp
	CharacterAnimation.h
	ColladaFile.cpp
	ColladaFile.h
	ColladaToMesh.cpp
	ColladaToMesh.h
	Mesh.h
	MeshCompiler.cpp
	MeshCompiler.h
	MeshUtils.cpp
	MeshUtils.h
	ObjFile.cpp
	ObjFile.h
	PrecomputedRadianceTransfer.cpp
	PrecomputedRadianceTransfer.h
)
target_include_directories(molecularmeshcompiler PRIVATE ../runtime)
target_link_libraries(molecularmeshcompiler pugixml opcode trilistopt molecular::util)
