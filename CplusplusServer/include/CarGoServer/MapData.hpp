#pragma once
#include <vector>
#include <iostream>
#include <physx/PxPhysicsAPI.h>
#include <nlohmann/json.hpp>

using namespace physx;

void from_json(const nlohmann::json& j, PxVec3& vec) {
	vec = PxVec3(j[0], j[1], j[2]);
}

void from_json(const nlohmann::json& j, PxQuat& quat) {
	quat = PxQuat(j[0], j[1], j[2], j[3]);
}

void from_json(const nlohmann::json& j, std::vector<PxVec3>& vertices) {
	vertices.clear();
	for (const auto& elem : j) {
		PxVec3 vec;
		::from_json(elem, vec);
		vertices.push_back(vec);
	}
}

class PhysicObject {
public:
	std::string Type;

	virtual ~PhysicObject() = default;

	virtual void from_json(const nlohmann::json& j) {
        j.at("Type").get_to(Type);
    }

	virtual void CreatePhysxObject() {}
};

class CapsuleObject : public PhysicObject {
public:
	PxVec3 position;
	PxQuat rotation;
	float radius;
	float height;

	CapsuleObject() {
		Type = "Capsule";
	}

	void from_json(const nlohmann::json& j) override {
		PhysicObject::from_json(j);
		::from_json(j.at("position"), position);
		::from_json(j.at("rotation"), rotation);
		::from_json(j.at("radius"), radius);
		j.at("height").get_to(height);
	}

	void CreatePhysxObject() {
		physx::PxCapsuleGeometry capsule = physx::PxCapsuleGeometry(physx::PxReal(radius), physx::PxReal(height / 2));
	}
};

class SphereObject : public PhysicObject {
public:
	PxVec3 position;
	PxVec3 radius;

	SphereObject() {
		Type = "Sphere";
	}

	void from_json(const nlohmann::json& j) override {
		PhysicObject::from_json(j);
		::from_json(j.at("position"), position);
		::from_json(j.at("radius"), radius);
	}
};

class BoxObject : public PhysicObject {
public:
	PxVec3 position;
	PxVec3 scale;

	BoxObject() {
		Type = "Box";
	}

	void from_json(const nlohmann::json& j) override {
		PhysicObject::from_json(j);
		::from_json(j.at("position"), position);
		::from_json(j.at("scale"), scale);
	}
};


class MeshObject : public PhysicObject {
public:
	std::vector<PxVec3> vertices;
	std::vector<int> triangles;

	MeshObject() {
		Type = "Mesh";
	}

	void from_json(const nlohmann::json& j) override {
		PhysicObject::from_json(j);
		::from_json(j.at("vertices"), vertices);
		j.at("triangles").get_to(triangles);
	}
};

class MapData {
public:
	std::vector<std::shared_ptr<PhysicObject>> physicObjects;

	// Fonction pour créer un objet physique en fonction du type
	std::shared_ptr<PhysicObject> createPhysicObject(const std::string& type) {
		if (type == "Capsule") {
			return std::make_shared<CapsuleObject>();
		}
		else if (type == "Sphere") {
			return std::make_shared<SphereObject>();
		}
		else if (type == "Box") {
			return std::make_shared<BoxObject>();
		}
		else if (type == "Mesh") {
			return std::make_shared<MeshObject>();
		}
		else {
			std::cerr << "Type inconnu: " << type << std::endl;
			return nullptr; // Retourne nullptr pour un type inconnu
		}
	}

	void from_json(const nlohmann::json& j) {
		if (j.contains("physicObjects")) {
			for (const auto& item : j.at("physicObjects")) {
				// Vérification du type et création de l'objet
				std::string type = item.at("Type").get<std::string>();
				auto obj = createPhysicObject(type); // Appel à createPhysicObject

				if (obj) {
					// Désérialise l'objet
					obj->from_json(item);
					physicObjects.push_back(obj); // Ajoute l'objet au vecteur
				}
			}
		}
		else {
			std::cerr << "Erreur: Le champ 'physicObjects' est manquant dans le JSON." << std::endl;
		}
	}
};