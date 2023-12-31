#ifndef RANDJSON_H
#define RANDJSON_H

typedef struct JsonGenerator {
  int max_recursion;   /* Maximum recursion level. */
  int max_array_len;   /* Maximum size of both keys and values. */
  int max_string_len;  /* Maximum array length. */
  int max_keys;        /* Maximum keys per JSON object. */
} JsonGenerator;

char *randjson_make_json(unsigned int seed, JsonGenerator *JG);
void randjson_set_default(JsonGenerator *JG);
#endif /* RANDJSON_H */
