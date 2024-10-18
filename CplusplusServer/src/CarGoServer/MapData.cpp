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
	type = "capsule";
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
	type = "sphere";
}

void SphereObject::from_json(const nlohmann::json& j) {
	PhysicObject::from_json(j);
	::from_json(j.at("position"), position);
	j.at("radius").get_to(radius);
}

BoxObject::BoxObject() {
	type = "box";
}

void BoxObject::from_json(const nlohmann::json& j) {
	PhysicObject::from_json(j);
	::from_json(j.at("position"), position);
	::from_json(j.at("rotation"), rotation);
	::from_json(j.at("extents"), extents);
}

MeshObject::MeshObject() {
	type = "mesh";
}

void MeshObject::from_json(const nlohmann::json& j) {
	PhysicObject::from_json(j);
	::from_json(j.at("position"), position);
	::from_json(j.at("rotation"), rotation);
	::from_json(j.at("vertices"), vertices);
	j.at("triangles").get_to(triangles);
}

std::shared_ptr<PhysicObject> MapData::createPhysicObject(const std::string& type) {
	if (type == "capsule") {
		return std::make_shared<CapsuleObject>();
	}
	else if (type == "sphere") {
		return std::make_shared<SphereObject>();
	}
	else if (type == "box") {
		return std::make_shared<BoxObject>();
	}
	else if (type == "mesh") {
		return std::make_shared<MeshObject>();
	}
	else {
		std::cerr << "Type inconnu: " << type << std::endl;
		return nullptr; // Retourne nullptr pour un type inconnu
	}
}

void MapData::from_json(const nlohmann::json& j) {
	if (j.contains("SceneObjects")) {
		for (const auto& item : j.at("SceneObjects")) {
			// Vérification du type et création de l'objet
			std::string type = item.at("type").get<std::string>();
			auto obj = createPhysicObject(type); // Appel à createPhysicObject

			if (obj) {
				// Désérialise l'objet
				obj->from_json(item);
				physicObjects.push_back(obj); // Ajoute l'objet au vecteur
			}
		}
	}
	else {
		std::cerr << "Erreur: Le champ 'SceneObjects' est manquant dans le JSON." << std::endl;
	}
}