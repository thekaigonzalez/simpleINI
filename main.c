/*Copyright 2019-2023 Kai D. Gonzalez*/

#include "main.h"

ini_name *
i_create_name (const char *name)
{
  if (name == NULL)
    return NULL;

  ini_name *ini = malloc (sizeof (ini_name));

  if (ini == NULL)
    return NULL;

  ini->ptr = malloc (strlen (name) + 1);

  if (ini->ptr == NULL)
    {
      free (ini);
      return NULL;
    }

  strcpy (ini->ptr, name);
  ini->size = strlen (name);

  return ini;
}

void
i_destroy_name (ini_name *ini)
{
  if (ini == NULL)
    return;

  free (ini->ptr);
  free (ini);

  ini = NULL;
}

ini_temp *
i_create_temp (void)
{
  ini_temp *temp = malloc (sizeof (ini_temp));

  if (temp == NULL)
    return NULL;

  temp->ptr = malloc (1);
  temp->ptr[0] = '\0';
  temp->size = 0;

  return temp;
}

void
i_append_temp (ini_temp *temp, char n)
{
  if (temp == NULL)
    return;
  temp->ptr = realloc (temp->ptr, temp->size + 2);

  if (temp->ptr == NULL)
    return;

  temp->ptr[temp->size] = n;
  temp->ptr[temp->size + 1] = '\0';

  temp->size++;
}

void
i_destroy_temp (ini_temp *temp)
{
  if (temp == NULL)
    return;

  free (temp->ptr);
  free (temp);

  temp->ptr = NULL;
  temp = NULL;
}

void
i_remove_char (ini_temp *temp, int index)
{
  if (temp == NULL)
    return;
  if (temp->ptr == NULL)
    return;

  if (index < 0 || index >= temp->size)
    return;

  memmove (temp->ptr + index, temp->ptr + index + 1, temp->size - index);
  temp->size--;

  if (temp->ptr == NULL)
    fprintf (stderr,
             "ini: failed to remove character (the string is NULL?)\n");
}

char *
i_temp_ptr (ini_temp *temp)
{
  if (temp == NULL)
    return NULL;
  if (temp->ptr == NULL)
    return NULL;
  return temp->ptr;
}

int
i_buffer_equals (ini_temp *a, char *N)
{
  if (a == NULL || N == NULL)
    return 0;

  if (strcmp (a->ptr, N) == 0)
    return 1;
  return 0;
}

void
i_buffer_easy_fix_size (ini_temp *temp)
{
  if (temp == NULL)
    return;

  if (temp->ptr == NULL)
    {
      free (temp->ptr);
      temp->ptr = malloc (1);
    }
  temp->size = strlen (temp->ptr);
}

ini_parser *
i_create_parser (void)
{
  ini_parser *parser = malloc (sizeof (ini_parser));

  if (parser == NULL)
    return NULL;

  parser->state = INI_STATE_INIT;
  parser->buf = i_create_temp ();

  return parser;
}

void
i_destroy_parser (ini_parser *parser)
{
  if (parser == NULL)
    return;

  i_destroy_temp (parser->buf);
  free (parser);
  parser = NULL;
}

void
i_parser_update_state (ini_parser *parser, int state)
{
  if (parser == NULL)
    return;

  parser->state = state;
}

int
i_parser_get_state (ini_parser *parser)
{
  if (parser == NULL)
    return -1;

  return parser->state;
}

void
i_parser_append (ini_parser *parser, char n)
{
  if (parser == NULL)
    return;

  i_append_temp (parser->buf, n);
}

size_t
i_parser_get_buffer_size (ini_parser *parser)
{
  if (parser == NULL)
    return -1;

  return parser->buf->size;
}

