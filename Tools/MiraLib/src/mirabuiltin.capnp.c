#include "mirabuiltin.capnp.h"
/* AUTO GENERATED - DO NOT EDIT */
#ifdef __GNUC__
# define capnp_unused __attribute__((unused))
# define capnp_use(x) (void) x;
#else
# define capnp_unused
# define capnp_use(x)
#endif

static const capn_text capn_val0 = {0,"",0};

Message_ptr new_Message(struct capn_segment *s) {
	Message_ptr p;
	p.p = capn_new_struct(s, 8, 1);
	return p;
}
Message_list new_Message_list(struct capn_segment *s, int len) {
	Message_list p;
	p.p = capn_new_list(s, len, 8, 1);
	return p;
}
void read_Message(struct Message *s capnp_unused, Message_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->category = (enum MessageCategory)(int) capn_read16(p.p, 0);
	s->type = capn_read32(p.p, 4);
	s->containedMessage = capn_get_text(p.p, 0, capn_val0);
}
void write_Message(const struct Message *s capnp_unused, Message_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write16(p.p, 0, (uint16_t) (s->category));
	capn_write32(p.p, 4, s->type);
	capn_set_text(p.p, 0, s->containedMessage);
}
void get_Message(struct Message *s, Message_list l, int i) {
	Message_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_Message(s, p);
}
void set_Message(const struct Message *s, Message_list l, int i) {
	Message_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_Message(s, p);
}
