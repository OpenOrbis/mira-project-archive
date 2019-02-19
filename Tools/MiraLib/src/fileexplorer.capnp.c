#include "fileexplorer.capnp.h"
/* AUTO GENERATED - DO NOT EDIT */
#ifdef __GNUC__
# define capnp_unused __attribute__((unused))
# define capnp_use(x) (void) x;
#else
# define capnp_unused
# define capnp_use(x)
#endif

static const capn_text capn_val0 = {0,"",0};

DirEnt_ptr new_DirEnt(struct capn_segment *s) {
	DirEnt_ptr p;
	p.p = capn_new_struct(s, 16, 1);
	return p;
}
DirEnt_list new_DirEnt_list(struct capn_segment *s, int len) {
	DirEnt_list p;
	p.p = capn_new_list(s, len, 16, 1);
	return p;
}
void read_DirEnt(struct DirEnt *s capnp_unused, DirEnt_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->fileno = capn_read32(p.p, 0);
	s->reclen = capn_read32(p.p, 4);
	s->type = (enum DirEnt_DirEntType)(int) capn_read16(p.p, 8);
	s->name = capn_get_text(p.p, 0, capn_val0);
}
void write_DirEnt(const struct DirEnt *s capnp_unused, DirEnt_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, s->fileno);
	capn_write32(p.p, 4, s->reclen);
	capn_write16(p.p, 8, (uint16_t) (s->type));
	capn_set_text(p.p, 0, s->name);
}
void get_DirEnt(struct DirEnt *s, DirEnt_list l, int i) {
	DirEnt_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_DirEnt(s, p);
}
void set_DirEnt(const struct DirEnt *s, DirEnt_list l, int i) {
	DirEnt_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_DirEnt(s, p);
}

EchoRequest_ptr new_EchoRequest(struct capn_segment *s) {
	EchoRequest_ptr p;
	p.p = capn_new_struct(s, 0, 1);
	return p;
}
EchoRequest_list new_EchoRequest_list(struct capn_segment *s, int len) {
	EchoRequest_list p;
	p.p = capn_new_list(s, len, 0, 1);
	return p;
}
void read_EchoRequest(struct EchoRequest *s capnp_unused, EchoRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->message = capn_get_text(p.p, 0, capn_val0);
}
void write_EchoRequest(const struct EchoRequest *s capnp_unused, EchoRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_set_text(p.p, 0, s->message);
}
void get_EchoRequest(struct EchoRequest *s, EchoRequest_list l, int i) {
	EchoRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_EchoRequest(s, p);
}
void set_EchoRequest(const struct EchoRequest *s, EchoRequest_list l, int i) {
	EchoRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_EchoRequest(s, p);
}

EchoResponse_ptr new_EchoResponse(struct capn_segment *s) {
	EchoResponse_ptr p;
	p.p = capn_new_struct(s, 8, 0);
	return p;
}
EchoResponse_list new_EchoResponse_list(struct capn_segment *s, int len) {
	EchoResponse_list p;
	p.p = capn_new_list(s, len, 8, 0);
	return p;
}
void read_EchoResponse(struct EchoResponse *s capnp_unused, EchoResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->error = (int32_t) ((int32_t)capn_read32(p.p, 0));
}
void write_EchoResponse(const struct EchoResponse *s capnp_unused, EchoResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->error));
}
void get_EchoResponse(struct EchoResponse *s, EchoResponse_list l, int i) {
	EchoResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_EchoResponse(s, p);
}
void set_EchoResponse(const struct EchoResponse *s, EchoResponse_list l, int i) {
	EchoResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_EchoResponse(s, p);
}

