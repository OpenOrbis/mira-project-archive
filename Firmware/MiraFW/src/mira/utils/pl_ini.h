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

#ifndef _PL_INI_H
#define _PL_INI_H

#ifdef __cplusplus
extern "C" {
#endif

struct pl_ini_section_t;

typedef struct pl_ini_file_t
{
  struct pl_ini_section_t *head;
} pl_ini_file;

int  pl_ini_create(pl_ini_file *file);
void pl_ini_destroy(pl_ini_file *file);
int  pl_ini_load(pl_ini_file *file,
                 const char *path);
int  pl_ini_save(const pl_ini_file *file,
                 const char *path);
int  pl_ini_get_int(const pl_ini_file *file,
                    const char *section,
                    const char *key,
                    const int default_value);
void pl_ini_set_int(pl_ini_file *file,
                    const char *section,
                    const char *key,
                    int value);
int  pl_ini_get_string(const pl_ini_file *file,
                       const char *section,
                       const char *key,
                       const char *default_value,
                       char *copy_to,
                       int dest_len);
void pl_ini_set_string(pl_ini_file *file,
                       const char *section,
                       const char *key,
                       const char *string);

#ifdef __cplusplus
}
#endif

#endif // _PL_INI_H