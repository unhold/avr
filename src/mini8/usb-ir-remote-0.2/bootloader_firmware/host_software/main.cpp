/*
  Flashtool for AVRUSBBoot, an USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  Creation Date..: 2006-03-18
  Last change....: 2006-06-25
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "cflashmem.h"
#include "cbootloader.h"


typedef struct {
	const char* filename;
	bool resetAfterFlash;
} parameters_t;

static bool getParameters(parameters_t &parameters, int argc, char **argv) {
	memset(&parameters,0,sizeof(parameters));
	if (argc < 2) {
		return 0;
	}
	if (argc > 2) {
		if (strcmp(argv[1],"--reset") == 0) {
			parameters.filename = argv[2];
			parameters.resetAfterFlash = true;
		}
		else {
			parameters.filename = argv[1];
		}
		return true;
	}
	else {
		parameters.filename = argv[1];
		return true;
	}
	return false;
}


int main(int argc, char **argv) {
	parameters_t parameters;

	setvbuf (stdout,0,_IONBF ,0);

	if (!getParameters(parameters,argc,argv)) {
		fprintf(stderr, "usage: avrusbboot [--reset] filename.hex\n");
		exit(1);
	}

	CBootloader * bootloader = new CBootloader();
	unsigned int pagesize = bootloader->getPagesize();
	unsigned int lastaddr = bootloader->getLastAddr();

	printf("avrusboot:\n  Pagesize: %d\n", pagesize);

	CFlashmem * flashmem = new CFlashmem(pagesize);

	flashmem->readFromIHEX(parameters.filename);

	CPage* pPage = flashmem->getPageToAddress(lastaddr);

	if (pPage) {
		printf("\rWont fit.\n");
		return -1;
	}

	pPage = flashmem->getFirstpage();

	bool flashOk = true;
	printf("Write pages: ");
	while (pPage != NULL) {
		printf("\rWrite page at address: %d ", pPage->getPageaddress());
		int res = bootloader->writePage(pPage);
		if (res == -1) {
			printf("%s", "NOK\n");
			flashOk = false;
		}
		else if (res == +1) {
			printf("%s","OK");
		}
		pPage = pPage->getNext();
	}

	printf("\n");

	if (flashOk) {
		printf("\rFlash was ok, reseting device\n");
		bootloader->startApplication();
	}
	return 0;
}
