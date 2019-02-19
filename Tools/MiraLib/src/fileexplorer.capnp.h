#ifndef CAPN_D35134BFE0754755
#define CAPN_D35134BFE0754755
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

struct DirEnt;
struct EchoRequest;
struct EchoResponse;
struct OpenRequest;
struct OpenResponse;
struct CloseRequest;
struct CloseResponse;
struct ReadRequest;
struct ReadResponse;
struct WriteRequest;
struct MkDirRequest;
struct MkDirResponse;
struct RmDirRequest;
struct RmDirResponse;
struct UnlinkRequest;
struct UnlinkResponse;
struct TimeSpec;
struct StatRequest;
struct StatResponse;
struct GetDentsRequest;
struct GetDentsResponse;

typedef struct {capn_ptr p;} DirEnt_ptr;
typedef struct {capn_ptr p;} EchoRequest_ptr;
typedef struct {capn_ptr p;} EchoResponse_ptr;
typedef struct {capn_ptr p;} OpenRequest_ptr;
typedef struct {capn_ptr p;} OpenResponse_ptr;
typedef struct {capn_ptr p;} CloseRequest_ptr;
typedef struct {capn_ptr p;} CloseResponse_ptr;
typedef struct {capn_ptr p;} ReadRequest_ptr;
typedef struct {capn_ptr p;} ReadResponse_ptr;
typedef struct {capn_ptr p;} WriteRequest_ptr;
typedef struct {capn_ptr p;} MkDirRequest_ptr;
typedef struct {capn_ptr p;} MkDirResponse_ptr;
typedef struct {capn_ptr p;} RmDirRequest_ptr;
typedef struct {capn_ptr p;} RmDirResponse_ptr;
typedef struct {capn_ptr p;} UnlinkRequest_ptr;
typedef struct {capn_ptr p;} UnlinkResponse_ptr;
typedef struct {capn_ptr p;} TimeSpec_ptr;
typedef struct {capn_ptr p;} StatRequest_ptr;
typedef struct {capn_ptr p;} StatResponse_ptr;
typedef struct {capn_ptr p;} GetDentsRequest_ptr;
typedef struct {capn_ptr p;} GetDentsResponse_ptr;

typedef struct {capn_ptr p;} DirEnt_list;
typedef struct {capn_ptr p;} EchoRequest_list;
typedef struct {capn_ptr p;} EchoResponse_list;
typedef struct {capn_ptr p;} OpenRequest_list;
typedef struct {capn_ptr p;} OpenResponse_list;
typedef struct {capn_ptr p;} CloseRequest_list;
typedef struct {capn_ptr p;} CloseResponse_list;
typedef struct {capn_ptr p;} ReadRequest_list;
typedef struct {capn_ptr p;} ReadResponse_list;
typedef struct {capn_ptr p;} WriteRequest_list;
typedef struct {capn_ptr p;} MkDirRequest_list;
typedef struct {capn_ptr p;} MkDirResponse_list;
typedef struct {capn_ptr p;} RmDirRequest_list;
typedef struct {capn_ptr p;} RmDirResponse_list;
typedef struct {capn_ptr p;} UnlinkRequest_list;
typedef struct {capn_ptr p;} UnlinkResponse_list;
typedef struct {capn_ptr p;} TimeSpec_list;
typedef struct {capn_ptr p;} StatRequest_list;
typedef struct {capn_ptr p;} StatResponse_list;
typedef struct {capn_ptr p;} GetDentsRequest_list;
typedef struct {capn_ptr p;} GetDentsResponse_list;

enum DirEnt_DirEntType {
	DirEnt_DirEntType_unknown = 0,
	DirEnt_DirEntType_fifo = 1,
	DirEnt_DirEntType_chr = 2,
	DirEnt_DirEntType_invalid0 = 3,
	DirEnt_DirEntType_dir = 4,
	DirEnt_DirEntType_invalid1 = 5,
	DirEnt_DirEntType_blk = 6,
	DirEnt_DirEntType_invalid2 = 7,
	DirEnt_DirEntType_reg = 8,
	DirEnt_DirEntType_invalid3 = 9,
	DirEnt_DirEntType_lnk = 10,
	DirEnt_DirEntType_invalid4 = 11,
	DirEnt_DirEntType_sock = 12,
	DirEnt_DirEntType_invalid5 = 13,
	DirEnt_DirEntType_wht = 14
};

