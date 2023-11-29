#include "jsongen.h"
#include <stdio.h>
#include <time.h>

int main() {
  struct JsonGenerator JG;
  struct _jsongen_Prng p;
  jsongen_prepare_JsonGenerator(&JG, 3, 10, clock());

  for (int i = 0; i < 5; i++) {
  jsongen_make_root(&JG);
  printf("%s\n", JG.out_json);
  jsongen_clean_up(&JG);
  }
  return 0;
}
