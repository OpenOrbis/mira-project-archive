#define _CRT_SECURE_NO_WARNINGS

#include "capnp_c.h"
#include "mirabuiltin.capnp.h"
#include "fileexplorer.capnp.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef false
#define false 0
#define true 1
#endif

#define magix 2

#pragma optimize( "", off )

static capn_text chars_to_text(const char* chars, int len)
{
	return (capn_text)
	{
		.len = len,
			.str = chars,
			.seg = NULL
	};
}

void chainofevents()
{
	uint64_t bits = 0;
	uint32_t value = 321;

	uint8_t magic = (uint8_t)(bits & 0x3);
	if (magic != magix)
		goto loop;

	uint8_t category = (uint8_t)((bits & 0x3C) >> 2);
	if (category < 0 || category > 15)
		goto cont;

	uint8_t request = (uint8_t)((bits & 0x40) >> 6) == 1;
	if (request == false)
		goto loop;

	int32_t error = (int32_t)((bits & 0x7FFFFFFF80) >> 7);
	if (error < 0)
		goto loop;

	uint32_t type = (uint32_t)((bits & 0x7FFFFFFF80) >> 7);
	if (type > 0 && error < 0)
		goto cont;

	bits = (bits & 0xFFFFFF800000007F);
	bits |= ((uint32_t)value << 7);

	if ((uint32_t)((bits & 0x7FFFFFFF80) >> 7) != type)
		goto cont;

	uint16_t payloadLength = (uint16_t)((bits & 0x7FFF8000000000) >> 39);
	uint16_t oldPayloadLength = payloadLength;

	bits = (bits & 0xFF80007FFFFFFFFF);
	bits |= (int32_t)((int32_t)value << 39);

	payloadLength = (uint16_t)((bits & 0x7FFF8000000000) >> 39);
	if (payloadLength != oldPayloadLength)
		goto loop;

	uint32_t cnt = 0;

loop:
	for (;;)
	{
		cnt++;

		if (cnt >= 33)
		{
			cnt = 0;
		}
	}

cont:
	cnt = 1;

	uint8_t data[] = { 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41 };
	uint8_t buf[0x1000];
	int32_t bufDataLen = 0;

	uint8_t echoBuf[0x1000];
	int32_t echoBufLen = 0;

	{

		const char message[] = "hello fucking world";

		struct capn c;
		capn_init_malloc(&c);

		capn_ptr cr = capn_root(&c);
		struct capn_segment* cs = cr.seg;

		struct EchoRequest msg =
		{
			.message = chars_to_text(message, sizeof(message))
		};

		EchoRequest_ptr msgp = new_EchoRequest(cs);
		write_EchoRequest(&msg, msgp);

		int setp_ret = capn_setp(cr, 0, msgp.p);
		if (setp_ret != 0)
			return -1;

		echoBufLen = capn_write_mem(&c, echoBuf, sizeof(echoBuf), false);
		capn_free(&c);
	}

	{
		// Write the message
		struct capn c;
		capn_init_malloc(&c);

		capn_ptr cr = capn_root(&c);
		struct capn_segment* cs = cr.seg;


		struct Message msg =
		{
			.type = 0xEBDB1342,
			.category = MessageCategory_file,
			.containedMessage = chars_to_text(echoBuf, echoBufLen)
		};

		Message_ptr msgp = new_Message(cs);
		write_Message(&msg, msgp);

		int setp_ret = capn_setp(cr, 0, msgp.p);
		if (setp_ret != 0)
			return -1;

		bufDataLen = capn_write_mem(&c, buf, sizeof(buf), false);
		capn_free(&c);
	}

	FILE* file = fopen("dump.bin", "wb");
	fwrite(&bufDataLen, sizeof(bufDataLen), 1, file);
	fwrite(buf, sizeof(uint8_t), bufDataLen, file);
	fclose(file);
	file = NULL;

	uint8_t* finalBuffer = NULL;
	int32_t finalBufferLen = 0;

	{
		uint8_t* buf2 = (uint8_t*)malloc(bufDataLen);
		printf("errno: %d\n", errno);
		if (buf2 == NULL)
			return -2;

		memset(buf2, 0xFE, bufDataLen);
		memcpy(buf2, buf, bufDataLen);

		struct capn rc;
		int init_mem_ret = capn_init_mem(&rc, buf2, bufDataLen, false);
		if (init_mem_ret != 0)
		{
			return -1;
		}
		Message_ptr root;
		struct Message msg;
		root.p = capn_getp(capn_root(&rc), 0, true);
		read_Message(&msg, root);

		printf("got msg (%d) (%d)\n", msg.category, msg.type);

		finalBuffer = (const uint8_t*)msg.containedMessage.str;
		finalBufferLen = msg.containedMessage.len;


		//capn_free(&rc);
	}

	{
		struct capn rc;
		/*uint8_t buffer[0x8000];

		(void)memset(buffer, 0xFE, sizeof(buffer));
		(void)memcpy(buffer, finalBuffer, finalBufferLen);*/

		int32_t ret = capn_init_mem(&rc, finalBuffer, finalBufferLen, false);
		if (ret != 0)
		{
			return -3;
		}


		EchoRequest_ptr root;
		struct EchoRequest request;
		root.p = capn_getp(capn_root(&rc), 0, true);
		read_EchoRequest(&request, root);

		printf("me hoy minoy: %s", request.message.str);
	}
}

#pragma optimize( "", on ) 

