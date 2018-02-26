# Regex For Me

I never fully understood existing `regex` syntax and rules, so I chose to implements mine!

It is possible to generate regex rules for string or programmatically.

## Regex Syntax


Some syntax:

 * `a:z` (**Range**) return `TRUE` if the char is between `a` and `z` included
 * `*` (**All**) return `TRUE` for any char
 * `abc` (**Sequence**) return `TRUE` if the sequence `abc` exist in the string at the iterator position
 * `R1|R2` (**Or**) return `TRUE` if one of the 2 rules is `TRUE`
 * `R1&R2` (**And**) return `TRUE` if both of the 2 rules are `TRUE`
 * `#R1` (**Any**) return `TRUE` always and iterate until the rule is `TRUE`
 * `=NR1`  (**Exact**) return `TRUE` if the rule is `TRUE` exactly `N` times
 * `>NR1`  (**Minimum**)  return `TRUE` if the rule is `TRUE` at least `N` times
 * `<NR1`  (**Maximum**) return `TRUE` if the rule is `TRUE` maximum `N` times
 * `(..)` (**Nested Rule**) return `TRUE` if the rule inside the parenthesis is `TRUE`
 * `\` (**Special chars**) special chars such as `|` or `&` or `\` or any other can be used in rules by adding `\` before.

## Example:

A simple regex to find markdown links is `[(#!])]\((>1!\))\)`. To compile the rule:

```c
 Regex * r = parse("[(#!])]\\((>1!\\)))");
```


A more complex regex to find markdown tables is `\|(>1!\|)\|(#!\n)\n\|(\:|-)(>1-&!((\:|-)\|)\|(#!\n)\n` the code is the following:

```c
 const char *str = "| A | B |\n"
                   "|:-:|--:|\n";

 size_t n = strlen(str);
 Regex * r = parse("\\|(>1!\\|)\\|(#!\n)\n"
                   "\\|(\\:|-)(>1-&!((\\:|-)\\|))(\\:|-)\\|(#!\n)\n");
 if (!r) {
   fprintf(stderr, "Parse error!");
   return -1;
 }

 size_t i = 0;
 Bool res = eval(str, &i, n, r);
 printf("Res: %s, at %c (%lu)\n", (res ? "true":"false"), str[(i-1)], i);
 regex_free(r);
```

The result of such code is

```
Res: true, at
 (20)
```
