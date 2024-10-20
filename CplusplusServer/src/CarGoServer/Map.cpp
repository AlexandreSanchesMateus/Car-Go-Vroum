#include "CarGoServer/Constant.hpp"
#include "CarGoServer/Map.hpp"
#include "CarGoServer/ClientCar.hpp"
#include "CarGoServer/GameData.hpp"
#include <fstream>
#include <physx/cooking/PxCooking.h>
#include <random>
#include <fmt/core.h>
#include <fmt/color.h>

Map::Map(GameData& gameData) : m_carSimulationCallback(gameData, *this)
{
	std::srand(std::time(nullptr));
	fmt::println("Initialize map ...");
    InitPhysics();
}

Map::~Map()
{
	Release();
}

void Map::SimulatePhysics(float elapsedTime)
{
	m_gScene->simulate(elapsedTime);
	m_gScene->fetchResults(true);
}

void Map::Clear(GameData& gameData)
{
	// remove all cars
	for (Player& player : gameData.players)
	{
		player.car = nullptr;
	}

	for (const std::shared_ptr<ClientCar>& car : m_clientCars)
	{
		m_gScene->removeActor(car->GetPhysixActor(), false);
	}

	m_clientCars.clear();
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

	boxShape1->setContactOffset(0.01f); // unity default
	boxShape1->setRestOffset(0.f); // unity default
	boxShape2->setContactOffset(0.01f); // unity default
	boxShape2->setRestOffset(0.f); // unity default

	dynamicCar->attachShape(*boxShape1);
	dynamicCar->attachShape(*boxShape2);

	// physx::PxRigidBodyExt::setMassAndUpdateInertia(*dynamicCar, 500.0f);
	dynamicCar->setMassSpaceInertiaTensor(physx::PxVec3(1116.45f, 1213.20f, 179.91f));
	dynamicCar->setMass(500.f);
	dynamicCar->setSolverIterationCounts(6, 1); // unity default
	dynamicCar->setMaxDepenetrationVelocity(10); // unity default

	dynamicCar->setLinearVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
	dynamicCar->setCMassLocalPose(physx::PxTransform(physx::PxVec3(0, -0.05f, 0.25f)));

	dynamicCar->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);

	m_gScene->addActor(*dynamicCar);

	boxShape1->release();
	boxShape2->release();

	return dynamicCar;
}

void Map::InitPlayers(GameData& gameData)
{
	// Copie id list
	std::vector<std::uint16_t> playersId;
	for (const Player& player : gameData.players) 
	{
		if(player.peer != nullptr && !player.IsPending())
			playersId.push_back(player.index);
	}

	// From id list, take x id and set to infected
	int nbInfected = ceil(playersId.size() * InfectedPercentage);
	fmt::print(stderr, fg(fmt::color::yellow), "INFO :");
	fmt::println(" {} infected in the game", nbInfected);
	for (int i = 0; i < nbInfected; ++i)
	{
		std::vector<std::uint16_t>::iterator it = playersId.begin();
		int index = std::rand() % playersId.size();
		std::advance(it, index);

		fmt::println("Player infected index : {} => {}", index, *it);

		auto playerIt = std::find_if(gameData.players.begin(), gameData.players.end(), [&](const Player& player) {return player.index == *it; });
		if (playerIt != gameData.players.end())
		{
			playerIt->isInfected = true;
			fmt::println("Player found, name : {} => {}", playerIt->name, playerIt->isInfected);

			playersId.erase(it);
		}
	}

	int infectedCount = 0;
	int survivorCount = 0;
	for (Player& player : gameData.players)
	{
		if (player.peer == nullptr || player.IsPending())
			continue;

		if (player.isInfected)
			player.spawnSlotId = infectedCount++;
		else
			player.spawnSlotId = survivorCount++;

		std::shared_ptr<ClientCar> carController = std::make_shared<ClientCar>(ClientCar(player.index, CreateRigidCar(player.spawnSlotId, player.isInfected), m_gScene, m_gPhysics));
		m_clientCars.emplace_back(carController);
		player.car = carController;
	}
}