struct DirEnt {
	uint32_t fileno;
	uint32_t reclen;
	enum DirEnt_DirEntType type;
	capn_text name;
};

static const size_t DirEnt_word_count = 2;

static const size_t DirEnt_pointer_count = 1;

static const size_t DirEnt_struct_bytes_count = 24;

struct EchoRequest {
	capn_text message;
};

static const size_t EchoRequest_word_count = 0;

static const size_t EchoRequest_pointer_count = 1;

static const size_t EchoRequest_struct_bytes_count = 8;

struct EchoResponse {
	int32_t error;
};

static const size_t EchoResponse_word_count = 1;

static const size_t EchoResponse_pointer_count = 0;

static const size_t EchoResponse_struct_bytes_count = 8;

struct OpenRequest {
	capn_text path;
	int32_t flags;
	int32_t mode;
};

static const size_t OpenRequest_word_count = 1;

static const size_t OpenRequest_pointer_count = 1;

static const size_t OpenRequest_struct_bytes_count = 16;

struct OpenResponse {
	int32_t error;
	int32_t handle;
};

static const size_t OpenResponse_word_count = 1;

static const size_t OpenResponse_pointer_count = 0;

static const size_t OpenResponse_struct_bytes_count = 8;

struct CloseRequest {
	int32_t handle;
};

static const size_t CloseRequest_word_count = 1;

static const size_t CloseRequest_pointer_count = 0;

static const size_t CloseRequest_struct_bytes_count = 8;

struct CloseResponse {
	int32_t error;
};

static const size_t CloseResponse_word_count = 1;

static const size_t CloseResponse_pointer_count = 0;

static const size_t CloseResponse_struct_bytes_count = 8;

struct ReadRequest {
	int32_t handle;
	uint32_t offset;
	uint32_t size;
};

static const size_t ReadRequest_word_count = 2;

static const size_t ReadRequest_pointer_count = 0;

static const size_t ReadRequest_struct_bytes_count = 16;

struct ReadResponse {
	int32_t error;
	capn_data bytes;
};

static const size_t ReadResponse_word_count = 1;

static const size_t ReadResponse_pointer_count = 1;

static const size_t ReadResponse_struct_bytes_count = 16;

struct WriteRequest {
	int32_t handle;
	uint32_t offset;
	capn_data bytes;
};

static const size_t WriteRequest_word_count = 1;

static const size_t WriteRequest_pointer_count = 1;

static const size_t WriteRequest_struct_bytes_count = 16;

struct MkDirRequest {
	capn_text path;
	int32_t mode;
};

static const size_t MkDirRequest_word_count = 1;

static const size_t MkDirRequest_pointer_count = 1;

static const size_t MkDirRequest_struct_bytes_count = 16;

struct MkDirResponse {
	int32_t error;
};

static const size_t MkDirResponse_word_count = 1;

static const size_t MkDirResponse_pointer_count = 0;

static const size_t MkDirResponse_struct_bytes_count = 8;

struct RmDirRequest {
	capn_text path;
	uint8_t recursive;
};

static const size_t RmDirRequest_word_count = 1;

static const size_t RmDirRequest_pointer_count = 1;

static const size_t RmDirRequest_struct_bytes_count = 16;

struct RmDirResponse {
	int32_t error;
};

static const size_t RmDirResponse_word_count = 1;

static const size_t RmDirResponse_pointer_count = 0;

static const size_t RmDirResponse_struct_bytes_count = 8;

struct UnlinkRequest {
	capn_text path;
};

static const size_t UnlinkRequest_word_count = 0;

static const size_t UnlinkRequest_pointer_count = 1;

static const size_t UnlinkRequest_struct_bytes_count = 8;

struct UnlinkResponse {
	int32_t error;
};

static const size_t UnlinkResponse_word_count = 1;

static const size_t UnlinkResponse_pointer_count = 0;

