#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "expr.h"
#include "nl.h"
#include "sparse.h"
#include "ad.h"

int main(int narg, char ** argv){
  const int nvar = 3;
  const int bsize = 80;
  char buffer[bsize];

  struct Variable x = {0, 1.1};
  struct Variable y = {1, -2.2};
  struct Variable z = {2, 3.3};
  union NodeData xdata = {.var = &x};
  union NodeData ydata = {.var = &y};
  union NodeData zdata = {.var = &z};

  struct Variable variables[3] = {x, y, z};

  int ret;
  double values[nvar] = {0.0, 0.0, 0.0};
  struct VarListNode * wrt = NULL;

  // Recall: A `Node` contains the node type and its data.
  struct Node expr = {CONST_NODE, 5.4};
  ret = differentiate(expr, wrt, values, nvar);
  to_string(buffer, bsize, expr);
  printf("Expression: %s\n", buffer);
  printf("Derivative: [ ");
  for (int i = 0; i < nvar; i++){printf("%f, ", values[i]);}
  printf("]\n\n");

  struct Node expr2 = {VAR_NODE, xdata};
  struct VarListNode wrt2 = {NULL, &z};
  struct VarListNode wrt3 = {&wrt2, &y};
  struct VarListNode wrt4 = {&wrt3, &x};
  // TODO: Combine identify_variables and differentiate into a single function?
  // What is the output? A CSRMatrix?

  ret = differentiate(expr2, &wrt4, values, nvar);
  to_string(buffer, bsize, expr2);
  printf("Expression: %s\n", buffer);
  printf("Derivative: [ ");
  for (int i = 0; i < nvar; i++){printf("%f, ", values[i]);}
  printf("]\n\n");

  return 0;
}
