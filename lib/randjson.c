/*
 * randjson: 2023-11-29, by Lemuria
 *
 * randjson is a simple C-language library to generate random JSON strings.
 * There is likely no real-world use for this JSON string generator, except
 * for use in fuzz-testing JSON parsers or just satiating boredom. It is also
 * written in ANSI C for maximum compatibility.
 *
 *
 * MIT License
 *
 * Copyright (c) 2023 Lemuria
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <parson/parson.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "randjson.h"

/******************************************************************************
 * PREPROCESSOR DEFINES
 *****************************************************************************/
/*** DEFAULT VALUES ***/
/* Maximum recursion level. */
#define MAX_RECURSION 5
/* Maximum size of both keys and values. */
#define MAX_STRING_LEN 8
/* Maximum array length. */
#define MAX_ARRAY_LEN 9
/* Maximum number of keys per object. */
#define MAX_KEYS 8

/*** TYPES ***/
#define NUMBER 1
#define STRING 2
#define ARRAY 3
#define OBJECT 4
#define JSON_NULL 5

/*** JSON VALUE TYPE ARRAY ***/
static int _json_value_types[] = {NUMBER, STRING, ARRAY, OBJECT, JSON_NULL};
#define ARRAY_SIZE(x) ((sizeof x) / (sizeof *x))
#define _TYPES_COUNT ARRAY_SIZE(_json_value_types)

/******************************************************************************
 * FORWARD DECLARATIONS
 *****************************************************************************/

/*
 * Quick and dirty internal RNG object.
 */
typedef struct Prng {
  unsigned int x, y;
} Prng;

static void _randjson_array(JSON_Array *out, Prng *p, JsonGenerator *JG,
                            int recursion_level);
static void _randjson_object(JSON_Object *obj, Prng *p, JsonGenerator *JG,
                             int recursion_level);

/******************************************************************************
 * UTILITY FUNCTIONS
 *****************************************************************************/

/* Reseed the PRNG. */
static void _prng_seed(Prng *p, unsigned int seed) {
  p->x = seed | 1;
  p->y = seed;
}

/* Extract a random number. */
static unsigned int _prng_int(Prng *p) {
  p->x = (p->x >> 1) ^ ((1 + ~(p->x & 1)) & 0xd0000001);
  p->y = p->y * 1103515245 + 12345;
  return p->x ^ p->y;
}

static int _recursion_too_deep(int level) {}

static void _die(int i, char* reason)
{
  fprintf(stderr, "randjson (error): %s", reason);
  exit(i);
}

static void _validate_jg(JsonGenerator *JG)
{
  if (JG->max_keys == 0) {
    _die(1, "Maximum keys cannot be zero");
  }
}

/******************************************************************************
 * GENERATOR FUNCTIONS
 *****************************************************************************/

/*
 * Note that generator functions, when creating JSON objects, will not have to
 * call json_value_free(), as that can simply be done by randjson_make_json()
 * on the root object. Parson, the library this library depends on, will do
 * the heavy-lifting of freeing the entire JSON object.
 */

/* String of possible characters to use when generating random strings. */
static const char string_table[87] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                     "abcdefghijklmnopqrstuvwxyz"
                                     "0123456789+/ ,.:;-_!@#$^&*"
                                     "()){}'`~\"";

/* Generate a random string for use as a key or value. */
static void _randjson_string(size_t len, char *out, Prng *p) {
  size_t i;
  /* Add 1 to len to prevent empty keys or strings. */
  for (i = 0; i < (len + 1); i++) {
    out[i] = string_table[_prng_int(p) % 65];
  }
}

/* Generate a random number and set it to a key */
static void _randjson_number(JSON_Object *obj, Prng *p, char *k) {
  json_object_set_number(obj, k, (double)_prng_int(p));
}