OpenRequest_ptr new_OpenRequest(struct capn_segment *s) {
	OpenRequest_ptr p;
	p.p = capn_new_struct(s, 8, 1);
	return p;
}
OpenRequest_list new_OpenRequest_list(struct capn_segment *s, int len) {
	OpenRequest_list p;
	p.p = capn_new_list(s, len, 8, 1);
	return p;
}
void read_OpenRequest(struct OpenRequest *s capnp_unused, OpenRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->path = capn_get_text(p.p, 0, capn_val0);
	s->flags = (int32_t) ((int32_t)capn_read32(p.p, 0));
	s->mode = (int32_t) ((int32_t)capn_read32(p.p, 4));
}
void write_OpenRequest(const struct OpenRequest *s capnp_unused, OpenRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_set_text(p.p, 0, s->path);
	capn_write32(p.p, 0, (uint32_t) (s->flags));
	capn_write32(p.p, 4, (uint32_t) (s->mode));
}
void get_OpenRequest(struct OpenRequest *s, OpenRequest_list l, int i) {
	OpenRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_OpenRequest(s, p);
}
void set_OpenRequest(const struct OpenRequest *s, OpenRequest_list l, int i) {
	OpenRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_OpenRequest(s, p);
}

OpenResponse_ptr new_OpenResponse(struct capn_segment *s) {
	OpenResponse_ptr p;
	p.p = capn_new_struct(s, 8, 0);
	return p;
}
OpenResponse_list new_OpenResponse_list(struct capn_segment *s, int len) {
	OpenResponse_list p;
	p.p = capn_new_list(s, len, 8, 0);
	return p;
}
void read_OpenResponse(struct OpenResponse *s capnp_unused, OpenResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->error = (int32_t) ((int32_t)capn_read32(p.p, 0));
	s->handle = (int32_t) ((int32_t)capn_read32(p.p, 4));
}
void write_OpenResponse(const struct OpenResponse *s capnp_unused, OpenResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->error));
	capn_write32(p.p, 4, (uint32_t) (s->handle));
}
void get_OpenResponse(struct OpenResponse *s, OpenResponse_list l, int i) {
	OpenResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_OpenResponse(s, p);
}
void set_OpenResponse(const struct OpenResponse *s, OpenResponse_list l, int i) {
	OpenResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_OpenResponse(s, p);
}

CloseRequest_ptr new_CloseRequest(struct capn_segment *s) {
	CloseRequest_ptr p;
	p.p = capn_new_struct(s, 8, 0);
	return p;
}
CloseRequest_list new_CloseRequest_list(struct capn_segment *s, int len) {
	CloseRequest_list p;
	p.p = capn_new_list(s, len, 8, 0);
	return p;
}
void read_CloseRequest(struct CloseRequest *s capnp_unused, CloseRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->handle = (int32_t) ((int32_t)capn_read32(p.p, 0));
}
void write_CloseRequest(const struct CloseRequest *s capnp_unused, CloseRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->handle));
}
void get_CloseRequest(struct CloseRequest *s, CloseRequest_list l, int i) {
	CloseRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_CloseRequest(s, p);
}
void set_CloseRequest(const struct CloseRequest *s, CloseRequest_list l, int i) {
	CloseRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_CloseRequest(s, p);
}

CloseResponse_ptr new_CloseResponse(struct capn_segment *s) {
	CloseResponse_ptr p;
	p.p = capn_new_struct(s, 8, 0);
	return p;
}
CloseResponse_list new_CloseResponse_list(struct capn_segment *s, int len) {
	CloseResponse_list p;
	p.p = capn_new_list(s, len, 8, 0);
	return p;
}
void read_CloseResponse(struct CloseResponse *s capnp_unused, CloseResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->error = (int32_t) ((int32_t)capn_read32(p.p, 0));
}
void write_CloseResponse(const struct CloseResponse *s capnp_unused, CloseResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->error));
}
void get_CloseResponse(struct CloseResponse *s, CloseResponse_list l, int i) {
	CloseResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_CloseResponse(s, p);
}
void set_CloseResponse(const struct CloseResponse *s, CloseResponse_list l, int i) {
	CloseResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_CloseResponse(s, p);
}

