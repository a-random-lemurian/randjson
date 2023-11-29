#ifndef JSONGEN_H
#define JSONGEN_H


struct _jsongen_Prng {
  unsigned int x, y;
};

struct JsonGenerator
{
    int max_recursion;      // Maximum depth of JSON
    int max_vals_per_obj;   // Maixmum keys per JSON object
    char *out_json;         // String containing output JSON

    struct _jsongen_Prng _prng;             // RNG object
};

void jsongen_prepare_JsonGenerator(struct JsonGenerator *JG, int max_recursion,
                                   int max_vals_per_obj, unsigned int seed);
void jsongen_seed_rng(struct JsonGenerator *JG, unsigned int seed);
void jsongen_make_root(struct JsonGenerator *JG);
void jsongen_clean_output(struct JsonGenerator *JG);
void jsongen_clean_up(struct JsonGenerator *JG);
#endif /* JSONGEN_H */
