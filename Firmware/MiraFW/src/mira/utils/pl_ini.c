/* psplib/pl_ini.h
   INI file reading/writing
   Copyright (C) 2007-2009 Akop Karapetyan
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
   Author contact information: dev@psp.akop.org
*/

#include "pl_ini.h"

#include <oni/utils/memory/allocator.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/logger.h>
#include <oni/framework.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/kernel.h>

#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#define PL_MAX_LINE_LENGTH 512

typedef struct pl_ini_pair_t
{
  char *key;
  char *value;
  struct pl_ini_pair_t *next;
} pl_ini_pair;

typedef struct pl_ini_section_t
{
  char *name;
  struct pl_ini_pair_t *head;
  struct pl_ini_section_t *next;
} pl_ini_section;

static pl_ini_section* create_section(const char *name);
static pl_ini_pair* create_pair(char *string);
static pl_ini_section* find_section(const pl_ini_file *file,
                                    const char *section_name);
static pl_ini_pair* find_pair(const pl_ini_section *section,
                              const char *key_name);
static pl_ini_pair* locate(const pl_ini_file *file,
                           const char *section_name,
                           const char *key_name);
static pl_ini_pair* locate_or_create(pl_ini_file *file,
                                     const char *section_name,
                                     const char *key_name);

char *strrchr (const char *s, int c) {
  char *rtnval = 0;

  do {
    if (*s == c)
      rtnval = (char*) s;
  } while (*s++);
  return (rtnval);
}

char *strchr(const char *s, int c)
{
  while (*s != (char)c)
    if (!*s++)
      return 0;
  return (char *)s;
}

char *strdup(const char* s) {
  void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
  size_t(*strlen)(const char* str) = kdlsym(strlen);

  int size = strlen(s);
  char* duplicate = kmalloc(size + 1);
  memset(duplicate, 0, size + 1);

  memcpy(duplicate, s, size);

  return duplicate;
}

int atoi(const char *str) 
  { 
    int sign = 1, base = 0, i = 0; 
    // if whitespaces then ignore. 
    while (str[i] == ' ') 
     {   
      i++; 
     } 
    // sign of number 
    if (str[i] == '-' || str[i] == '+') { 
        sign = 1 - 2 * (str[i++] == '-');  
    } 
   // checking for valid input 
    while (str[i] >= '0' && str[i] <= '9')  
      { 
        //handling overflow test case   
        if (base >  INT_MAX / 10 || (base == INT_MAX / 10 && str[i] - '0' > 7)) { 
            if (sign == 1) return INT_MAX; 
            else return INT_MIN; 
        } 
        base  = 10 * base + (str[i++] - '0'); 
    } 
    return base * sign; 
} 

int pl_ini_create(pl_ini_file *file)
{
  file->head = NULL;
  return 1;
}

int pl_ini_load(pl_ini_file *file,
                const char *path)
{
  file->head = NULL;

  void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
  size_t(*strlen)(const char* str) = kdlsym(strlen);
  char* (*strncpy)(char* dst, const char* src, size_t len) = kdlsym(strncpy);

  int fp = -1;
  int n, sz;

  /* Open file */
  fp = kopen((char*)path, O_RDONLY, 0);
  if (fp <= 0) {
    return 0;
  }

  /* Get file size */
  sz = klseek(fp, 0, SEEK_END);
  klseek(fp, 0, SEEK_SET);

  /* Load file content into memory, null terminate, init end var */
  void* data = (void*)kmalloc(sz + 1);
  memset(data, 0, sz + 1);
  n = kread(fp, data, sz);
  if (n != sz) {
    return 0;
  }

  /* Close file */
  kclose(fp);

  pl_ini_section *current_section = NULL;
  pl_ini_pair *tail;

  char* string = (char*)data;
  char name[PL_MAX_LINE_LENGTH];
  char *ptr;
  int len;

  /* Create unnamed section */
  current_section = NULL;
  tail = NULL;

  while((void*)string < (data + (sz+1)) && string[0] != 0)
  {
    /* TODO: Skip whitespace */
    /* New section */
    if (string[0] == '[')
    {
      if ((ptr = strrchr(string, ']')))
      {
        len = ptr - string - 1;
        strncpy(name, string + 1, len);
        name[len] = '\0';

        if (!current_section)
          current_section = file->head = create_section(name);
        else
        {
          current_section->next = create_section(name);
          current_section = current_section->next;
        }

        tail = NULL;
      }
    }
    else if (string[0] =='#'); /* Do nothing - comment */
    else
    {
      /* No section defined - create empty section */
      if (!current_section)
      {
        current_section = create_section(strdup(""));
        file->head = current_section;
        tail = NULL;
      }

      pl_ini_pair *pair = create_pair(string);
      if (pair)
      {
        if (tail) tail->next = pair;
        else current_section->head = pair;
        tail = pair;
      }
    }

    string = (char*)(data + (strlen(string) + 1));
  }

  return 1;
}

