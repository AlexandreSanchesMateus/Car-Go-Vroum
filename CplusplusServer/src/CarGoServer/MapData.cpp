#include "CarGoServer/MapData.hpp"

void from_json(const nlohmann::json& j, physx::PxVec3& vec) {
	vec = physx::PxVec3(j["X"], j["Y"], j["Z"]);
}

void from_json(const nlohmann::json& j, physx::PxQuat& quat) {
	quat = physx::PxQuat(j["X"], j["Y"], j["Z"], j["W"]);
}

void from_json(const nlohmann::json& j, std::vector <physx::PxVec3 > & vertices) {
	vertices.clear();
	for (const auto& elem : j) {
		physx::PxVec3 vec;
		::from_json(elem, vec);
		vertices.push_back(vec);
	}
}

CapsuleObject::CapsuleObject() {
	Type = "Capsule";
}

void CapsuleObject::from_json(const nlohmann::json& j) {
	PhysicObject::from_json(j);
	::from_json(j.at("position"), position);
	::from_json(j.at("rotation"), rotation);
	j.at("radius").get_to(radius);
	j.at("height").get_to(height);
}

void CapsuleObject::CreatePhysxObject() {
	physx::PxCapsuleGeometry capsule = physx::PxCapsuleGeometry(physx::PxReal(radius), physx::PxReal(height / 2));
}

SphereObject::SphereObject() {
	Type = "Sphere";
}

void SphereObject::from_json(const nlohmann::json& j) {
	PhysicObject::from_json(j);
	::from_json(j.at("position"), position);
	j.at("radius").get_to(radius);
}

BoxObject::BoxObject() {
	Type = "Box";
}

void BoxObject::from_json(const nlohmann::json& j) {
	PhysicObject::from_json(j);
	::from_json(j.at("position"), position);
	::from_json(j.at("rotation"), rotation);
	::from_json(j.at("extents"), extents);
}

MeshObject::MeshObject() {
	Type = "Mesh";
}

void MeshObject::from_json(const nlohmann::json& j) {
	PhysicObject::from_json(j);
	::from_json(j.at("position"), position);
	::from_json(j.at("rotation"), rotation);
	::from_json(j.at("vertices"), vertices);
	j.at("triangles").get_to(triangles);
}