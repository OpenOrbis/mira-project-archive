#pragma once

enum loaderkeytype
{
	KeyType_None,
	KeyType_RSA,
	KeyType_AES256,
	KeyType_SHA1,
	KeyType_COUNT
};
struct loaderkey_t
{
	enum loaderkeytype keyType;

	// TODO: Finish signature verification once the elf loader actually works >_>
};