void Map::InitPhysics()
{
    m_gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_gAllocator, m_gErrorCallback);

    m_gPvd = physx::PxCreatePvd(*m_gFoundation);
    physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    m_gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

    m_gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_gFoundation, physx::PxTolerancesScale(), true, m_gPvd);

    m_gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);

    physx::PxSceneDesc sceneDesc(m_gPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.cpuDispatcher = m_gDispatcher;
	sceneDesc.solverType = physx::PxSolverType::ePGS;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    m_gScene = m_gPhysics->createScene(sceneDesc);

	m_gScene->setSimulationEventCallback(&m_carSimulationCallback);

    m_gMaterial = m_gPhysics->createMaterial(0.6f, 0.6f, 0.f); // Unity Default Params

	fmt::print("    => ");
	fmt::print(stderr, fg(fmt::color::green), "Physx Initialized\n");

	UnserializeMap(MapPath);

	fmt::print("    => ");
	fmt::print(stderr, fg(fmt::color::green), "Map Deserialized\n\n");
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

	m_mapData.from_json(data);

	for (const auto& obj : m_mapData.physicObjects)
	{
		if (obj) 
		{
			if (obj->type == "capsule")
			{
				CapsuleObject* capsule = dynamic_cast<CapsuleObject*>(obj.get());

				physx::PxRigidStatic* capsuleStaticActor = m_gPhysics->createRigidStatic(physx::PxTransform(capsule->position));
				physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
				physx::PxShape* aCapsuleShape = physx::PxRigidActorExt::createExclusiveShape(*capsuleStaticActor, physx::PxCapsuleGeometry(capsule->radius, capsule->height / 2), *m_gMaterial);
				aCapsuleShape->setLocalPose(relativePose);
				m_gScene->addActor(*capsuleStaticActor);
			}
			else if (obj->type == "sphere") 
			{
				SphereObject* sphere = dynamic_cast<SphereObject*>(obj.get());

				physx::PxRigidStatic* sphereStaticActor = m_gPhysics->createRigidStatic(physx::PxTransform(sphere->position));

				physx::PxShape* sphereShape = physx::PxRigidActorExt::createExclusiveShape(*sphereStaticActor,
					physx::PxSphereGeometry(sphere->radius), *m_gMaterial);
				
				m_gScene->addActor(*sphereStaticActor);
			}
			else if (obj->type == "box")
			{
				BoxObject* box = dynamic_cast<BoxObject*>(obj.get());

				physx::PxRigidStatic* boxStaticActor = m_gPhysics->createRigidStatic(physx::PxTransform(box->position, box->rotation));

				physx::PxShape* sphereShape = physx::PxRigidActorExt::createExclusiveShape(*boxStaticActor,
					physx::PxBoxGeometry(box->extents / 2), *m_gMaterial);

				m_gScene->addActor(*boxStaticActor);
			}
			else if (obj->type == "mesh") 
			{
				MeshObject* mesh = dynamic_cast<MeshObject*>(obj.get());
				physx::PxTriangleMeshDesc meshDesc;
				meshDesc.points.count = mesh->vertices.size();
				meshDesc.points.stride = sizeof(physx::PxVec3); 
				meshDesc.points.data = mesh->vertices.data();

				meshDesc.triangles.count = mesh->triangles.size() / 3;
				meshDesc.triangles.stride = 3 * sizeof(physx::PxU32); 
				meshDesc.triangles.data = mesh->triangles.data(); 

				physx::PxTolerancesScale scale;

				physx::PxDefaultMemoryOutputStream writeBuffer;
				physx::PxTriangleMeshCookingResult::Enum result;

				bool status = PxCookTriangleMesh(physx::PxCookingParams(scale), meshDesc, writeBuffer, &result);
				if (!status) {
					fmt::println("Failed to cook triangle mesh.");
				}

				physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
				physx::PxTriangleMesh* triangleMesh = m_gPhysics->createTriangleMesh(readBuffer);
				
				physx::PxRigidStatic* staticActor = m_gPhysics->createRigidStatic(physx::PxTransform(mesh->position, mesh->rotation));

				physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*staticActor, physx::PxTriangleMeshGeometry(triangleMesh), *m_gMaterial);
				shape->setContactOffset(0.01f);
				shape->setRestOffset(0.f);

				m_gScene->addActor(*staticActor);
			}
		}
	}
}

void Map::Release()
{
	m_gScene->release();
	m_gDispatcher->release();
	m_gPhysics->release();
	m_gFoundation->release();
}

physx::PxFoundation* Map::GetFoundation() const
{
	return m_gFoundation;
}

physx::PxPhysics* Map::GetPhysics() const
{
	return m_gPhysics;
}
