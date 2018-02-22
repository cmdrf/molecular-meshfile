# molecular-meshfile
GPU-optimized 3D mesh file format with command line exporter and header-only runtime component.

Files contain a collection of buffers that can be copied to the GPU as-is. The exporter (called "compiler") imports OBJ or COLLADA files and executes a series of precomputation and optimization steps.

## Compiler ##

The compiler comes as a command line utility and converts meshes exported by your DCC applications to the optimized mesh files.

Features:
- Loads OBJ and COLLADA (.dae) files.
- Reorders triangles for optimized GPU cache utilization.
- Reduces precision to 16 Bit floats or integers where appropriate (e.g. normals).
- Interleaves data that is needed within the same pass. E.g. color and normals are not needed in shadow pass, so they are not interleaved with position.
- Stores vertex weights and vertex-bone relationship for skeletal animation purposes.
- Optionally performs Precomputed Radiance Transfer calculations and stores Spherical Harmonics coefficients.

## Using the File Format in Your Engine ##

``` cpp
// Load entire file into one contiguous buffer, or even mmap() your file:
const void* data = MyEngine::LoadFile("mesh.mmf");

// Cast to MeshFile struct:
const MeshFile* file = static_cast<const MeshFile*>(data);

// Check magic number and version:
assert(mesh->magic == MeshFile::kMagic);
assert(mesh->version == MeshFile::kVersion);

MyEngine::Mesh* mesh = MyEngine::CreateMesh();

// Store buffer contents on GPU:
for(unsigned int i = 0; i < file->numBuffers; ++i)
{
  mesh->StoreBuffer(
    i, // Buffer index. Referenced by index specs or vertex specs
    file.GetBufferData(i), // Pointer to the data
    file.buffers[i].size, // Size of the data in bytes
    file.buffers[i].type // Vertex buffer or index buffer
  );
}

// Index specs define separate batches to draw:
for(unsigned int i = 0; i < file.numIndexSpecs; ++i)
{ 
  const IndexBufferInfo& info = file.GetIndexSpec(i);
  mesh->AddBatch(info.mode, // Draw mode (triangles, lines, ...)
    info.type, // Data type in index buffer (uint8, uint16, uint32)
    info.buffer, // Buffer index of index buffer :). See above.
    info.offset, // Offset into index buffer
    info.count, // Number of indices
    info.vertexDataSet // Index of vertex data set. See below.
  );
  mesh->SetMaterial(info.material);
}

// Vertex data sets define vertex attributes:
for(unsigned int i = 0; i < file.numVertexDataSets; ++i)
{
  const MeshFile::VertexDataSet& vSet = file.GetVertexDataSet(i);
  
  // Each vertex spec describes one vertex attribute, e.g. normal:
  for(unsigned int vSpec = 0; vSpec < vSet.numVertexSpecs; ++vSpec)
  {
    const VertexAttributeInfo& info = file.GetVertexSpec(i, vSpec);
    mesh->AddVertexAttribute(
      i, // Index of the data set, referenced by IndexBufferInfo above
      info.semantic, // Position, normal, vertex color, ...
      info.type, // Data type like float16, int32, ...
      info.components, // Number of components for vector attributes, e.g. 3 for normals
      info.offset, // Offset into the vertex buffer in bytes
      info.stride, // Distance from one entry to the next (for interleaved data)
      info.buffer, // Buffer index of the vertex buffer
      info.normalized
    );
  }
}

// Use precalculated axis-aligned bounding box:
mesh->SetBounds(file.boundsMin, file.boundsMax);
```

## License ##

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
