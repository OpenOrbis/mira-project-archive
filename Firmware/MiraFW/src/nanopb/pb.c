#include "pb.h"
#include <string.h>
#include <oni/utils/kdlsym.h>

void init_nanopb()
{
	memset = kdlsym(memset);
	memcpy = kdlsym(memcpy);
}