ini_section *
i_parser_parse (ini_parser *parser, char *stats)
{
  ini_section *section = i_create_section ();
  if (parser == NULL)
    return NULL;

  ini_temp *t = parser->buf;

  char *copycat = NULL;

  ini_key_value *CUR = NULL;

  ini_name *key = NULL;
  ini_name *value = NULL;

  int LINE = 1;
  int CHAR = 1;

  int prev_state = i_parser_get_state (parser);

  i_parser_update_state (parser, INI_STATE_NAME);

  while (*stats != '\0')
    {
      CHAR++; // the character number we're on
      if (*stats == INI_STAT_SECTION_START
          && i_parser_get_state (parser)
                 == INI_STATE_NAME) // stop collecting the name, start
                                    // collecting the section name instead
        {
          i_parser_update_state (parser, INI_STATE_SECTION);
        }
      else if (*stats == INI_STAT_SEPARATOR
               && (i_parser_get_state (parser) == INI_STATE_NAME))
        { // left/right sides (assuming there's no section), collected the
          // variable name (or value)
          i_str_strip (t);

          i_parser_update_state (parser, INI_STATE_VALUE);

          if (copycat != NULL)
            {
              copycat = realloc (copycat, strlen (t->ptr) + 1);
            }
          else
            {
              copycat = malloc (strlen (t->ptr) + 1);
            }
          strcpy (copycat, t->ptr);

          i_buffer_clear (t);

          key = i_create_name (copycat);

          i_parser_update_state (parser, INI_STATE_VALUE);

          // free(copycat);
          // copycat = NULL;
        }
      else if (*stats == INI_STAT_NEW_LINE
               && i_parser_get_state (parser) != INI_STATE_SECTION
               && i_parser_get_state (parser) != INI_STATE_NAME)
        {
          LINE++;

          i_parser_update_state (parser, INI_STATE_NAME);

          i_str_strip (t);

          if (copycat != NULL)
            {
              copycat = realloc (copycat, strlen (t->ptr) + 1);
            }
          else
            {
              copycat = malloc (strlen (t->ptr) + 1);
            }

          strcpy (copycat, t->ptr);

          i_buffer_clear (t);

          value = i_create_name (copycat);

          CUR = i_create_key_value (key, value);
          i_section_add_key_value (section, CUR);

          free(copycat);
          free (key);
          free (value);
          
          key = NULL;
          value = NULL;
          copycat = NULL;

          // printf("key and value: %s %s\n", key->ptr, value->ptr);
        }
      else if (*stats == INI_QUOTE_CHAR
               && i_parser_get_state (parser) != INI_STATE_SECTION)
        {
          prev_state = i_parser_get_state (parser);
          i_parser_update_state (parser, INI_STATE_STRING);
        }
      else if (*stats == INI_QUOTE_CHAR
               && i_parser_get_state (parser) == INI_STATE_STRING
               && *(stats - 1) != INI_ESCAPE_CHAR)
        {
          i_parser_update_state (parser, prev_state);
        }
      else
        {
          if (!ispunct (*stats)
              || i_parser_get_state (parser) == INI_STATE_STRING)
            i_parser_append (parser, *stats);
          else
            {
              fprintf (stderr, "ini(%d:%d): invalid character '%c'\n", LINE,
                       CHAR, *stats);
              break;
              // exit (1);
            }
        }
      if (*stats != '\0')
        *stats++;
    }

  if (copycat != NULL)
    {
      i_parser_update_state (parser, INI_STATE_VALUE);

      i_str_strip (t);

      if (copycat != NULL)
        {
          copycat = realloc (copycat, strlen (t->ptr) + 1);
        }
      else
        {
          copycat = malloc (strlen (t->ptr) + 1);
        }

      strcpy (copycat, t->ptr);

      i_buffer_clear (t);

      value = i_create_name (copycat);

      if (key == NULL) {
        fprintf(stderr, "ini(INTERNAL:%d): missing key\n", CHAR);
      }
      CUR = i_create_key_value (key, value);
      i_section_add_key_value (section, CUR);


      i_destroy_name (key);
      i_destroy_name (value);

      key = NULL;
      value = NULL;
    }
    
  free (copycat);
  i_destroy_temp (t);
  
  free(CUR);

  parser = NULL;
  copycat = NULL;
  return section;
}

ini_key_value *
i_create_key_value (ini_name *key, ini_name *value)
{
  ini_key_value *key_value = malloc (sizeof (ini_key_value));

  if (key_value == NULL)
    return NULL;

  key_value->key = i_create_name (key->ptr);
  key_value->value = i_create_name (value->ptr);

  return key_value;
}