int pl_ini_save(const pl_ini_file *file,
                const char *path)
{
  int (*snprintf)(char * s, size_t n, const char * format, ...) = kdlsym(snprintf);
  size_t(*strlen)(const char* str) = kdlsym(strlen);

  char string[PL_MAX_LINE_LENGTH];
  int stream = kopen((char*)path, O_WRONLY|O_CREAT, 0777);
  if (stream<=0)
    return 0;

  pl_ini_section *section;
  pl_ini_pair *pair;

  for (section = file->head; section; section = section->next)
  {
    snprintf(string, PL_MAX_LINE_LENGTH, "[%s]\n\0", section->name);
    kwrite(stream, string, strlen(string));
    for (pair = section->head; pair; pair = pair->next){
      snprintf(string, PL_MAX_LINE_LENGTH, "%s=%s\n\0", pair->key, pair->value);
      kwrite(stream, string, strlen(string));
    }
  }
  kclose(stream);
  return 1;
}

int pl_ini_get_int(const pl_ini_file *file,
                   const char *section,
                   const char *key,
                   int default_value)
{
  pl_ini_pair *pair = locate(file, section, key);
  return (pair) ? atoi(pair->value) : default_value;
}

int pl_ini_get_string(const pl_ini_file *file,
                      const char *section,
                      const char *key,
                      const char *default_value,
                      char *copy_to,
                      int dest_len)
{
  char* (*strncpy)(char* dst, const char* src, size_t len) = kdlsym(strncpy);

  pl_ini_pair *pair = locate(file, section, key);
  if (pair)
  {
    strncpy(copy_to, pair->value, dest_len);
    return 1;
  }
  else if (default_value)
  {
    strncpy(copy_to, default_value, dest_len);
    return 1;
  }

  return 0;
}

void pl_ini_set_int(pl_ini_file *file,
                    const char *section,
                    const char *key,
                    int value)
{
  int (*snprintf)(char * s, size_t n, const char * format, ...) = kdlsym(snprintf);
  size_t(*strlen)(const char* str) = kdlsym(strlen);

  pl_ini_pair *pair;
  if (!(pair = locate_or_create(file, section, key)))
    return;

  /* Replace the value */
  if (pair->value)
    kfree(pair->value, strlen(pair->value)+1);

  char temp[64];
  snprintf(temp, 63, "%i", value);
  pair->value = strdup(temp);
}

void pl_ini_set_string(pl_ini_file *file,
                       const char *section,
                       const char *key,
                       const char *string)
{
  size_t(*strlen)(const char* str) = kdlsym(strlen);

  pl_ini_pair *pair;
  if (!(pair = locate_or_create(file, section, key)))
    return;

  /* Replace the value */
  if (pair->value)
    kfree(pair->value, strlen(pair->value)+1);
  pair->value = strdup(string);
}