static const size_t UnlinkResponse_struct_bytes_count = 8;

struct TimeSpec {
	int64_t sec;
	uint64_t nsec;
};

static const size_t TimeSpec_word_count = 2;

static const size_t TimeSpec_pointer_count = 0;

static const size_t TimeSpec_struct_bytes_count = 16;

struct StatRequest {
	int32_t handle;
	capn_text path;
};

static const size_t StatRequest_word_count = 1;

static const size_t StatRequest_pointer_count = 1;

static const size_t StatRequest_struct_bytes_count = 16;

struct StatResponse {
	int32_t error;
	capn_text path;
	uint32_t dev;
	uint32_t ino;
	uint32_t mode;
	uint32_t nlink;
	uint32_t uid;
	uint32_t gid;
	uint32_t rdev;
	TimeSpec_ptr atim;
	TimeSpec_ptr mtim;
	TimeSpec_ptr ctim;
	int64_t size;
	int64_t blocks;
	uint32_t blksize;
	uint32_t flags;
	uint32_t gen;
	int32_t lspare;
	TimeSpec_ptr birthtim;
};

static const size_t StatResponse_word_count = 8;

static const size_t StatResponse_pointer_count = 5;

static const size_t StatResponse_struct_bytes_count = 104;

struct GetDentsRequest {
	capn_text path;
};

static const size_t GetDentsRequest_word_count = 0;

static const size_t GetDentsRequest_pointer_count = 1;

static const size_t GetDentsRequest_struct_bytes_count = 8;

struct GetDentsResponse {
	int32_t error;
	DirEnt_list entries;
};

static const size_t GetDentsResponse_word_count = 1;

static const size_t GetDentsResponse_pointer_count = 1;

static const size_t GetDentsResponse_struct_bytes_count = 16;

DirEnt_ptr new_DirEnt(struct capn_segment*);
EchoRequest_ptr new_EchoRequest(struct capn_segment*);
EchoResponse_ptr new_EchoResponse(struct capn_segment*);
OpenRequest_ptr new_OpenRequest(struct capn_segment*);
OpenResponse_ptr new_OpenResponse(struct capn_segment*);
CloseRequest_ptr new_CloseRequest(struct capn_segment*);
CloseResponse_ptr new_CloseResponse(struct capn_segment*);
ReadRequest_ptr new_ReadRequest(struct capn_segment*);
ReadResponse_ptr new_ReadResponse(struct capn_segment*);
WriteRequest_ptr new_WriteRequest(struct capn_segment*);
MkDirRequest_ptr new_MkDirRequest(struct capn_segment*);
MkDirResponse_ptr new_MkDirResponse(struct capn_segment*);
RmDirRequest_ptr new_RmDirRequest(struct capn_segment*);
RmDirResponse_ptr new_RmDirResponse(struct capn_segment*);
UnlinkRequest_ptr new_UnlinkRequest(struct capn_segment*);
UnlinkResponse_ptr new_UnlinkResponse(struct capn_segment*);
TimeSpec_ptr new_TimeSpec(struct capn_segment*);
StatRequest_ptr new_StatRequest(struct capn_segment*);
StatResponse_ptr new_StatResponse(struct capn_segment*);
GetDentsRequest_ptr new_GetDentsRequest(struct capn_segment*);
GetDentsResponse_ptr new_GetDentsResponse(struct capn_segment*);