void
i_destroy_key_value (ini_key_value *key_value)
{
  if (key_value == NULL)
    return;

  i_destroy_name (key_value->key);
  i_destroy_name (key_value->value);

  free (key_value);

  key_value->key = NULL;
  key_value->value = NULL;
  key_value = NULL;
}

ini_name *
i_key_value_get_key (ini_key_value *key_value)
{
  if (key_value == NULL)
    return NULL;

  return key_value->key;
}

ini_name *
i_key_value_get_value (ini_key_value *key_value)
{
  if (key_value == NULL)
    return NULL;

  return key_value->value;
}

ini_value *
i_create_value (enum ini_value_type type, char *ptr)
{
  ini_value *value = malloc (sizeof (ini_value));

  if (value == NULL)
    {
      fprintf (stderr, "ini: failed to create value\n");
      free (value);
      return NULL;
    }

  value->ptr = malloc (strlen (ptr) + 1);

  if (value->ptr == NULL)
    {
      fprintf (stderr, "ini: failed to create value\n");
      free (value->ptr);
      free (value);
      return NULL;
    }

  strcpy (value->ptr, ptr);
  value->type = type;

  return value;
}

void
i_destroy_value (ini_value *value)
{
  if (value == NULL)
    return;

  free (value->ptr);
  free (value);
  value = NULL;
}

ini_value *
i_create_value_from_temp (enum ini_value_type type, ini_temp *temp)
{
  if (temp == NULL)
    return NULL;

  ini_value *value = i_create_value (type, temp->ptr);
  free (temp);
  return value;
}

INI_NUMBER
i_asnumber (ini_value *value)
{
  if (value == NULL)
    return 0;

  if (value->type != INI_VALUE_TYPE_NUMBER)
    return 0;

  return atoi (value->ptr);
}

char *
i_as_string (ini_value *value)
{
  if (value == NULL)
    return NULL;

  if (value->type != INI_VALUE_TYPE_STRING)
    return NULL;

  if (value->ptr == NULL)
    return NULL;

  if (value->ptr[0] == '"')
    {
      value = i_parse_string (value);
    }

  char *s = value->ptr;

  if (s == NULL)
    return NULL;

  free (value->ptr);
  free (value);

  return s;
}

enum ini_value_type
i_guess_type (char *str)
{
  int n = 0; // is a number
  int st = 0;
  int i = 0;

  if (str[0] == '"')
    return INI_VALUE_TYPE_STRING;
  if (str[0] == '-')
    return INI_VALUE_TYPE_NUMBER;

  while (*str != '\0')
    {
      if (isdigit (*str))
        n = 1;
      else
        n = 0;

      if (isalpha (*str))
        st = 1;
      i++;
      *str++;
    }

  if (n == 1)
    return INI_VALUE_TYPE_NUMBER;
  if (st == 1)
    return INI_VALUE_TYPE_STRING;
}

// returns a ini_value which the inner value is stripped of `"` and `"`
// turns "Hello" into Hello
ini_value *
i_parse_string (ini_value *v)
{
  ini_value *temp = NULL;
  ini_temp *buf = i_create_temp ();

  int cmp = 0;

  if (v == NULL)
    return NULL;

  if (v->type != INI_VALUE_TYPE_STRING)
    return NULL;

  if (v->ptr[0] != '"')
    return NULL;

  for (int i = 1; i < strlen (v->ptr); ++i)
    {
      if (v->ptr[i] == '"' && v->ptr[i - 1] != '\\')
        {
          cmp = 1;
          break;
        }
      else
        {
          i_append_temp (buf, v->ptr[i]);
        }
    }

  if (cmp == 0)
    return NULL;

  temp = i_create_value_from_temp (INI_VALUE_TYPE_STRING, buf);

  i_destroy_temp (buf);

  return temp;
}