ReadRequest_ptr new_ReadRequest(struct capn_segment *s) {
	ReadRequest_ptr p;
	p.p = capn_new_struct(s, 16, 0);
	return p;
}
ReadRequest_list new_ReadRequest_list(struct capn_segment *s, int len) {
	ReadRequest_list p;
	p.p = capn_new_list(s, len, 16, 0);
	return p;
}
void read_ReadRequest(struct ReadRequest *s capnp_unused, ReadRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->handle = (int32_t) ((int32_t)capn_read32(p.p, 0));
	s->offset = capn_read32(p.p, 4);
	s->size = capn_read32(p.p, 8);
}
void write_ReadRequest(const struct ReadRequest *s capnp_unused, ReadRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->handle));
	capn_write32(p.p, 4, s->offset);
	capn_write32(p.p, 8, s->size);
}
void get_ReadRequest(struct ReadRequest *s, ReadRequest_list l, int i) {
	ReadRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_ReadRequest(s, p);
}
void set_ReadRequest(const struct ReadRequest *s, ReadRequest_list l, int i) {
	ReadRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_ReadRequest(s, p);
}

ReadResponse_ptr new_ReadResponse(struct capn_segment *s) {
	ReadResponse_ptr p;
	p.p = capn_new_struct(s, 8, 1);
	return p;
}
ReadResponse_list new_ReadResponse_list(struct capn_segment *s, int len) {
	ReadResponse_list p;
	p.p = capn_new_list(s, len, 8, 1);
	return p;
}
void read_ReadResponse(struct ReadResponse *s capnp_unused, ReadResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->error = (int32_t) ((int32_t)capn_read32(p.p, 0));
	s->bytes = capn_get_data(p.p, 0);
}
void write_ReadResponse(const struct ReadResponse *s capnp_unused, ReadResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->error));
	capn_setp(p.p, 0, s->bytes.p);
}
void get_ReadResponse(struct ReadResponse *s, ReadResponse_list l, int i) {
	ReadResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_ReadResponse(s, p);
}
void set_ReadResponse(const struct ReadResponse *s, ReadResponse_list l, int i) {
	ReadResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_ReadResponse(s, p);
}

WriteRequest_ptr new_WriteRequest(struct capn_segment *s) {
	WriteRequest_ptr p;
	p.p = capn_new_struct(s, 8, 1);
	return p;
}
WriteRequest_list new_WriteRequest_list(struct capn_segment *s, int len) {
	WriteRequest_list p;
	p.p = capn_new_list(s, len, 8, 1);
	return p;
}
void read_WriteRequest(struct WriteRequest *s capnp_unused, WriteRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->handle = (int32_t) ((int32_t)capn_read32(p.p, 0));
	s->offset = capn_read32(p.p, 4);
	s->bytes = capn_get_data(p.p, 0);
}
void write_WriteRequest(const struct WriteRequest *s capnp_unused, WriteRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->handle));
	capn_write32(p.p, 4, s->offset);
	capn_setp(p.p, 0, s->bytes.p);
}
void get_WriteRequest(struct WriteRequest *s, WriteRequest_list l, int i) {
	WriteRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_WriteRequest(s, p);
}
void set_WriteRequest(const struct WriteRequest *s, WriteRequest_list l, int i) {
	WriteRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_WriteRequest(s, p);
}

MkDirRequest_ptr new_MkDirRequest(struct capn_segment *s) {
	MkDirRequest_ptr p;
	p.p = capn_new_struct(s, 8, 1);
	return p;
}
MkDirRequest_list new_MkDirRequest_list(struct capn_segment *s, int len) {
	MkDirRequest_list p;
	p.p = capn_new_list(s, len, 8, 1);
	return p;
}
void read_MkDirRequest(struct MkDirRequest *s capnp_unused, MkDirRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->path = capn_get_text(p.p, 0, capn_val0);
	s->mode = (int32_t) ((int32_t)capn_read32(p.p, 0));
}
void write_MkDirRequest(const struct MkDirRequest *s capnp_unused, MkDirRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_set_text(p.p, 0, s->path);
	capn_write32(p.p, 0, (uint32_t) (s->mode));
}
void get_MkDirRequest(struct MkDirRequest *s, MkDirRequest_list l, int i) {
	MkDirRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_MkDirRequest(s, p);
}
void set_MkDirRequest(const struct MkDirRequest *s, MkDirRequest_list l, int i) {
	MkDirRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_MkDirRequest(s, p);
}

