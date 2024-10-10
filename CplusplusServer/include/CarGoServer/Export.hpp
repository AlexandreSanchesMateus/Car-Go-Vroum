#pragma once

// A ajouter dans toute les class ayant un cpp

#ifdef SERVER_STATIC
	#define CARGOSERVER_API
#else
    #ifdef _WIN32
		#ifdef SERVER_BUILD
			#define CARGOSERVER_API __declspec(dllexport)
		#else
			#define CARGOSERVER_API __declspec(dllimport)
		#endif // OWNED_ENGINE_BUILD
	#else
		#define CARGOSERVER_API __attribute__((visibility("default")))
	#endif // _WIN32
#endif // OWNED_ENGINE_STATIC