ini_section *
i_create_section ()
{
  // if (sname == NULL)
  // return NULL;
  ini_section *section
      = malloc (sizeof (ini_section)); // allocate the section itself
  section->key_value
      = malloc (sizeof (ini_key_value *)
                * INI_INITIAL_SECTION_SIZE); // allocate the initial section
  section->size = INI_INITIAL_SECTION_SIZE;  // the initial section size (will
                                             // increase as needed )
  // section->ptrrname = sname;                 // the name of the section
  section->count
      = 0; // how many items are in the section, increases when adding items

  return section;
}

void
i_destroy_section (ini_section *section)
{
  if (section == NULL)
    return;

  if (section->key_value == NULL)
    return;

  // if (section->ptrrname == NULL)
  // return;

  for (int i = 0; i < section->count; i++)
    {
      if (section->key_value[i] == NULL)
        {
          fprintf (stderr, "NULL\n");
          i_destroy_name (section->key_value[i]->key);
          i_destroy_name (section->key_value[i]->value);
          section->key_value[i] = NULL;
        }
      else
        {

          i_destroy_name (section->key_value[i]->key);
          i_destroy_name (section->key_value[i]->value);
          section->key_value[i] = NULL;
        }
    }

  free (section->key_value);
  // free (section->ptrrname);
  free (section);

  section->key_value = NULL;
  // section->ptrrname = NULL;
  section = NULL;
}

void
i_section_add_key_value (ini_section *section, ini_key_value *key_value)
{
  if (section == NULL)
    return;

  if (section->key_value == NULL)
    return;

  section->key_value[section->count] = key_value;
  
  section->count++;
}

ini_key_value *
i_section_get_key_value (ini_section *section, char *key)
{
  if (section == NULL)
    return NULL;

  if (section->key_value == NULL)
    return NULL;

  for (int i = 0; i < section->count; i++)
    {
      if (strcmp (section->key_value[i]->key->ptr, key) == 0)
        {
          return section->key_value[i];
        }
    }
  return NULL;
}

ini_value *
i_value_to_string (ini_value *value)
{
  if (value == NULL)
    return NULL;

  if (value->ptr == NULL)
    return NULL;

  switch (value->type)
    {
    case INI_VALUE_TYPE_STRING:
      return value;
    case INI_VALUE_TYPE_NUMBER:
      return i_create_value (INI_VALUE_TYPE_STRING, value->ptr);
    }
}

void
i_str_strip (ini_temp *str)
{
  if (str == NULL)
    return;

  if (str->ptr == NULL)
    return;

  for (int i = 0; i < str->size; i++)
    {
      if (isspace (str->ptr[i]))
        {
          i_remove_char (str, i);
          i--;
        }
      else
        break;
    }

  for (int i = str->size - 1; i > 0; i--)
    {
      if (isspace (str->ptr[i]))
        {
          i_remove_char (str, i);
        }
      else
        return;
    }
}

void
i_buffer_append_str (ini_temp *str, char *n)
{
  if (str == NULL)
    return;

  if (str->ptr == NULL)
    return;

  str->ptr = realloc (str->ptr, str->size + strlen (n));
  strcat (str->ptr, n);

  str->size += strlen (n);
}

int
i_buffer_checkempty (ini_temp *t)
{
  if (t == NULL)
    return 0;

  if (t->ptr == NULL)
    return 0;

  i_str_strip (t);

  if (t->size == 0 || t->ptr[0] == '\0')
    return 1;
  return 0;
}

void
i_buffer_clear (ini_temp *t)
{
  if (t == NULL)
    return;

  if (t->ptr == NULL)
    return;

  free (t->ptr);
  t->size = 0;

  t->ptr = malloc (1);
  t->size = 0;
}

void
print_key_value (ini_key_value **key_value)
{
  if (key_value == NULL)
    return;

  while (*key_value != NULL)
    {
      printf ("\t%s\t %s\n", (*key_value)->key->ptr, (*key_value)->value->ptr);
      *key_value++;
    }
}

int
main (void)
{
  ini_parser *p = i_create_parser ();

  ini_section *s = i_parser_parse (p, "a = 1\nb = 3\nc = 5");

  print_key_value (s->key_value);

  if (p == NULL)
    return 1;

  i_destroy_parser (p);
  i_destroy_section (s);

  return 0;
}
