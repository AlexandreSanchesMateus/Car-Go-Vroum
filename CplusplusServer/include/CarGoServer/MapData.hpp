#pragma once
#include <vector>
#include <iostream>
#include <physx/PxPhysicsAPI.h>
#include <nlohmann/json.hpp>

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