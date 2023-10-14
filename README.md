<!--Copyright 2019-2023 Kai D. Gonzalez-->

# simpleINI

implements simple structures to work with INI statements. working on support for
sections as well

```c
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
```