DirEnt_list new_DirEnt_list(struct capn_segment*, int len);
EchoRequest_list new_EchoRequest_list(struct capn_segment*, int len);
EchoResponse_list new_EchoResponse_list(struct capn_segment*, int len);
OpenRequest_list new_OpenRequest_list(struct capn_segment*, int len);
OpenResponse_list new_OpenResponse_list(struct capn_segment*, int len);
CloseRequest_list new_CloseRequest_list(struct capn_segment*, int len);
CloseResponse_list new_CloseResponse_list(struct capn_segment*, int len);
ReadRequest_list new_ReadRequest_list(struct capn_segment*, int len);
ReadResponse_list new_ReadResponse_list(struct capn_segment*, int len);
WriteRequest_list new_WriteRequest_list(struct capn_segment*, int len);
MkDirRequest_list new_MkDirRequest_list(struct capn_segment*, int len);
MkDirResponse_list new_MkDirResponse_list(struct capn_segment*, int len);
RmDirRequest_list new_RmDirRequest_list(struct capn_segment*, int len);
RmDirResponse_list new_RmDirResponse_list(struct capn_segment*, int len);
UnlinkRequest_list new_UnlinkRequest_list(struct capn_segment*, int len);
UnlinkResponse_list new_UnlinkResponse_list(struct capn_segment*, int len);
TimeSpec_list new_TimeSpec_list(struct capn_segment*, int len);
StatRequest_list new_StatRequest_list(struct capn_segment*, int len);
StatResponse_list new_StatResponse_list(struct capn_segment*, int len);
GetDentsRequest_list new_GetDentsRequest_list(struct capn_segment*, int len);
GetDentsResponse_list new_GetDentsResponse_list(struct capn_segment*, int len);

void read_DirEnt(struct DirEnt*, DirEnt_ptr);
void read_EchoRequest(struct EchoRequest*, EchoRequest_ptr);
void read_EchoResponse(struct EchoResponse*, EchoResponse_ptr);
void read_OpenRequest(struct OpenRequest*, OpenRequest_ptr);
void read_OpenResponse(struct OpenResponse*, OpenResponse_ptr);
void read_CloseRequest(struct CloseRequest*, CloseRequest_ptr);
void read_CloseResponse(struct CloseResponse*, CloseResponse_ptr);
void read_ReadRequest(struct ReadRequest*, ReadRequest_ptr);
void read_ReadResponse(struct ReadResponse*, ReadResponse_ptr);
void read_WriteRequest(struct WriteRequest*, WriteRequest_ptr);
void read_MkDirRequest(struct MkDirRequest*, MkDirRequest_ptr);
void read_MkDirResponse(struct MkDirResponse*, MkDirResponse_ptr);
void read_RmDirRequest(struct RmDirRequest*, RmDirRequest_ptr);
void read_RmDirResponse(struct RmDirResponse*, RmDirResponse_ptr);
void read_UnlinkRequest(struct UnlinkRequest*, UnlinkRequest_ptr);
void read_UnlinkResponse(struct UnlinkResponse*, UnlinkResponse_ptr);
void read_TimeSpec(struct TimeSpec*, TimeSpec_ptr);
void read_StatRequest(struct StatRequest*, StatRequest_ptr);
void read_StatResponse(struct StatResponse*, StatResponse_ptr);
void read_GetDentsRequest(struct GetDentsRequest*, GetDentsRequest_ptr);
void read_GetDentsResponse(struct GetDentsResponse*, GetDentsResponse_ptr);

void write_DirEnt(const struct DirEnt*, DirEnt_ptr);
void write_EchoRequest(const struct EchoRequest*, EchoRequest_ptr);
void write_EchoResponse(const struct EchoResponse*, EchoResponse_ptr);
void write_OpenRequest(const struct OpenRequest*, OpenRequest_ptr);
void write_OpenResponse(const struct OpenResponse*, OpenResponse_ptr);
void write_CloseRequest(const struct CloseRequest*, CloseRequest_ptr);
void write_CloseResponse(const struct CloseResponse*, CloseResponse_ptr);
void write_ReadRequest(const struct ReadRequest*, ReadRequest_ptr);
void write_ReadResponse(const struct ReadResponse*, ReadResponse_ptr);
void write_WriteRequest(const struct WriteRequest*, WriteRequest_ptr);
void write_MkDirRequest(const struct MkDirRequest*, MkDirRequest_ptr);
void write_MkDirResponse(const struct MkDirResponse*, MkDirResponse_ptr);
void write_RmDirRequest(const struct RmDirRequest*, RmDirRequest_ptr);
void write_RmDirResponse(const struct RmDirResponse*, RmDirResponse_ptr);
void write_UnlinkRequest(const struct UnlinkRequest*, UnlinkRequest_ptr);
void write_UnlinkResponse(const struct UnlinkResponse*, UnlinkResponse_ptr);
void write_TimeSpec(const struct TimeSpec*, TimeSpec_ptr);
void write_StatRequest(const struct StatRequest*, StatRequest_ptr);
void write_StatResponse(const struct StatResponse*, StatResponse_ptr);
void write_GetDentsRequest(const struct GetDentsRequest*, GetDentsRequest_ptr);
void write_GetDentsResponse(const struct GetDentsResponse*, GetDentsResponse_ptr);

