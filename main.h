/*Copyright 2019-2023 Kai D. Gonzalez*/

#ifndef MAIN_H
#define MAIN_H

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INI_MAX_NAME_SIZE 256
#define INI_INITIAL_SECTION_SIZE 16

#define INI_STAT_SEPARATOR '='
#define INI_STAT_SECTION_START '['
#define INI_STAT_SECTION_END ']'
#define INI_STAT_NEW_LINE '\n'
#define INI_QUOTE_CHAR '"'
#define INI_ESCAPE_CHAR '\\'

#define INI_STATE_INIT 0
#define INI_STATE_NAME 1
#define INI_STATE_VALUE 2
#define INI_STATE_SECTION 3
#define INI_STATE_STRING 4


#define INI_NUMBER float

// defines a simple text object in INI
typedef struct
{
  char *ptr;
  size_t size;
} ini_name;

// defines a buffer that can be added to and removed
typedef struct
{
  char *ptr;
  size_t size;
} ini_temp;

// defiens a parser for INI files
typedef struct
{
  int state;
  ini_temp *buf;
} ini_parser;

// value types
enum ini_value_type
{
  INI_VALUE_TYPE_STRING,
  INI_VALUE_TYPE_NUMBER,
};

// defines an INI value
typedef struct
{
  char *ptr;
  enum ini_value_type type;
} ini_value;

// defines an INI key-value
typedef struct
{
  ini_name *key;
  ini_name *value;
} ini_key_value;

// defines a top level INI section
typedef struct
{
  // ini_name *ptrrname;
  ini_key_value **key_value;
  size_t size;
  size_t count;
} ini_section;

ini_name *i_create_name (const char *name);
void i_destroy_name (ini_name *name);

ini_temp *i_create_temp (void);
void i_append_temp (ini_temp *temp, char n);
void i_destroy_temp (ini_temp *temp);
void i_remove_char (ini_temp *temp, int index);
char *i_temp_ptr (ini_temp *temp);
int i_buffer_equals (ini_temp *a, char *N);
void i_buffer_easy_fix_size (ini_temp *temp);
void i_str_strip (ini_temp *str);
void i_buffer_append_str (ini_temp *str, char *n);
int i_buffer_checkempty (ini_temp *t);
void i_buffer_clear (ini_temp *t);

ini_parser *i_create_parser (void);
void i_destroy_parser (ini_parser *parser);
void i_parser_update_state (ini_parser *parser, int state);
int i_parser_get_state (ini_parser *parser);
void i_parser_append (ini_parser *parser, char n);
size_t i_parser_get_buffer_size (ini_parser *parser);
ini_section* i_parser_parse (ini_parser *parser, char *stats);

ini_key_value *i_create_key_value (ini_name *key, ini_name *value);
void i_destroy_key_value (ini_key_value *key_value);
ini_name *i_key_value_get_key (ini_key_value *key_value);
ini_name *i_key_value_get_value (ini_key_value *key_value);

ini_value *i_create_value (enum ini_value_type type, char *ptr);
void i_destroy_value (ini_value *value);
ini_value *i_create_value_from_temp (enum ini_value_type type, ini_temp *temp);
INI_NUMBER i_asnumber (ini_value *value);
char *i_as_string (ini_value *value);
enum ini_value_type i_guess_type (char *str);
ini_value *i_parse_string (ini_value *v);

ini_section *i_create_section ();
void i_destroy_section (ini_section *section);
void i_section_add_key_value (ini_section *section, ini_key_value *key_value);
ini_key_value *i_section_get_key_value (ini_section *section, char *key);

#endif