MkDirResponse_ptr new_MkDirResponse(struct capn_segment *s) {
	MkDirResponse_ptr p;
	p.p = capn_new_struct(s, 8, 0);
	return p;
}
MkDirResponse_list new_MkDirResponse_list(struct capn_segment *s, int len) {
	MkDirResponse_list p;
	p.p = capn_new_list(s, len, 8, 0);
	return p;
}
void read_MkDirResponse(struct MkDirResponse *s capnp_unused, MkDirResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->error = (int32_t) ((int32_t)capn_read32(p.p, 0));
}
void write_MkDirResponse(const struct MkDirResponse *s capnp_unused, MkDirResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->error));
}
void get_MkDirResponse(struct MkDirResponse *s, MkDirResponse_list l, int i) {
	MkDirResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_MkDirResponse(s, p);
}
void set_MkDirResponse(const struct MkDirResponse *s, MkDirResponse_list l, int i) {
	MkDirResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_MkDirResponse(s, p);
}

RmDirRequest_ptr new_RmDirRequest(struct capn_segment *s) {
	RmDirRequest_ptr p;
	p.p = capn_new_struct(s, 8, 1);
	return p;
}
RmDirRequest_list new_RmDirRequest_list(struct capn_segment *s, int len) {
	RmDirRequest_list p;
	p.p = capn_new_list(s, len, 8, 1);
	return p;
}
void read_RmDirRequest(struct RmDirRequest *s capnp_unused, RmDirRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->path = capn_get_text(p.p, 0, capn_val0);
	s->recursive = capn_read8(p.p, 0);
}
void write_RmDirRequest(const struct RmDirRequest *s capnp_unused, RmDirRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_set_text(p.p, 0, s->path);
	capn_write8(p.p, 0, s->recursive);
}
void get_RmDirRequest(struct RmDirRequest *s, RmDirRequest_list l, int i) {
	RmDirRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_RmDirRequest(s, p);
}
void set_RmDirRequest(const struct RmDirRequest *s, RmDirRequest_list l, int i) {
	RmDirRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_RmDirRequest(s, p);
}

RmDirResponse_ptr new_RmDirResponse(struct capn_segment *s) {
	RmDirResponse_ptr p;
	p.p = capn_new_struct(s, 8, 0);
	return p;
}
RmDirResponse_list new_RmDirResponse_list(struct capn_segment *s, int len) {
	RmDirResponse_list p;
	p.p = capn_new_list(s, len, 8, 0);
	return p;
}
void read_RmDirResponse(struct RmDirResponse *s capnp_unused, RmDirResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->error = (int32_t) ((int32_t)capn_read32(p.p, 0));
}
void write_RmDirResponse(const struct RmDirResponse *s capnp_unused, RmDirResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->error));
}
void get_RmDirResponse(struct RmDirResponse *s, RmDirResponse_list l, int i) {
	RmDirResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_RmDirResponse(s, p);
}
void set_RmDirResponse(const struct RmDirResponse *s, RmDirResponse_list l, int i) {
	RmDirResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_RmDirResponse(s, p);
}

UnlinkRequest_ptr new_UnlinkRequest(struct capn_segment *s) {
	UnlinkRequest_ptr p;
	p.p = capn_new_struct(s, 0, 1);
	return p;
}
UnlinkRequest_list new_UnlinkRequest_list(struct capn_segment *s, int len) {
	UnlinkRequest_list p;
	p.p = capn_new_list(s, len, 0, 1);
	return p;
}
void read_UnlinkRequest(struct UnlinkRequest *s capnp_unused, UnlinkRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->path = capn_get_text(p.p, 0, capn_val0);
}
void write_UnlinkRequest(const struct UnlinkRequest *s capnp_unused, UnlinkRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_set_text(p.p, 0, s->path);
}
void get_UnlinkRequest(struct UnlinkRequest *s, UnlinkRequest_list l, int i) {
	UnlinkRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_UnlinkRequest(s, p);
}
void set_UnlinkRequest(const struct UnlinkRequest *s, UnlinkRequest_list l, int i) {
	UnlinkRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_UnlinkRequest(s, p);
}

