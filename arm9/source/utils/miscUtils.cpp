#include "miscUtils.hpp"
#include <cstring>
#include <stdio.h>
#include <unistd.h>

#include "version.hpp"

bool nitroFSGood(void) {
	bool nitroFSGood = false;
	FILE *file = fopen("nitro:/version.txt", "r");
	if(file) {
		fseek(file, 0, SEEK_END);
		int length = ftell(file);
		fseek(file, 0, SEEK_SET);

		char version[length + 1] = {0};
		fread(version, 1, length, file);
		nitroFSGood = (strcmp(version, VER_NUMBER) == 0);
		fclose(file);
	}
	return nitroFSGood;
}
