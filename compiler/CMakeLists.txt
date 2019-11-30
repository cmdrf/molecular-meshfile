# Mesh Compiler
add_executable(molecularmeshcompiler
	MeshCompilerMain.cpp

	ColladaFile.cpp
	ColladaFile.h
	ColladaToMesh.cpp
	ColladaToMesh.h
	MeshCompiler.cpp
	MeshCompiler.h
	ObjFile.cpp
	ObjFile.h
	PrecomputedRadianceTransfer.cpp
	PrecomputedRadianceTransfer.h
)
target_include_directories(molecularmeshcompiler PRIVATE ..)
target_link_libraries(molecularmeshcompiler pugixml opcode trilistopt molecular::util)
