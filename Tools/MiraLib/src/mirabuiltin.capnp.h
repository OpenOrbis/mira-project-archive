#ifndef CAPN_EA90F3BD797E05A9
#define CAPN_EA90F3BD797E05A9
/* AUTO GENERATED - DO NOT EDIT */
#include <capnp_c.h>

#if CAPN_VERSION != 1
#error "version mismatch between capnp_c.h and generated code"
#endif

#ifndef capnp_nowarn
# ifdef __GNUC__
#  define capnp_nowarn __extension__
# else
#  define capnp_nowarn
# endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct Message;

typedef struct {capn_ptr p;} Message_ptr;

typedef struct {capn_ptr p;} Message_list;

enum MessageCategory {
	MessageCategory_none = 0,
	MessageCategory_system = 1,
	MessageCategory_log = 2,
	MessageCategory_debug = 3,
	MessageCategory_file = 4,
	MessageCategory_cmd = 5,
	MessageCategory_max = 6
};

struct Message {
	enum MessageCategory category;
	uint32_t type;
	capn_text containedMessage;
};

static const size_t Message_word_count = 1;

static const size_t Message_pointer_count = 1;

static const size_t Message_struct_bytes_count = 16;

Message_ptr new_Message(struct capn_segment*);

Message_list new_Message_list(struct capn_segment*, int len);

void read_Message(struct Message*, Message_ptr);

void write_Message(const struct Message*, Message_ptr);

void get_Message(struct Message*, Message_list, int i);

void set_Message(const struct Message*, Message_list, int i);

#ifdef __cplusplus
}
#endif
#endif
