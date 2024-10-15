#include "CarGoServer/Constant.hpp"
#include <CarGoServer/Map.hpp>
#include <fstream>
#include <physx/cooking/PxCooking.h>

Map::Map()
{
	// init physicx
	// load map
    InitPhysics();
}

Map::~Map()
{
	// unload physicx
	Release();
}

void Map::UpdatePhysics()
{
	// update all cars
}

void Map::Clear()
{
	// remove all cars
}

void Map::InitPlayers(const GameData& gameData)
{
	// determine who is infected and who's not


	// add cars dynamic body (by static function of ClientCar(int spawnId, bool isInfected))
	// store in a list
}


void Map::InitPhysics() {
    // 1. Créer la fondation PhysX
    m_gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_gAllocator, m_gErrorCallback);

    // 2. Initialiser PhysX Visual Debugger (PVD) pour le débogage
    m_gPvd = physx::PxCreatePvd(*m_gFoundation);
    physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    m_gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

    // 3. Créer l'instance PhysX
    m_gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_gFoundation, physx::PxTolerancesScale(), true, m_gPvd);

    // 4. Créer un dispatcher pour les tâches multi-thread
    m_gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);

    // 5. Créer une scène
    physx::PxSceneDesc sceneDesc(m_gPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);  // Gravité
    sceneDesc.cpuDispatcher = m_gDispatcher;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    m_gScene = m_gPhysics->createScene(sceneDesc);

    // 6. Créer un matériau par défaut
    m_gMaterial = m_gPhysics->createMaterial(0.5f, 0.5f, 0.6f);  // friction statique, dynamique, restitution
}

void Map::UnserializeMap(std::string mapPath) {
	std::ifstream file(mapPath);

	if (!file.is_open()) {
		std::cerr << "Erreur lors de l'ouverture du fichier : " << mapPath << std::endl;
		return;
	}

	nlohmann::json data;
	try {
		data = nlohmann::json::parse(file);
	}
	catch (const std::exception& e) {
		std::cerr << "Erreur lors de l'analyse du fichier JSON: " << e.what() << std::endl;
		return;
	}

	MapData mapData;
	mapData.from_json(data);

	for (const auto& obj : mapData.physicObjects)
	{
		if (obj) {
			if (obj->Type == "Capsule") {
				CapsuleObject* capsule = dynamic_cast<CapsuleObject*>(obj.get());

				physx::PxRigidStatic* capsuleStaticActor = m_gPhysics->createRigidStatic(physx::PxTransform(capsule->position));
				physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
				physx::PxShape* aCapsuleShape = physx::PxRigidActorExt::createExclusiveShape(*capsuleStaticActor,
					physx::PxCapsuleGeometry(capsule->radius, capsule->height / 2), *m_gMaterial);
				aCapsuleShape->setLocalPose(relativePose);
				m_gScene->addActor(*capsuleStaticActor);
			}
			else if (obj->Type == "Sphere") {
				SphereObject* sphere = dynamic_cast<SphereObject*>(obj.get());

				physx::PxRigidStatic* sphereStaticActor = m_gPhysics->createRigidStatic(physx::PxTransform(sphere->position));

				physx::PxShape* sphereShape = physx::PxRigidActorExt::createExclusiveShape(*sphereStaticActor,
					physx::PxSphereGeometry(sphere->radius), *m_gMaterial);
				
				m_gScene->addActor(*sphereStaticActor);
			}
			else if (obj->Type == "Box") {
				BoxObject* box = dynamic_cast<BoxObject*>(obj.get());

				physx::PxRigidStatic* boxStaticActor = m_gPhysics->createRigidStatic(physx::PxTransform(box->position, box->rotation));

				physx::PxShape* sphereShape = physx::PxRigidActorExt::createExclusiveShape(*boxStaticActor,
					physx::PxBoxGeometry(box->extents / 2), *m_gMaterial);

				m_gScene->addActor(*boxStaticActor);
			}
			else if (obj->Type == "Mesh") {
				MeshObject* mesh = dynamic_cast<MeshObject*>(obj.get());
				physx::PxTriangleMeshDesc meshDesc;
				meshDesc.points.count = mesh->vertices.size();
				meshDesc.points.stride = sizeof(physx::PxVec3); 
				meshDesc.points.data = &mesh->vertices;

				meshDesc.triangles.count = mesh->triangles.size();
				meshDesc.triangles.stride = 3 * sizeof(physx::PxU32); 
				meshDesc.triangles.data = &mesh->triangles; 

				physx::PxTolerancesScale scale;

				physx::PxDefaultMemoryOutputStream writeBuffer;
				physx::PxTriangleMeshCookingResult::Enum result;

				bool status = PxCookTriangleMesh(physx::PxCookingParams(scale), meshDesc, writeBuffer, &result);
				if (!status) {
					printf("Failed to cook triangle mesh.\n");
				}

				physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
				physx::PxTriangleMesh* triangleMesh = m_gPhysics->createTriangleMesh(readBuffer);
				
				physx::PxRigidStatic* staticActor = m_gPhysics->createRigidStatic(physx::PxTransform(mesh->position, mesh->rotation));

				physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*staticActor, physx::PxTriangleMeshGeometry(triangleMesh), *m_gMaterial);

				m_gScene->addActor(*staticActor);
			}
		}
	}
}

void Map::Release()
{
	m_gFoundation->release();
	m_gPhysics->release();
}

physx::PxFoundation* Map::GetFoundation() const
{
	return m_gFoundation;
}

physx::PxPhysics* Map::GetPhysics() const
{
	return m_gPhysics;
}
