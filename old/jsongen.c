/*
 * random json generator
 */
#include "jsongen.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define JSON_SEP ","
#define JSON_KEY_VAL ":"

#define NUMBER 1
#define STRING 2
#define OBJECT 3

/****************************************************************************
 * RANDOM NUMBER GENERATOR
 ****************************************************************************/
static void _jsongen_prng_seed(struct _jsongen_Prng *p, unsigned int iSeed) {
  p->x = iSeed | 1;
  p->y = iSeed;
}

/* Extract a random number */
static unsigned int _jsongen_prng_int(struct _jsongen_Prng *p) {
  p->x = (p->x >> 1) ^ ((1 + ~(p->x & 1)) & 0xd0000001);
  p->y = p->y * 1103515245 + 12345;
  return p->x ^ p->y;
}

/****************************************************************************
 * RANDOM VALUE GENERATORS
 ****************************************************************************/

static const char _b64[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789+/";
void _jsongen_b64(int length, char* out, struct jsongen_Prng *p) {
  for (int i = 0; i < length; i++) {
    out[i] = _b64[_jsongen_prng_int(p) % 65];
  }
}

void _jsongen_number(int length, char* out, struct jsongen_Prng *p) {
  int i = _jsongen_prng_int(p);
  sprintf(out, "%d", i);
}

void _jsongen_obj(int recursion, int max_keys, struct JsonGenerator *JG,
                  struct _jsongen_Prng *p, char* out) {
  int current_size = 0;
  int value_type = 0;
  char* key = malloc(200);
  char* value = malloc(200);

  strcat(out, "{");

  int vals = _jsongen_prng_int(p) % JG->max_vals_per_obj;

  for (int i = 0; i < vals; i++) {

    // Generate a key.
    int key_len = _jsongen_prng_int(p) % 7 + 2;
    current_size += key_len;
    _jsongen_b64(key_len, key, p);
    _jsongen_concat_json_key(out, key);
    strcat(JG->out_json, JSON_KEY_VAL);

    // Generate a value.
    int value_len = _jsongen_prng_int(p) % 50 + 20;
    current_size += value_len;
    int chosen_type = _jsongen_prng_int(p) % 2;
    switch (chosen_type)
    {
      case NUMBER:
        _jsongen_number(value_len, value, p);
        _jsongen_concat_json_value(out, value);
        break;
      case STRING:
        _jsongen_b64(value_len, value, p);
        _jsongen_concat_json_string(out, value);
        break;
    }

    // Do not print trailing commas.
    if (!(i == vals-1)) {
      strcat(out, JSON_SEP);
    }
  }
  strcat(out, "}");
  free(key);
  free(value);
}

/****************************************************************************
 * CONCATENATORS
 ****************************************************************************/

void _jsongen_concat_json_key(char *dest, char *src) {
  strcat(dest, "\"");
  strcat(dest, src);
  strcat(dest, "\"");
}

#define _jsongen_concat_json_string(d,s) _jsongen_concat_json_key(d,s);

void _jsongen_concat_json_value(char* dest, char *src) {
  strcat(dest, src);
}

/****************************************************************************
 * PUBLIC API
 ****************************************************************************/

void jsongen_prepare_JsonGenerator(struct JsonGenerator *JG, int max_recursion,
                                   int max_vals_per_obj, unsigned int seed) {
  JG->max_recursion = max_recursion;
  JG->max_vals_per_obj = max_vals_per_obj;
  JG->out_json = NULL;

  _jsongen_prng_seed(&JG->_prng, seed);
}

void jsongen_seed_rng(struct JsonGenerator *JG, unsigned int seed) {
  _jsongen_prng_seed(&JG->_prng, seed);
}

void jsongen_make_root(struct JsonGenerator *JG) {
  int sz = 5000;
  JG->out_json = malloc(sz);
  memset(JG->out_json, 0, sz);
  _jsongen_obj(0, JG->max_vals_per_obj, JG, &JG->_prng, JG->out_json);
}

void jsongen_clean_output(struct JsonGenerator *JG) {
  if (JG->out_json != NULL) {
    free(JG->out_json);
  }
}

void jsongen_clean_up(struct JsonGenerator *JG) {
  jsongen_clean_output(JG);
}