void pl_ini_destroy(pl_ini_file *file)
{
  size_t(*strlen)(const char* str) = kdlsym(strlen);

  pl_ini_section *section, *next_section;
  pl_ini_pair *pair, *next_pair;

  for (section = file->head; section; section = next_section)
  {
    next_section = section->next;

    if (section->name)
      kfree(section->name, strlen(section->name)+1);
    for (pair = section->head; pair; pair = next_pair)
    {
      next_pair = pair->next;
      if (pair->key)
        kfree(pair->key, strlen(pair->key)+1);
      if (pair->value)
        kfree(pair->value, strlen(pair->value)+1);
      kfree(pair, sizeof(pl_ini_pair));
    }

    kfree(section, sizeof(pl_ini_section));
  }
}

static pl_ini_section* find_section(const pl_ini_file *file,
                                    const char *section_name)
{
  int (*strcmp)(const char *s1, const char *s2) = kdlsym(strcmp);

  pl_ini_section *section;
  for (section = file->head; section; section = section->next)
    if (strcmp(section_name, section->name) == 0)
      return section;

  return NULL;
}

static pl_ini_pair* find_pair(const pl_ini_section *section,
                              const char *key_name)
{
  int (*strcmp)(const char *s1, const char *s2) = kdlsym(strcmp);

  pl_ini_pair *pair;
  for (pair = section->head; pair; pair = pair->next)
    if (strcmp(pair->key, key_name) == 0)
      return pair;

  return NULL;
}

static pl_ini_pair* locate(const pl_ini_file *file,
                           const char *section_name,
                           const char *key_name)
{
  pl_ini_section *section;
  if (!(section = find_section(file, section_name)))
    return NULL;

  return find_pair(section, key_name);
}

static pl_ini_pair* locate_or_create(pl_ini_file *file,
                                     const char *section_name,
                                     const char *key_name)
{
  pl_ini_section *section = find_section(file, section_name);
  pl_ini_pair *pair = NULL;

  if (section)
    pair = find_pair(section, key_name);
  else
  {
    /* Create section */
    section = create_section(section_name);

    if (!file->head)
      file->head = section;
    else
    {
      pl_ini_section *s;
      for (s = file->head; s->next; s = s->next); /* Find the tail */
      s->next = section;
    }
  }

  if (!pair)
  {
    /* Create pair */
    pair = (pl_ini_pair*)kmalloc(sizeof(pl_ini_pair));
    pair->key = strdup(key_name);
    pair->value = NULL;
    pair->next = NULL;

    if (!section->head)
      section->head = pair;
    else
    {
      pl_ini_pair *p;
      for (p = section->head; p->next; p = p->next); /* Find the tail */
      p->next = pair;
    }
  }

  return pair;
}

static pl_ini_section* create_section(const char *name)
{
  pl_ini_section *section
    = (pl_ini_section*)kmalloc(sizeof(pl_ini_section));
  section->head = NULL;
  section->next = NULL;
  section->name = strdup(name);

  return section;
}

static pl_ini_pair* create_pair(char *string)
{
  size_t(*strlen)(const char* str) = kdlsym(strlen);
  char* (*strncpy)(char* dst, const char* src, size_t len) = kdlsym(strncpy);

  char *ptr;
  if (!(ptr = strchr(string, '=')))
    return NULL;

  int len;
  char *name, *value;

  /* Copy NAME */
  len = ptr - string;
  if (!(name = (char*)kmalloc(sizeof(char) * (len + 1))))
    return NULL;
  strncpy(name, string, len);
  name[len] = '\0';

  /* Copy VALUE */
  if (!(value = strdup(ptr + 1)))
  {
    kfree(name, strlen(name)+1);
    return NULL;
  }

  len = strlen(value);
  if (value[len - 1] == '\n') value[len - 1] = '\0';

  /* Create struct */
  pl_ini_pair *pair = (pl_ini_pair*)kmalloc(sizeof(pl_ini_pair));

  if (!pair)
  {
    kfree(name, strlen(name)+1);
    kfree(value, strlen(value)+1);
    return NULL;
  }

  pair->key = name;
  pair->value = value;
  pair->next = NULL;

  return pair;
}