//// b:0 l:2
//public byte Magic
//{
//
//	get
//	{
//	// Only the first 2 bits make up the magic
//	return (byte)(m_Bits & 0x3);
//}
//set
//{
//	if (value > 3 || value < 0)
//		value = 0;
//
//// Clear our lower 2 bits
//m_Bits = (m_Bits & 0xFFFFFFFFFFFFFFFC);
//
//// Set our bits
//m_Bits |= ((ulong)value & 0x3);
//}
//}
//
//// b:2 l:4
//public MessageCategory Category
//{
//	get
//	{
//		return (MessageCategory)((m_Bits & 0x3C) >> 2);
//	}
//	set
//	{
//		var s_Category = value;
//		if (s_Category < 0 || s_Category >= MessageCategory.Max)
//			s_Category = MessageCategory.None;
//
//		m_Bits = (m_Bits & 0xFFFFFFFFFFFFFFC3);
//		m_Bits |= ((ulong)s_Category << 2);
//	}
//}
//
//// b:6 l:1
//public bool IsRequest
//	{
//		get
//		{
//			return ((m_Bits & 0x40) >> 6) == 1;
//		}
//		set
//		{
//			m_Bits = (m_Bits & 0xFFFFFFFFFFFFFFBF);
//			m_Bits |= (ulong)(value ? 1 : 0) << 6;
//		}
//	}
//
//		// b:7 l:32
//			public int Error
//		{
//			get
//			{
//				return (int)((m_Bits & 0x7FFFFFFF80) >> 7);
//			}
//			set
//			{
//				m_Bits = (m_Bits & 0xFFFFFF800000007F);
//				m_Bits |= (uint)((ulong)value << 7);
//			}
//		}
//
//			// b:7 l:32
//				public uint Type
//			{
//				get
//				{
//					return (uint)((m_Bits & 0x7FFFFFFF80) >> 7);
//				}
//				set
//				{
//					m_Bits = (m_Bits & 0xFFFFFF800000007F);
//					m_Bits |= ((ulong)value << 7);
//				}
//			}
//
//				// b:39 l:16
//					public ushort PayloadLength
//				{
//					get
//					{
//						return (ushort)((m_Bits & 0x7FFF8000000000) >> 39);
//					}
//					set
//					{
//						m_Bits = (m_Bits & 0xFF80007FFFFFFFFF);
//						m_Bits |= ((ulong)value << 39);
//					}
//				}


int main()
{
	chainofevents();

	uint8_t data[] = { 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41 };
	uint8_t buf[0x1000];
	int32_t bufDataLen = 0;

	uint8_t echoBuf[0x1000];
	int32_t echoBufLen = 0;

	{

		const char message[] = "hello fucking world";

		struct capn c;
		capn_init_malloc(&c);

		capn_ptr cr = capn_root(&c);
		struct capn_segment* cs = cr.seg;

		struct EchoRequest msg =
		{
			.message = chars_to_text(message, sizeof(message))
		};

		EchoRequest_ptr msgp = new_EchoRequest(cs);
		write_EchoRequest(&msg, msgp);

		int setp_ret = capn_setp(cr, 0, msgp.p);
		if (setp_ret != 0)
			return -1;

		echoBufLen = capn_write_mem(&c, echoBuf, sizeof(echoBuf), false);
		capn_free(&c);
	}

	{
		// Write the message
		struct capn c;
		capn_init_malloc(&c);

		capn_ptr cr = capn_root(&c);
		struct capn_segment* cs = cr.seg;
		

		struct Message msg = 
		{
			.type = 0xEBDB1342,
			.category = MessageCategory_file,
			.containedMessage = chars_to_text(echoBuf, echoBufLen)
		};

		Message_ptr msgp = new_Message(cs);
		write_Message(&msg, msgp);

		int setp_ret = capn_setp(cr, 0, msgp.p);
		if (setp_ret != 0)
			return -1;

		bufDataLen = capn_write_mem(&c, buf, sizeof(buf), false);
		capn_free(&c);
	}

	FILE* file = fopen("dump.bin", "wb");
	fwrite(&bufDataLen, sizeof(bufDataLen), 1, file);
	fwrite(buf, sizeof(uint8_t), bufDataLen, file);
	fclose(file);
	file = NULL;

	uint8_t* finalBuffer = NULL;
	int32_t finalBufferLen = 0;

	{
		uint8_t* buf2 = (uint8_t*)malloc(bufDataLen);
		printf("errno: %d\n", errno);
		if (buf2 == NULL)
			return -2;

		memset(buf2, 0xFE, bufDataLen);
		memcpy(buf2, buf, bufDataLen);

		struct capn rc;
		int init_mem_ret = capn_init_mem(&rc, buf2, bufDataLen, false);
		if (init_mem_ret != 0)
		{
			return -1;
		}
		Message_ptr root;
		struct Message msg;
		root.p = capn_getp(capn_root(&rc), 0, true);
		read_Message(&msg, root);

		printf("got msg (%d) (%d)\n", msg.category, msg.type);

		finalBuffer = (const uint8_t*)msg.containedMessage.str;
		finalBufferLen = msg.containedMessage.len;


		//capn_free(&rc);
	}

	{
		struct capn rc;
		/*uint8_t buffer[0x8000];

		(void)memset(buffer, 0xFE, sizeof(buffer));
		(void)memcpy(buffer, finalBuffer, finalBufferLen);*/

		int32_t ret = capn_init_mem(&rc, finalBuffer, finalBufferLen, false);
		if (ret != 0)
		{
			return -3;
		}


		EchoRequest_ptr root;
		struct EchoRequest request;
		root.p = capn_getp(capn_root(&rc), 0, true);
		read_EchoRequest(&request, root);

		printf("me hoy minoy: %s", request.message.str);
	}
	return 0;
}