#ifndef CAPN_9EB32E19F86EE174
#define CAPN_9EB32E19F86EE174
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

#include "c.capnp.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Person;
struct Person_PhoneNumber;
struct AddressBook;

typedef struct {capn_ptr p;} Person_ptr;
typedef struct {capn_ptr p;} Person_PhoneNumber_ptr;
typedef struct {capn_ptr p;} AddressBook_ptr;

typedef struct {capn_ptr p;} Person_list;
typedef struct {capn_ptr p;} Person_PhoneNumber_list;
typedef struct {capn_ptr p;} AddressBook_list;

enum Person_PhoneNumber_Type {
	Person_PhoneNumber_Type_mobile = 0,
	Person_PhoneNumber_Type_home = 1,
	Person_PhoneNumber_Type_work = 2
};
enum Person_employment_which {
	Person_employment_unemployed = 0,
	Person_employment_employer = 1,
	Person_employment_school = 2,
	Person_employment_selfEmployed = 3
};

struct Person {
	uint32_t id;
	capn_text name;
	capn_text email;
	Person_PhoneNumber_list phones;
	enum Person_employment_which employment_which;
	capnp_nowarn union {
		capn_text employer;
		capn_text school;
	} employment;
};

static const size_t Person_word_count = 1;

static const size_t Person_pointer_count = 4;

static const size_t Person_struct_bytes_count = 40;

uint32_t Person_get_id(Person_ptr p);

capn_text Person_get_name(Person_ptr p);

capn_text Person_get_email(Person_ptr p);

Person_PhoneNumber_list Person_get_phones(Person_ptr p);

void Person_set_id(Person_ptr p, uint32_t id);

void Person_set_name(Person_ptr p, capn_text name);

void Person_set_email(Person_ptr p, capn_text email);

void Person_set_phones(Person_ptr p, Person_PhoneNumber_list phones);

struct Person_PhoneNumber {
	capn_text number;
	enum Person_PhoneNumber_Type type;
};

static const size_t Person_PhoneNumber_word_count = 1;

static const size_t Person_PhoneNumber_pointer_count = 1;

static const size_t Person_PhoneNumber_struct_bytes_count = 16;

capn_text Person_PhoneNumber_get_number(Person_PhoneNumber_ptr p);

enum Person_PhoneNumber_Type Person_PhoneNumber_get_type(Person_PhoneNumber_ptr p);

void Person_PhoneNumber_set_number(Person_PhoneNumber_ptr p, capn_text number);

void Person_PhoneNumber_set_type(Person_PhoneNumber_ptr p, enum Person_PhoneNumber_Type type);

struct AddressBook {
	Person_list people;
};

static const size_t AddressBook_word_count = 0;

static const size_t AddressBook_pointer_count = 1;

static const size_t AddressBook_struct_bytes_count = 8;

Person_list AddressBook_get_people(AddressBook_ptr p);

void AddressBook_set_people(AddressBook_ptr p, Person_list people);

Person_ptr new_Person(struct capn_segment*);
Person_PhoneNumber_ptr new_Person_PhoneNumber(struct capn_segment*);
AddressBook_ptr new_AddressBook(struct capn_segment*);

Person_list new_Person_list(struct capn_segment*, int len);
Person_PhoneNumber_list new_Person_PhoneNumber_list(struct capn_segment*, int len);
AddressBook_list new_AddressBook_list(struct capn_segment*, int len);

void read_Person(struct Person*, Person_ptr);
void read_Person_PhoneNumber(struct Person_PhoneNumber*, Person_PhoneNumber_ptr);
void read_AddressBook(struct AddressBook*, AddressBook_ptr);

void write_Person(const struct Person*, Person_ptr);
void write_Person_PhoneNumber(const struct Person_PhoneNumber*, Person_PhoneNumber_ptr);
void write_AddressBook(const struct AddressBook*, AddressBook_ptr);

void get_Person(struct Person*, Person_list, int i);
void get_Person_PhoneNumber(struct Person_PhoneNumber*, Person_PhoneNumber_list, int i);
void get_AddressBook(struct AddressBook*, AddressBook_list, int i);

void set_Person(const struct Person*, Person_list, int i);
void set_Person_PhoneNumber(const struct Person_PhoneNumber*, Person_PhoneNumber_list, int i);
void set_AddressBook(const struct AddressBook*, AddressBook_list, int i);

#ifdef __cplusplus
}
#endif
#endif
