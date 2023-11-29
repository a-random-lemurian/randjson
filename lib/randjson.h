#ifndef RANDJSON_H
#define RANDJSON_H

typedef struct Prng {
  unsigned int x, y;
} Prng;

typedef struct JsonGenerator {
  int max_recursion;
  int max_array_len;
  int max_string_len;
} JsonGenerator;

char *randjson_make_json(unsigned int seed, JsonGenerator *JG);
void randjson_set_default(JsonGenerator *JG);
#endif /* RANDJSON_H */
