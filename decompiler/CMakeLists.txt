# Mesh Decompiler
add_executable(molecularmeshdecompiler
	MeshDecompilerMain.cpp
)
target_include_directories(molecularmeshdecompiler PRIVATE ..)
target_link_libraries(molecularmeshdecompiler molecular::util)