void get_DirEnt(struct DirEnt*, DirEnt_list, int i);
void get_EchoRequest(struct EchoRequest*, EchoRequest_list, int i);
void get_EchoResponse(struct EchoResponse*, EchoResponse_list, int i);
void get_OpenRequest(struct OpenRequest*, OpenRequest_list, int i);
void get_OpenResponse(struct OpenResponse*, OpenResponse_list, int i);
void get_CloseRequest(struct CloseRequest*, CloseRequest_list, int i);
void get_CloseResponse(struct CloseResponse*, CloseResponse_list, int i);
void get_ReadRequest(struct ReadRequest*, ReadRequest_list, int i);
void get_ReadResponse(struct ReadResponse*, ReadResponse_list, int i);
void get_WriteRequest(struct WriteRequest*, WriteRequest_list, int i);
void get_MkDirRequest(struct MkDirRequest*, MkDirRequest_list, int i);
void get_MkDirResponse(struct MkDirResponse*, MkDirResponse_list, int i);
void get_RmDirRequest(struct RmDirRequest*, RmDirRequest_list, int i);
void get_RmDirResponse(struct RmDirResponse*, RmDirResponse_list, int i);
void get_UnlinkRequest(struct UnlinkRequest*, UnlinkRequest_list, int i);
void get_UnlinkResponse(struct UnlinkResponse*, UnlinkResponse_list, int i);
void get_TimeSpec(struct TimeSpec*, TimeSpec_list, int i);
void get_StatRequest(struct StatRequest*, StatRequest_list, int i);
void get_StatResponse(struct StatResponse*, StatResponse_list, int i);
void get_GetDentsRequest(struct GetDentsRequest*, GetDentsRequest_list, int i);
void get_GetDentsResponse(struct GetDentsResponse*, GetDentsResponse_list, int i);

void set_DirEnt(const struct DirEnt*, DirEnt_list, int i);
void set_EchoRequest(const struct EchoRequest*, EchoRequest_list, int i);
void set_EchoResponse(const struct EchoResponse*, EchoResponse_list, int i);
void set_OpenRequest(const struct OpenRequest*, OpenRequest_list, int i);
void set_OpenResponse(const struct OpenResponse*, OpenResponse_list, int i);
void set_CloseRequest(const struct CloseRequest*, CloseRequest_list, int i);
void set_CloseResponse(const struct CloseResponse*, CloseResponse_list, int i);
void set_ReadRequest(const struct ReadRequest*, ReadRequest_list, int i);
void set_ReadResponse(const struct ReadResponse*, ReadResponse_list, int i);
void set_WriteRequest(const struct WriteRequest*, WriteRequest_list, int i);
void set_MkDirRequest(const struct MkDirRequest*, MkDirRequest_list, int i);
void set_MkDirResponse(const struct MkDirResponse*, MkDirResponse_list, int i);
void set_RmDirRequest(const struct RmDirRequest*, RmDirRequest_list, int i);
void set_RmDirResponse(const struct RmDirResponse*, RmDirResponse_list, int i);
void set_UnlinkRequest(const struct UnlinkRequest*, UnlinkRequest_list, int i);
void set_UnlinkResponse(const struct UnlinkResponse*, UnlinkResponse_list, int i);
void set_TimeSpec(const struct TimeSpec*, TimeSpec_list, int i);
void set_StatRequest(const struct StatRequest*, StatRequest_list, int i);
void set_StatResponse(const struct StatResponse*, StatResponse_list, int i);
void set_GetDentsRequest(const struct GetDentsRequest*, GetDentsRequest_list, int i);
void set_GetDentsResponse(const struct GetDentsResponse*, GetDentsResponse_list, int i);

#ifdef __cplusplus
}
#endif
#endif
