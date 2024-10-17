#pragma once
#include <vector>
#include <iostream>
#include <physx/PxPhysicsAPI.h>
#include <nlohmann/json.hpp>

class PhysicObject {
public:
	std::string type;

	virtual ~PhysicObject() = default;

	virtual void from_json(const nlohmann::json& j) {
        j.at("type").get_to(type);
    }

	virtual void CreatePhysxObject() {}
};

class CapsuleObject : public PhysicObject {
public:
	physx::PxVec3 position;
	physx::PxQuat rotation;
	float radius;
	float height;

	CapsuleObject();

	void from_json(const nlohmann::json& j) override;
	void CreatePhysxObject();
};

class SphereObject : public PhysicObject {
public:
	physx::PxVec3 position;
	float radius;

	SphereObject();

	void from_json(const nlohmann::json& j) override;
};

class BoxObject : public PhysicObject {
public:
	physx::PxVec3 position;
	physx::PxQuat rotation;
	physx::PxVec3 extents;

	BoxObject();

	void from_json(const nlohmann::json& j) override;
};


class MeshObject : public PhysicObject {
public:
	physx::PxVec3 position;
	physx::PxQuat rotation;
	std::vector<physx::PxVec3> vertices;
	std::vector<int> triangles;

	MeshObject();

	void from_json(const nlohmann::json& j) override;
};

class MapData {
public:
	std::vector<std::shared_ptr<PhysicObject>> physicObjects;

	std::shared_ptr<PhysicObject> createPhysicObject(const std::string& type);

	void from_json(const nlohmann::json& j);
};