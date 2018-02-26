#include "regex.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

Bool
eval_range   (char        c,
              CharRange  *range)
{
  assert(range);
  Bool r = (range->from <= c && c <= range->to);
  /*printf("%c <= %c <= %c = %d\n", range->from, c, range->to, r);*/
  return r;
}

Bool
eval_sequence (const char *string,
               size_t     *pos,
               size_t      len,
               Sequence   *sequence)
{
  assert(sequence);
  size_t i;

  for (i = 0; i < sequence->size && i < len; i++) {
    /*printf(">> %lu\n", i+1);*/
    if (!eval(string, pos, len, sequence->sequence[i]))
      return FALSE;
  }

  return TRUE;
}

Bool
eval_operator (const char *string,
               size_t     *pos,
               size_t      len,
               BinaryOp   *op)
{
  assert(op);
  size_t p_l = *pos;
  size_t p_r = *pos;
  Bool res_l = eval(string, &p_l, len, op->lt);
  Bool res_r = eval(string, &p_r, len, op->rt);
  /*printf("%d (%lu) %s %d (%lu)\n", res_l, p_l, (op->type == AND? "and" : "or"), res_r, p_r);*/
  (*pos) = p_l < p_r ? p_l : p_r;
  if (op->type == AND)
    return res_l && res_r;
  else
    return res_l || res_r;
}

Bool
eval_not      (const char *string,
               size_t     *pos,
               size_t      len,
               NotOp      *op)
{
  return !(eval(string, pos, len, op->a));
}


Bool
eval_exp      (const char *string,
               size_t     *pos,
               size_t      len,
               Regex      *regex)
{
  switch (regex->type) {
  case CHARRANGE:
    return eval_range(string[(*pos) ++], regex->expression);
  case BIN_OP:
    return eval_operator(string, pos, len, regex->expression);
  case NOT_OP:
    return eval_not(string, pos, len, regex->expression);
  case SEQUENCE:
    return eval_sequence(string, pos, len, regex->expression);
  case REGEX:
    return eval(string, pos, len, regex->expression);
  }
  return FALSE;
}

Bool
eval          (const char *string,
               size_t     *pos,
               size_t      len,
               Regex      *regex)
{
  if (*pos > len)
    return FALSE;
  if (regex->cardinality.type == ONE) {
    return eval_exp(string, pos, len, regex);
  } else {
    size_t n = 0;
    while (eval_exp(string, pos, len, regex)) {
      n++;
      if ((regex->cardinality.type == LT || regex->cardinality.type == EQ) && n > regex->cardinality.a)
        return FALSE;
      if (regex->cardinality.type == INTO && n > regex->cardinality.b)
        return FALSE;
    }
    (*pos) --;
    switch (regex->cardinality.type) {
    case ANY:
      return TRUE;
    case LT:
      return n <= regex->cardinality.a;
    case GT:
      return n >= regex->cardinality.a;
    case EQ:
      return n == regex->cardinality.a;
    case INTO:
      return n >= regex->cardinality.a && n <= regex->cardinality.b;
    default:
      return n == 1;
    }
  }
  return TRUE;
}


CharRange*
range      (char        a,
            char        b)
{
  CharRange * r = malloc(sizeof *r);
  r->from = a;
  r->to = b;
  return r;
}

CharRange*
range_mono (char a)
{
  return range(a, a);
}

CharRange*
range_any  (void)
{
  return range(-128, 127);
}

Sequence*
sequence(Regex *seq[],
         size_t n)
{
  Sequence *s = malloc(sizeof *s);
  s->sequence = seq;
  s->size = n;
  return s;
}

Regex*
regex(Cardinality c,
      void       *exp,
      ExpType     type)
{
  Regex * r = malloc(sizeof *r);
  r->cardinality = c;
  r->type = type;
  r->expression = exp;
  return r;
}


Regex*
regex_char (Cardinality c,
            char a)
{
  return regex_range(c, a, a);
}

Regex*
regex_range(Cardinality c,
            char a,
            char b)
{
  CharRange * r = range(a, b);
  return regex(c, r, CHARRANGE);
}

Regex*
regex_any  (Cardinality c)
{
  return regex_range(c, -128, 127);
}

Regex*
regex_seq  (Regex *rs[],
            size_t n)
{
  Sequence * s =  sequence(rs, n);
  return regex(one(), s, SEQUENCE);
}

Cardinality
one      (void)
{
  return (Cardinality) {ONE, 1, 0};
}
Cardinality
any      (void)
{
  return (Cardinality) {ANY, 0, 0};
}

Cardinality
max       (size_t n)
{
  return (Cardinality) {LT, n, 0};
}

