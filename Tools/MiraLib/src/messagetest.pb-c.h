/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: messagetest.proto */

#ifndef PROTOBUF_C_messagetest_2eproto__INCLUDED
#define PROTOBUF_C_messagetest_2eproto__INCLUDED

#include "protobuf-c.h"

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1002001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _MyHeader MyHeader;
typedef struct _MyOtherMessage MyOtherMessage;


/* --- enums --- */


/* --- messages --- */

struct  _MyHeader
{
  ProtobufCMessage base;
  uint32_t category;
  uint32_t type;
  int32_t error;
};
#define MY_HEADER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&my_header__descriptor) \
    , 0, 0, 0 }


struct  _MyOtherMessage
{
  ProtobufCMessage base;
  MyHeader *header;
  uint32_t yamsayin;
};
#define MY_OTHER_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&my_other_message__descriptor) \
    , NULL, 0 }


/* MyHeader methods */
void   my_header__init
                     (MyHeader         *message);
size_t my_header__get_packed_size
                     (const MyHeader   *message);
size_t my_header__pack
                     (const MyHeader   *message,
                      uint8_t             *out);
size_t my_header__pack_to_buffer
                     (const MyHeader   *message,
                      ProtobufCBuffer     *buffer);
MyHeader *
       my_header__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   my_header__free_unpacked
                     (MyHeader *message,
                      ProtobufCAllocator *allocator);
/* MyOtherMessage methods */
void   my_other_message__init
                     (MyOtherMessage         *message);
size_t my_other_message__get_packed_size
                     (const MyOtherMessage   *message);
size_t my_other_message__pack
                     (const MyOtherMessage   *message,
                      uint8_t             *out);
size_t my_other_message__pack_to_buffer
                     (const MyOtherMessage   *message,
                      ProtobufCBuffer     *buffer);
MyOtherMessage *
       my_other_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   my_other_message__free_unpacked
                     (MyOtherMessage *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*MyHeader_Closure)
                 (const MyHeader *message,
                  void *closure_data);
typedef void (*MyOtherMessage_Closure)
                 (const MyOtherMessage *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor my_header__descriptor;
extern const ProtobufCMessageDescriptor my_other_message__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_messagetest_2eproto__INCLUDED */
