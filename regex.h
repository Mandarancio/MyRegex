#ifndef __DYN_REGEX_H__
#define __DYN_REGEX_H__

#include <stdlib.h>


typedef enum CardinalityType {
  ONE,
  ANY,
  LT,
  GT,
  EQ,
  INTO
} CardinalityType;

typedef enum ExpType {
  SEQUENCE,
  BIN_OP,
  NOT_OP,
  CHARRANGE,
  REGEX
} ExpType;

typedef enum OpType {
  OR,
  AND
} OpType;

typedef struct Cardinality {
  CardinalityType type;
  int             a;
  int             b;
} Cardinality;

typedef struct Regex {
  Cardinality    cardinality;
  void          *expression;
  ExpType        type;
} Regex;

typedef struct Sequence {
  Regex**        sequence;
  size_t         size;
} Sequence;

typedef struct BinaryOp  {
  Regex         *lt;
  Regex         *rt;
  OpType         type;
} BinaryOp;

typedef struct NotOp {
  Regex         *a;
} NotOp;

typedef struct CharRange {
  char           from;
  char           to;
} CharRange;

typedef enum Bool {
  FALSE,
  TRUE
} Bool;

Cardinality one      (void);
Cardinality any      (void);
Cardinality min      (size_t n);
Cardinality max      (size_t n);
Cardinality eq       (size_t n);
Cardinality into     (size_t gt,
                      size_t lt);

CharRange* range     (char        a,
                      char        b);
CharRange* range_mono(char        a);
CharRange* range_any (void);

Sequence*  sequence  (Regex      *seq[],
                      size_t      n);

Regex*     regex     (Cardinality c,
                      void       *exp,
                      ExpType     type);
Regex*     regex_char(Cardinality c,
                      char        a);
Regex*     regex_range(Cardinality c,
                      char        a,
                      char        b);
Regex*     regex_any (Cardinality c);
Regex*     regex_seq (Regex      *seq[],
                      size_t      n);
Regex*     regex_or  (Cardinality c,
                      Regex      *lt,
                      Regex      *rt);
Regex*     regex_and (Cardinality c,
                      Regex      *lt,
                      Regex      *rt);
Regex*     regex_not (Cardinality c,
                      Regex      *lt);

void   regex_free    (Regex      *r);
void   not_free      (NotOp      *op);
void   operator_free (BinaryOp   *op);
void   sequence_free (Sequence   *seq);
void   range_free    (CharRange  *range);

Bool   eval_range    (char        c,
                      CharRange  *range);


Bool   eval_sequence (const char *string,
                      size_t     *pos,
                      size_t      len,
                      Sequence   *sequence);

Bool   eval_operator (const char *strig,
                      size_t     *pos,
                      size_t      len,
                      BinaryOp   *op);

Bool   eval_not      (const char *string,
                      size_t     *pos,
                      size_t      len,
                      NotOp      *op);

Bool   eval          (const char *string,
                      size_t     *pos,
                      size_t      len,
                      Regex      *regex);

Regex* parse         (const char *str);
void   print_regex   (Regex      *reg,
                      const char *spacer);

#endif