UnlinkResponse_ptr new_UnlinkResponse(struct capn_segment *s) {
	UnlinkResponse_ptr p;
	p.p = capn_new_struct(s, 8, 0);
	return p;
}
UnlinkResponse_list new_UnlinkResponse_list(struct capn_segment *s, int len) {
	UnlinkResponse_list p;
	p.p = capn_new_list(s, len, 8, 0);
	return p;
}
void read_UnlinkResponse(struct UnlinkResponse *s capnp_unused, UnlinkResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->error = (int32_t) ((int32_t)capn_read32(p.p, 0));
}
void write_UnlinkResponse(const struct UnlinkResponse *s capnp_unused, UnlinkResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->error));
}
void get_UnlinkResponse(struct UnlinkResponse *s, UnlinkResponse_list l, int i) {
	UnlinkResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_UnlinkResponse(s, p);
}
void set_UnlinkResponse(const struct UnlinkResponse *s, UnlinkResponse_list l, int i) {
	UnlinkResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_UnlinkResponse(s, p);
}

TimeSpec_ptr new_TimeSpec(struct capn_segment *s) {
	TimeSpec_ptr p;
	p.p = capn_new_struct(s, 16, 0);
	return p;
}
TimeSpec_list new_TimeSpec_list(struct capn_segment *s, int len) {
	TimeSpec_list p;
	p.p = capn_new_list(s, len, 16, 0);
	return p;
}
void read_TimeSpec(struct TimeSpec *s capnp_unused, TimeSpec_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->sec = (int64_t) ((int64_t)(capn_read64(p.p, 0)));
	s->nsec = capn_read64(p.p, 8);
}
void write_TimeSpec(const struct TimeSpec *s capnp_unused, TimeSpec_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write64(p.p, 0, (uint64_t) (s->sec));
	capn_write64(p.p, 8, s->nsec);
}
void get_TimeSpec(struct TimeSpec *s, TimeSpec_list l, int i) {
	TimeSpec_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_TimeSpec(s, p);
}
void set_TimeSpec(const struct TimeSpec *s, TimeSpec_list l, int i) {
	TimeSpec_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_TimeSpec(s, p);
}

StatRequest_ptr new_StatRequest(struct capn_segment *s) {
	StatRequest_ptr p;
	p.p = capn_new_struct(s, 8, 1);
	return p;
}
StatRequest_list new_StatRequest_list(struct capn_segment *s, int len) {
	StatRequest_list p;
	p.p = capn_new_list(s, len, 8, 1);
	return p;
}
void read_StatRequest(struct StatRequest *s capnp_unused, StatRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->handle = (int32_t) ((int32_t)capn_read32(p.p, 0)) ^ -1;
	s->path = capn_get_text(p.p, 0, capn_val0);
}
void write_StatRequest(const struct StatRequest *s capnp_unused, StatRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->handle ^ -1));
	capn_set_text(p.p, 0, s->path);
}
void get_StatRequest(struct StatRequest *s, StatRequest_list l, int i) {
	StatRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_StatRequest(s, p);
}
void set_StatRequest(const struct StatRequest *s, StatRequest_list l, int i) {
	StatRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_StatRequest(s, p);
}

