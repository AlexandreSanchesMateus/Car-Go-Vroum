#include "CarGoServer/Constant.hpp"
#include "CarGoServer/Map.hpp"
#include "CarGoServer/ClientCar.hpp"
#include "CarGoServer/GameData.hpp"
#include <fstream>
#include <physx/cooking/PxCooking.h>
#include <random>
#include <fmt/core.h>
#include <fmt/color.h>

Map::Map()
{
	// init physicx
	// load map
    InitPhysics();
	std::srand(std::time(nullptr));
}

Map::~Map()
{
	// unload physicx
	Release();
}

void Map::UpdatePhysics(float elapsedTime)
{
	// update all cars
	m_gScene->simulate(elapsedTime);
}

void Map::Clear()
{
	// remove all cars
}

physx::PxRigidDynamic* Map::CreateRigidCar(std::uint8_t spawnSlotId, bool isInfected)
{
	physx::PxRigidDynamic* dynamicCar;

	if (isInfected)
		dynamicCar = m_gPhysics->createRigidDynamic(InfectedSpawns[spawnSlotId]);
	else 
		dynamicCar = m_gPhysics->createRigidDynamic(SurvivorSpawns[spawnSlotId]);

	physx::PxShape* boxShape1 = m_gPhysics->createShape(physx::PxBoxGeometry(physx::PxVec3(0.95, 0.3, 2.75)), *m_gMaterial);
	physx::PxShape* boxShape2 = m_gPhysics->createShape(physx::PxBoxGeometry(physx::PxVec3(0.7, 0.245, 0.975)), *m_gMaterial);
	boxShape2->setLocalPose(physx::PxTransform(physx::PxVec3(0.0, 0.5, 0.0)));

	dynamicCar->attachShape(*boxShape1);
	dynamicCar->attachShape(*boxShape2);

	physx::PxRigidBodyExt::setMassAndUpdateInertia(*dynamicCar, 5.0f);

	dynamicCar->setLinearVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));

	m_gScene->addActor(*dynamicCar);

	boxShape1->release();
	boxShape2->release();

	return dynamicCar;
}

void Map::InitPlayers(GameData& gameData)
{
	// determine who is infected and who's not
	std::vector<std::uint16_t> playersId;
	for (const Player& player : gameData.players) 
	{
		if(player.peer != nullptr && !player.IsPending())
			playersId.push_back(player.index);
	}

	int infectedCount = round(playersId.size() * InfectedPercentage);
	for (std::vector<std::uint16_t>::const_iterator i = playersId.begin(); i != playersId.end();) 
	{
		int index = std::rand() % playersId.size();
		if (!gameData.players[index].isInfected)
		{
			auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&](const Player& player) {return player.index == index;});
			if (it == gameData.players.end())
			{
				++i;
				continue;
			}

			it->isInfected = true;
			i = playersId.erase(i);
		}
		else
			++i;
	}

	// add cars dynamic body (by static function of ClientCar(int spawnId, bool isInfected))
	// store in a list 
	for (const Player& player : gameData.players) 
	{
		ClientCar vroum(CreateRigidCar(player.spawnSlotId, player.isInfected));
	}
}


void Map::InitPhysics() 
{
	fmt::println("Initialize map ...");

    m_gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_gAllocator, m_gErrorCallback);

    m_gPvd = physx::PxCreatePvd(*m_gFoundation);
    physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    m_gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

    m_gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_gFoundation, physx::PxTolerancesScale(), true, m_gPvd);

    m_gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);

    physx::PxSceneDesc sceneDesc(m_gPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f); 
    sceneDesc.cpuDispatcher = m_gDispatcher;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    m_gScene = m_gPhysics->createScene(sceneDesc);

    m_gMaterial = m_gPhysics->createMaterial(0.5f, 0.5f, 0.6f);  
}

void Map::UnserializeMap(std::string mapPath) {
	std::ifstream file(mapPath);

	if (!file.is_open()) 
	{
		std::cerr << "Erreur lors de l'ouverture du fichier : " << mapPath << std::endl;
		return;
	}

	nlohmann::json data;
	try 
	{
		data = nlohmann::json::parse(file);
	}
	catch (const std::exception& e) 
	{
		std::cerr << "Erreur lors de l'analyse du fichier JSON: " << e.what() << std::endl;
		return;
	}

	MapData mapData;
	mapData.from_json(data);

	for (const auto& obj : mapData.physicObjects)
	{
		if (obj) 
		{
			if (obj->Type == "Capsule")
			{
				CapsuleObject* capsule = dynamic_cast<CapsuleObject*>(obj.get());

				physx::PxRigidStatic* capsuleStaticActor = m_gPhysics->createRigidStatic(physx::PxTransform(capsule->position));
				physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
				physx::PxShape* aCapsuleShape = physx::PxRigidActorExt::createExclusiveShape(*capsuleStaticActor,
					physx::PxCapsuleGeometry(capsule->radius, capsule->height / 2), *m_gMaterial);
				aCapsuleShape->setLocalPose(relativePose);
				m_gScene->addActor(*capsuleStaticActor);
			}
			else if (obj->Type == "Sphere") 
			{
				SphereObject* sphere = dynamic_cast<SphereObject*>(obj.get());

				physx::PxRigidStatic* sphereStaticActor = m_gPhysics->createRigidStatic(physx::PxTransform(sphere->position));

				physx::PxShape* sphereShape = physx::PxRigidActorExt::createExclusiveShape(*sphereStaticActor,
					physx::PxSphereGeometry(sphere->radius), *m_gMaterial);
				
				m_gScene->addActor(*sphereStaticActor);
			}
			else if (obj->Type == "Box")
			{
				BoxObject* box = dynamic_cast<BoxObject*>(obj.get());

				physx::PxRigidStatic* boxStaticActor = m_gPhysics->createRigidStatic(physx::PxTransform(box->position, box->rotation));

				physx::PxShape* sphereShape = physx::PxRigidActorExt::createExclusiveShape(*boxStaticActor,
					physx::PxBoxGeometry(box->extents / 2), *m_gMaterial);

				m_gScene->addActor(*boxStaticActor);
			}
			else if (obj->Type == "Mesh") 
			{
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
