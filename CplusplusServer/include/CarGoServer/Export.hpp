#pragma once

// A ajouter dans toute les class ayant un cpp

#ifdef SERVER_STATIC
	#define OWNED_ENGINE_API
#else
    #ifdef _WIN32
		#ifdef SERVER_BUILD
			#define OWNED_ENGINE_API __declspec(dllexport)
		#else
			#define OWNED_ENGINE_API __declspec(dllimport)
		#endif // OWNED_ENGINE_BUILD
	#else
		#define OWNED_ENGINE_API __attribute__((visibility("default")))
	#endif // _WIN32
#endif // OWNED_ENGINE_STATIC
