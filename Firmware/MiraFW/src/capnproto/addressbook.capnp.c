#include "addressbook.capnp.h"
/* AUTO GENERATED - DO NOT EDIT */
#ifdef __GNUC__
# define capnp_unused __attribute__((unused))
# define capnp_use(x) (void) x;
#else
# define capnp_unused
# define capnp_use(x)
#endif

static const capn_text capn_val0 = {0,"",0};

Person_ptr new_Person(struct capn_segment *s) {
	Person_ptr p;
	p.p = capn_new_struct(s, 8, 4);
	return p;
}
Person_list new_Person_list(struct capn_segment *s, int len) {
	Person_list p;
	p.p = capn_new_list(s, len, 8, 4);
	return p;
}
void read_Person(struct Person *s capnp_unused, Person_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->id = capn_read32(p.p, 0);
	s->name = capn_get_text(p.p, 0, capn_val0);
	s->email = capn_get_text(p.p, 1, capn_val0);
	s->phones.p = capn_getp(p.p, 2, 0);
	s->employment_which = (enum Person_employment_which)(int) capn_read16(p.p, 4);
	switch (s->employment_which) {
	case Person_employment_employer:
	case Person_employment_school:
		s->employment.school = capn_get_text(p.p, 3, capn_val0);
		break;
	default:
		break;
	}
}
void write_Person(const struct Person *s capnp_unused, Person_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, s->id);
	capn_set_text(p.p, 0, s->name);
	capn_set_text(p.p, 1, s->email);
	capn_setp(p.p, 2, s->phones.p);
	capn_write16(p.p, 4, s->employment_which);
	switch (s->employment_which) {
	case Person_employment_employer:
	case Person_employment_school:
		capn_set_text(p.p, 3, s->employment.school);
		break;
	default:
		break;
	}
}
void get_Person(struct Person *s, Person_list l, int i) {
	Person_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_Person(s, p);
}
void set_Person(const struct Person *s, Person_list l, int i) {
	Person_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_Person(s, p);
}

uint32_t Person_get_id(Person_ptr p)
{
	uint32_t id;
	id = capn_read32(p.p, 0);
	return id;
}

capn_text Person_get_name(Person_ptr p)
{
	capn_text name;
	name = capn_get_text(p.p, 0, capn_val0);
	return name;
}

capn_text Person_get_email(Person_ptr p)
{
	capn_text email;
	email = capn_get_text(p.p, 1, capn_val0);
	return email;
}

Person_PhoneNumber_list Person_get_phones(Person_ptr p)
{
	Person_PhoneNumber_list phones;
	phones.p = capn_getp(p.p, 2, 0);
	return phones;
}

void Person_set_id(Person_ptr p, uint32_t id)
{
	capn_write32(p.p, 0, id);
}

void Person_set_name(Person_ptr p, capn_text name)
{
	capn_set_text(p.p, 0, name);
}

void Person_set_email(Person_ptr p, capn_text email)
{
	capn_set_text(p.p, 1, email);
}

void Person_set_phones(Person_ptr p, Person_PhoneNumber_list phones)
{
	capn_setp(p.p, 2, phones.p);
}

Person_PhoneNumber_ptr new_Person_PhoneNumber(struct capn_segment *s) {
	Person_PhoneNumber_ptr p;
	p.p = capn_new_struct(s, 8, 1);
	return p;
}
Person_PhoneNumber_list new_Person_PhoneNumber_list(struct capn_segment *s, int len) {
	Person_PhoneNumber_list p;
	p.p = capn_new_list(s, len, 8, 1);
	return p;
}
void read_Person_PhoneNumber(struct Person_PhoneNumber *s capnp_unused, Person_PhoneNumber_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->number = capn_get_text(p.p, 0, capn_val0);
	s->type = (enum Person_PhoneNumber_Type)(int) capn_read16(p.p, 0);
}
void write_Person_PhoneNumber(const struct Person_PhoneNumber *s capnp_unused, Person_PhoneNumber_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_set_text(p.p, 0, s->number);
	capn_write16(p.p, 0, (uint16_t) (s->type));
}
void get_Person_PhoneNumber(struct Person_PhoneNumber *s, Person_PhoneNumber_list l, int i) {
	Person_PhoneNumber_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_Person_PhoneNumber(s, p);
}
void set_Person_PhoneNumber(const struct Person_PhoneNumber *s, Person_PhoneNumber_list l, int i) {
	Person_PhoneNumber_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_Person_PhoneNumber(s, p);
}

capn_text Person_PhoneNumber_get_number(Person_PhoneNumber_ptr p)
{
	capn_text number;
	number = capn_get_text(p.p, 0, capn_val0);
	return number;
}

enum Person_PhoneNumber_Type Person_PhoneNumber_get_type(Person_PhoneNumber_ptr p)
{
	enum Person_PhoneNumber_Type type;
	type = (enum Person_PhoneNumber_Type)(int) capn_read16(p.p, 0);
	return type;
}

void Person_PhoneNumber_set_number(Person_PhoneNumber_ptr p, capn_text number)
{
	capn_set_text(p.p, 0, number);
}

void Person_PhoneNumber_set_type(Person_PhoneNumber_ptr p, enum Person_PhoneNumber_Type type)
{
	capn_write16(p.p, 0, (uint16_t) (type));
}

AddressBook_ptr new_AddressBook(struct capn_segment *s) {
	AddressBook_ptr p;
	p.p = capn_new_struct(s, 0, 1);
	return p;
}
AddressBook_list new_AddressBook_list(struct capn_segment *s, int len) {
	AddressBook_list p;
	p.p = capn_new_list(s, len, 0, 1);
	return p;
}
void read_AddressBook(struct AddressBook *s capnp_unused, AddressBook_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->people.p = capn_getp(p.p, 0, 0);
}
void write_AddressBook(const struct AddressBook *s capnp_unused, AddressBook_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_setp(p.p, 0, s->people.p);
}
void get_AddressBook(struct AddressBook *s, AddressBook_list l, int i) {
	AddressBook_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_AddressBook(s, p);
}
void set_AddressBook(const struct AddressBook *s, AddressBook_list l, int i) {
	AddressBook_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_AddressBook(s, p);
}

Person_list AddressBook_get_people(AddressBook_ptr p)
{
	Person_list people;
	people.p = capn_getp(p.p, 0, 0);
	return people;
}

void AddressBook_set_people(AddressBook_ptr p, Person_list people)
{
	capn_setp(p.p, 0, people.p);
}
