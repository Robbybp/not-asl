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

  struct CSRMatrix deriv;

  struct Node expr1 = {.type=CONST_NODE, .data=5.4};
  deriv = differentiate_expression(expr1, nvar);
  print_csrmatrix(deriv);
  free_csrmatrix(deriv);

  struct Node expr2 = {VAR_NODE, xdata};
  deriv = differentiate_expression(expr2, nvar);
  print_csrmatrix(deriv);
  free_csrmatrix(deriv);

  expr2.data = zdata;
  deriv = differentiate_expression(expr2, nvar);
  to_string(buffer, bsize, expr2);
  printf("\nExpression: %s", buffer);
  print_csrmatrix(deriv);
  free_csrmatrix(deriv);

  return 0;
}
