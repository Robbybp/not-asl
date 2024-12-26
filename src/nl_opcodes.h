/*
 * From Table 6 in "Writing .nl files"
 * Many indices appear to not correspond to anything, or correspond to operators
 * we don't support.
 */
int OP_LOOKUP[56] = {
  SUM,
  SUBTRACTION,
  PRODUCT,
  DIVISION,
  -1, // "rem". Remainder, presumably
  POWER,
  -1,
  -1,
  -1,
  -1,
  -1, // 10
  -1,
  -1,
  -1,
  -1,
  -1, // 15
  NEG,
  -1,
  -1,
  -1,
  -1, // 20
  -1,
  -1,
  -1,
  -1,
  -1, // 25
  -1,
  -1,
  -1,
  -1,
  -1, // 30
  -1,
  -1,
  -1,
  -1,
  -1, // 35
  -1,
  -1,
  TAN,
  SQRT,
  -1, // 40
  SIN,
  -1,
  LOG,
  EXP,
  -1, // 45
  COS,
  -1,
  -1,
  -1,
  -1, // 50
  -1,
  -1,
  -1,
  -1,
  -1, // 55
};
