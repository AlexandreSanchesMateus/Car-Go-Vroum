﻿#pragma once

class ClientCar
{
public:
	~ClientCar();

	ClientCar(const ClientCar&) = default;
	ClientCar(ClientCar&&) = delete;

	ClientCar& operator=(const ClientCar&) = delete;
	ClientCar& operator=(ClientCar&&) = delete;

	static ClientCar CreateClientCar(int spawnId, bool isInfected);

private:
	ClientCar();

	// jsp
	// std::unique_ptr<PxActor> m_actor;
};