/* Generate a random object */
static void _randjson_object(JSON_Object *obj, Prng *p, JsonGenerator *JG,
                             int recursion_level) {

  char *k = calloc(JG->max_string_len + 1, sizeof(char));
  char *v = calloc(JG->max_string_len + 1, sizeof(char));

  int i;
  int keys = _prng_int(p) % JG->max_keys;
  for (i = 0; i < keys; i++) {
    _randjson_string(_prng_int(p) % JG->max_string_len, k, p);
    int type = _json_value_types[_prng_int(p) % _TYPES_COUNT];
    switch (type) {
    case NUMBER:
      _randjson_number(obj, p, k);
      break;
    case STRING:
      _randjson_string((_prng_int(p) % JG->max_string_len), v, p);
      json_object_dotset_string(obj, k, v);
      break;
    /*
     * The semicolon at the end is an empty statement, as in ANSI C,
     * declarations do not count as statements.
     */
    case ARRAY:;
      if (recursion_level > JG->max_recursion) {
        break;
      }
      JSON_Value *arr_v = json_value_init_array();
      JSON_Array *arr = json_value_get_array(arr_v);
      _randjson_array(arr, p, JG, recursion_level + 1);
      json_object_set_value(obj, k, arr_v);
      break;
    case JSON_NULL:
      json_object_set_null(obj, k);
      break;
    case OBJECT:
      /*
       * The deeper we are, the less likely it should be that we decide to go
       * with an object. Also reduce i by 1 since we didn't generate something
       * this time.
       */
      if (_prng_int(p) % (recursion_level + 1 * 50) < 200) {
        i--;
        break;
      }

      if (recursion_level < JG->max_recursion) {
        JSON_Value *sub_v = json_value_init_object();
        JSON_Object *sub_obj = json_value_get_object(sub_v);
        _randjson_object(sub_obj, p, JG, recursion_level + 1);
        json_object_set_value(obj, k, sub_v);
      }
      break;
    }
  }

  free(k);
  free(v);
}

/* Generate a random array */
static void _randjson_array(JSON_Array *out, Prng *p, JsonGenerator *JG,
                            int recursion_level) {
  char *str_v = calloc(JG->max_string_len + 1, sizeof(char));
  int items = _prng_int(p) % JG->max_array_len;
  int i;
  for (i = 0; i < items; i++) {
    int type = _json_value_types[_prng_int(p) % _TYPES_COUNT];
    switch (type) {
    case NUMBER:
      json_array_append_number(out, (double)_prng_int(p));
      break;
    case STRING:
      _randjson_string(_prng_int(p) % JG->max_string_len, str_v, p);
      json_array_append_string(out, str_v);
      break;
    case JSON_NULL:
      json_array_append_null(out);
      break;
    /*
     * The semicolon at the end is an empty statement, as in ANSI C,
     * declarations do not count as statements.
     */
    case ARRAY:;
      if (recursion_level > JG->max_recursion) {
        break;
      }
      JSON_Value *arr_v = json_value_init_array();
      JSON_Array *arr = json_value_get_array(arr_v);
      _randjson_array(arr, p, JG, recursion_level + 1);
      json_array_append_value(out, arr_v);
      break;
    case OBJECT:
      /*
       * The deeper we are, the less likely it should be that we decide to go
       * with an object. Also reduce i by 1 since we didn't generate something
       * this time.
       */
      if (_prng_int(p) % ((recursion_level + 1) * 50) > 250) {
        i--;
        break;
      }

      if (recursion_level < JG->max_recursion) {
        JSON_Value *sub_v = json_value_init_object();
        JSON_Object *sub_obj = json_value_get_object(sub_v);
        _randjson_object(sub_obj, p, JG, recursion_level + 1);
        json_array_append_value(out, sub_v);
      }
      break;
    }
  }
  free(str_v);
}

/******************************************************************************
 * PUBLIC API
 *****************************************************************************/

/*
 * Generate a random JSON string. You must free() the string that this function
 * returns.
 */
char *randjson_make_json(unsigned int seed, JsonGenerator *JG) {
  Prng p;
  _prng_seed(&p, seed);

  JSON_Value *root_v = json_value_init_object();
  JSON_Object *root_obj = json_value_get_object(root_v);

  _validate_jg(JG);

  _randjson_object(root_obj, &p, JG, 0);

  char *serialized_out = NULL;
  serialized_out = json_serialize_to_string(root_v);

  json_value_free(root_v);

  return serialized_out;
}

/*
 * Set defaults for the JsonGenerator.
 */
void randjson_set_default(JsonGenerator *JG) {
  JG->max_array_len = MAX_ARRAY_LEN;
  JG->max_recursion = MAX_RECURSION;
  JG->max_string_len = MAX_STRING_LEN;
  JG->max_keys = MAX_KEYS;
}