StatResponse_ptr new_StatResponse(struct capn_segment *s) {
	StatResponse_ptr p;
	p.p = capn_new_struct(s, 64, 5);
	return p;
}
StatResponse_list new_StatResponse_list(struct capn_segment *s, int len) {
	StatResponse_list p;
	p.p = capn_new_list(s, len, 64, 5);
	return p;
}
void read_StatResponse(struct StatResponse *s capnp_unused, StatResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->error = (int32_t) ((int32_t)capn_read32(p.p, 0));
	s->path = capn_get_text(p.p, 0, capn_val0);
	s->dev = capn_read32(p.p, 4);
	s->ino = capn_read32(p.p, 8);
	s->mode = capn_read32(p.p, 12);
	s->nlink = capn_read32(p.p, 16);
	s->uid = capn_read32(p.p, 20);
	s->gid = capn_read32(p.p, 24);
	s->rdev = capn_read32(p.p, 28);
	s->atim.p = capn_getp(p.p, 1, 0);
	s->mtim.p = capn_getp(p.p, 2, 0);
	s->ctim.p = capn_getp(p.p, 3, 0);
	s->size = (int64_t) ((int64_t)(capn_read64(p.p, 32)));
	s->blocks = (int64_t) ((int64_t)(capn_read64(p.p, 40)));
	s->blksize = capn_read32(p.p, 48);
	s->flags = capn_read32(p.p, 52);
	s->gen = capn_read32(p.p, 56);
	s->lspare = (int32_t) ((int32_t)capn_read32(p.p, 60));
	s->birthtim.p = capn_getp(p.p, 4, 0);
}
void write_StatResponse(const struct StatResponse *s capnp_unused, StatResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->error));
	capn_set_text(p.p, 0, s->path);
	capn_write32(p.p, 4, s->dev);
	capn_write32(p.p, 8, s->ino);
	capn_write32(p.p, 12, s->mode);
	capn_write32(p.p, 16, s->nlink);
	capn_write32(p.p, 20, s->uid);
	capn_write32(p.p, 24, s->gid);
	capn_write32(p.p, 28, s->rdev);
	capn_setp(p.p, 1, s->atim.p);
	capn_setp(p.p, 2, s->mtim.p);
	capn_setp(p.p, 3, s->ctim.p);
	capn_write64(p.p, 32, (uint64_t) (s->size));
	capn_write64(p.p, 40, (uint64_t) (s->blocks));
	capn_write32(p.p, 48, s->blksize);
	capn_write32(p.p, 52, s->flags);
	capn_write32(p.p, 56, s->gen);
	capn_write32(p.p, 60, (uint32_t) (s->lspare));
	capn_setp(p.p, 4, s->birthtim.p);
}
void get_StatResponse(struct StatResponse *s, StatResponse_list l, int i) {
	StatResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_StatResponse(s, p);
}
void set_StatResponse(const struct StatResponse *s, StatResponse_list l, int i) {
	StatResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_StatResponse(s, p);
}

GetDentsRequest_ptr new_GetDentsRequest(struct capn_segment *s) {
	GetDentsRequest_ptr p;
	p.p = capn_new_struct(s, 0, 1);
	return p;
}
GetDentsRequest_list new_GetDentsRequest_list(struct capn_segment *s, int len) {
	GetDentsRequest_list p;
	p.p = capn_new_list(s, len, 0, 1);
	return p;
}
void read_GetDentsRequest(struct GetDentsRequest *s capnp_unused, GetDentsRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->path = capn_get_text(p.p, 0, capn_val0);
}
void write_GetDentsRequest(const struct GetDentsRequest *s capnp_unused, GetDentsRequest_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_set_text(p.p, 0, s->path);
}
void get_GetDentsRequest(struct GetDentsRequest *s, GetDentsRequest_list l, int i) {
	GetDentsRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_GetDentsRequest(s, p);
}
void set_GetDentsRequest(const struct GetDentsRequest *s, GetDentsRequest_list l, int i) {
	GetDentsRequest_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_GetDentsRequest(s, p);
}

GetDentsResponse_ptr new_GetDentsResponse(struct capn_segment *s) {
	GetDentsResponse_ptr p;
	p.p = capn_new_struct(s, 8, 1);
	return p;
}
GetDentsResponse_list new_GetDentsResponse_list(struct capn_segment *s, int len) {
	GetDentsResponse_list p;
	p.p = capn_new_list(s, len, 8, 1);
	return p;
}
void read_GetDentsResponse(struct GetDentsResponse *s capnp_unused, GetDentsResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->error = (int32_t) ((int32_t)capn_read32(p.p, 0));
	s->entries.p = capn_getp(p.p, 0, 0);
}
void write_GetDentsResponse(const struct GetDentsResponse *s capnp_unused, GetDentsResponse_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, (uint32_t) (s->error));
	capn_setp(p.p, 0, s->entries.p);
}
void get_GetDentsResponse(struct GetDentsResponse *s, GetDentsResponse_list l, int i) {
	GetDentsResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_GetDentsResponse(s, p);
}
void set_GetDentsResponse(const struct GetDentsResponse *s, GetDentsResponse_list l, int i) {
	GetDentsResponse_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_GetDentsResponse(s, p);
}
