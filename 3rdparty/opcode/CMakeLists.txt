add_library(opcode STATIC
	Opcode.cpp
	OPC_VolumeCollider.cpp
	OPC_TreeCollider.cpp
	OPC_TreeBuilders.cpp
	OPC_SphereCollider.cpp
	OPC_RayCollider.cpp
	OPC_PlanesCollider.cpp
#	OPC_Picking.cpp
    OPC_OptimizedTree.cpp
	OPC_OBBCollider.cpp
	OPC_Model.cpp
	OPC_MeshInterface.cpp
	OPC_LSSCollider.cpp
	OPC_HybridModel.cpp
#	OPC_Common.cpp # has no symbols
    OPC_Collider.cpp
	OPC_BaseModel.cpp
	OPC_AABBTree.cpp
	OPC_AABBCollider.cpp
	Ice/IceUtils.cpp
	Ice/IceTriangle.cpp
	Ice/IceSegment.cpp
	Ice/IceRevisitedRadix.cpp
	Ice/IceRay.cpp
	Ice/IceRandom.cpp
	Ice/IcePoint.cpp
	Ice/IcePlane.cpp
	Ice/IceOBB.cpp
	Ice/IceMatrix4x4.cpp
	Ice/IceMatrix3x3.cpp
	Ice/IceIndexedTriangle.cpp
	Ice/IceHPoint.cpp
	Ice/IceContainer.cpp
	Ice/IceAABB.cpp
)
target_include_directories(opcode PUBLIC .)
