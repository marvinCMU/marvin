#ifndef RETAILOBJECTCLASS_H_
#define RETAILOBJECTCLASS_H_

#include <cstdlib>

using namespace std;

enum class RetailObject {
	// list in alphabetic order
	ADAPTER,
	AIRBORNE,
	BOOT,
	BOTTLE,
	CAP,
	DISK,
	HALLS,
	POWDER,
	TOY,
	TUMBLER,
	TUPPERWARE,
	UMBRELLA
};

enum class RetailObjectCommands {
	INFORMATION,
	OPTION,
	REVIEW
};

#endif