Cardinality
min       (size_t n)
{
  return (Cardinality) {GT, n, 0};
}

Cardinality
eq       (size_t n)
{
  return (Cardinality) {EQ, n, 0};
}

Cardinality
into     (size_t gt,
          size_t lt)
{
  return (Cardinality) {INTO, gt, lt};
}

Regex*
op(Cardinality c,
   Regex      *lt,
   Regex      *rt,
   OpType      type)
{
  BinaryOp * op = malloc(sizeof *op);
  op->type = type;
  op->lt = lt;
  op->rt = rt;
  return regex(c, op, BIN_OP);
}

Regex*
regex_or   (Cardinality c,
            Regex      *lt,
            Regex      *rt)
{
  return op(c, lt, rt, OR);
}

Regex*
regex_and  (Cardinality c,
            Regex      *lt,
            Regex      *rt)
{
  return op(c, lt, rt, AND);
}

Regex*
regex_not  (Cardinality c,
            Regex      *lt)
{
  NotOp * op = malloc(sizeof *op);
  op->a = lt;
  return regex(c, op, NOT_OP);
}


void
regex_free    (Regex      *r)
{
  if (!r)
    return;
  switch (r->type) {
  case REGEX:
    regex_free(r->expression);
    break;
  case SEQUENCE:
    sequence_free(r->expression);
    break;
  case BIN_OP:
    operator_free(r->expression);
    break;
  case NOT_OP:
    not_free(r->expression);
    break;
  case CHARRANGE:
    range_free(r->expression);
    break;
  }
  free(r);
  r = NULL;
}

void
not_free      (NotOp      *op)
{
  if (!op)
    return;
  regex_free(op->a);
  free(op);
  op = NULL;
}

void
operator_free (BinaryOp   *op)
{
  if (!op)
    return;
  regex_free(op->lt);
  regex_free(op->rt);
  free(op);
  op = NULL;
}

void
sequence_free (Sequence   *seq)
{
  if (!seq)
    return;
  size_t i;
  for (i = 0; i < seq->size; i++) {
    regex_free(seq->sequence[i]);
  }
  free(seq->sequence);
  free(seq);
  seq = NULL;
}

void
range_free    (CharRange  *range)
{
  if (!range)
    return;
  free(range);
  range = NULL;
}

Cardinality
parse_cardinality(const char *str,
                  size_t     *it)
{
  assert(it && str);
  size_t i = (*it);
  Cardinality cn = one();
  if (str[i] == '#') {
    cn = any();
    (*it) = i+1;
  } else if (str[i] == '>' ||
           str[i] == '<' ||
           str[i] == '=' ) {
    if (str[i+1] >= '0' && str[i+1] <= '9') {
      size_t si = i + 1;
      size_t ei = i + 2;
      while ('0' <= str[ei] && str[ei] <= '9') {
        ei ++;
      }
      char num[ei-si+1];
      num[ei-si] = 0;
      memcpy(num, str + si, ei - si);

      size_t n = atoi(num);
      if (str[i] == '>')
        cn = min(n);
      else if (str[i] == '<')
        cn = max(n);
      else
        cn = eq(n);
      i = ei;
    }
    (*it) = i;
  }
  return cn;
}

Bool
is_special(char c) {
  return c == ':' || c == '|' || c == '(' || c == '&' || c == '*' || c == '!' || c == ')';
}

char
next_char(const char *str, size_t *i)
{
  (*i) ++;
  if (str[*i] == '\\') {
    (*i) ++;
    return str[*i];
  } else
    return str[*i];
}
Regex*
simple_seq(const char * str,
           size_t       len)
{
  if (len == 0)
    return NULL;
  Regex **list = malloc(sizeof(Regex*) * len);
  size_t i;
  for (i = 0; i < len; i++) {
    list[i] = regex_char(one(), str[i]);
  }
  return regex_seq(list, len);
}

Regex* acc(Regex *r, Regex* prev) {
  assert(r);
  if (prev) {
    Regex **seq = malloc(sizeof(Regex*) * 2);
    seq[0] = prev;
    seq[1] = r;
    return regex_seq(seq, 2);
  }
  return r;
}

