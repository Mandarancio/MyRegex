
#include <stdio.h>
#include <regex.h>
#include <string.h>

int
main (int argc, char **argv)
{
  const char *str = "| A | B |\n"
                    "|:-:|--:|\n";

  size_t n = strlen(str);
  printf("<%s> (%lu)\n", str, n);
  /* \|(>1!\|)\|(#!\n)\n */
  Regex * r = parse("\\|(>1!\\|)\\|(#!\n)\n"
                    "\\|(\\:|-)(>1-&!((\\:|-)\\|))(\\:|-)\\|(#!\n)\n");
  if (!r) {
    fprintf(stderr, "Parse error!");
    return -1;
  }
  print_regex(r, " ");
  size_t i = 0;
  Bool res = eval(str, &i, n, r);
  printf("Res: %s, at %c (%lu)\n", (res ? "true":"false"), str[(i-1)], i);
  regex_free(r);
  return 0;
}
