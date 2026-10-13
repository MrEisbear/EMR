// EisbearsModRuntime.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include "Versioning.h"

// TODO: Reference additional headers your program requires here.
// Imports

// Methods
void InitRuntime();

// Structs and Vars
namespace EMR {
	inline const SemVer RuntimeVersion{
		.major = 1,
		.minor = 0,
		.patch = 0,
		.prerelease = "alpha.1",
	};
}