Regex*
parse_next    (const char *str,
               size_t     *it,
               size_t      len)
{

  Bool open = (str[*it] == '(');
  if (open) {
    (*it) ++;
  }

  Cardinality cn = parse_cardinality(str, it);


  Regex * prev = NULL;
  Bool sls = FALSE;
  char current[len];
  size_t j = 0;
  for (; *it < len; (*it)++) {
     if (sls) {
       current[j] = str[*it];
       j++;
       sls = FALSE;
     } else if (str[*it] == '\\') {
       sls = TRUE;
     } else if (str[*it] == '!') {
       if (j > 0) {
         Regex * r = simple_seq(current, j);
         prev = acc(r, prev);
         j = 0;
         current[0] = 0;
       }
       (*it) ++;
       Regex * next = parse_next(str, it, len);
       if (next)
         prev = acc(regex_not(one(), next), prev);
       else
         fprintf(stderr, "Not syntax wrong at %lu of %s\n", (*it) - 1, str);

     } else if (str[*it] == '|' || str[*it] == '&') {
       if (j > 0 || prev) {
         if (j > 0) {
           Regex * r = simple_seq(current, j);
           prev = acc(r, prev);
           j = 0;
           current[0] = 0;
         }
         char op = str[*it];
         (*it)++;
         if (op == '|')
           prev = regex_or(one(), prev, parse_next(str, it, len));
         else
           prev = regex_and(one(), prev, parse_next(str, it, len));
       } else
         return NULL;
     } else if (str[*it] == '(') {
       if (j > 0) {
         Regex * r = simple_seq(current, j);
         prev = acc(r, prev);
         j = 0;
         current[0] = 0;
       }

       Regex * next = parse_next(str, it, len);
       prev = acc(next, prev);
     } else if (str[*it] == ')') {
       if (!open)
         (*it) --;
       break;
     } else if (str[*it] == ':') {
       if (*it == len - 1 || j == 0 || is_special(str[(*it)+1])) {
         if (prev)
           regex_free(prev);
         fprintf(stderr, "Range not valid found at %lu of %s\n", *it, str);
         return NULL;
       }
       char a = current[j-1];
       char b = next_char(str, it);
       if (j > 1) {
         Regex * r = simple_seq(current, j-1);
         prev = acc(r, prev);
       }
       j = 0;
       current[0] = 0;
       prev = acc(regex_range(one(), a, b), prev);
     } else if (str[*it] == '*') {
       if (j) {
         Regex * r = simple_seq(current, j);
         prev = acc(r, prev);
       }
       j = 0;
       current[0] = 0;
       prev = acc(regex_range(one(), -128, 127), prev);
     }else {
       current[j] = str[*it];
       j ++;
     }
  }
  if (j > 0) {
    Regex * r = simple_seq(current, j);
    prev = acc(r, prev);
  }
  if (prev) {
    if (cn.type == ONE)
      return prev;
    return regex(cn, prev, REGEX);
  }
  fprintf(stderr, "No regex found at %lu of %s\n", *it, str);
  return NULL;
}

Regex*
parse         (const char *str) {
  size_t it = 0;
  size_t len = strlen(str);
  return parse_next(str, &it, len);
}

void
print_seq     (Sequence   *seq,
               const char *sp)
{
  size_t i;
  for (i = 0; i < seq->size; i++) {
    print_regex(seq->sequence[i], sp);
  }
}

void
print_regex   (Regex      *reg,
               const char *spacer)
{
  if (reg->type != CHARRANGE) {
    printf("%s + (", spacer);
    switch (reg->cardinality.type) {
    case ONE:
      printf("1) ");
      break;
    case ANY:
      printf("#) ");
      break;
    case EQ:
      printf("%d) ", reg->cardinality.a);
      break;
    case LT:
      printf("<%d) ", reg->cardinality.a);
      break;
    case GT:
      printf(">%d) ", reg->cardinality.a);
      break;
    case INTO:
      printf("%d<x<%d) ", reg->cardinality.a, reg->cardinality.b);
      break;
    }
    size_t l = strlen(spacer);
    char s[l+2];
    s[l+1] = 0;
    memcpy(s, spacer, l);
    s[l] = ' ';
    switch (reg->type) {
    case REGEX:
      printf("group: \n");
      print_regex(reg->expression, s);
      break;
    case NOT_OP:
      printf("not: \n");
      print_regex(((NotOp*)reg->expression)->a, s);
      break;
    case BIN_OP:
      printf("%s:\n",(((BinaryOp*)reg->expression)->type == AND ? "and" : "or"));
      print_regex(((BinaryOp*)reg->expression)->lt, s);
      print_regex(((BinaryOp*)reg->expression)->rt, s);
      break;
    case SEQUENCE:
      printf("sequence:\n");
      print_seq(reg->expression, s);
    default:
      break;
    }
  } else {
    CharRange * r = reg->expression;
    if (r->from == r->to)
      printf("%s - char %c\n", spacer, r->from);
    else if (r->from == -128 && r->to == 127)
      printf("%s - any *\n", spacer);
    else
      printf("%s - range (%c - %c)\n", spacer, r->from, r->to);
  }
}
