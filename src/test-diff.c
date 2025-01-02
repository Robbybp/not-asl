#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "expr.h"
#include "nl.h"
#include "sparse.h"
#include "op_derivs.h"
#include "forward_diff.h"
#include "reverse_diff.h"

const bool REVERSE = true;

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
  struct Node xnode = {VAR_NODE, xdata};
  struct Node ynode = {VAR_NODE, ydata};
  struct Node znode = {VAR_NODE, zdata};

  struct Variable variables[3] = {x, y, z};

  struct CSRMatrix deriv;

  struct Node expr1 = {.type=CONST_NODE, .data=5.4};
  if (REVERSE){deriv = reverse_diff_expression(expr1, nvar);}
  else        {deriv = forward_diff_expression(expr1, nvar);}
  print_csrmatrix(deriv);
  free_csrmatrix(deriv);

  struct Node expr2 = {VAR_NODE, xdata};
  to_string(buffer, bsize, expr2);
  printf("\nExpression: %s", buffer);
  if (REVERSE){deriv = reverse_diff_expression(expr2, nvar);}
  else        {deriv = forward_diff_expression(expr2, nvar);}
  print_csrmatrix(deriv);
  free_csrmatrix(deriv);

  expr2.data = zdata;
  if (REVERSE){deriv = reverse_diff_expression(expr2, nvar);}
  else        {deriv = forward_diff_expression(expr2, nvar);}
  to_string(buffer, bsize, expr2);
  printf("\nExpression: %s", buffer);
  print_csrmatrix(deriv);
  free_csrmatrix(deriv);

  struct Node expr3;
  expr3.type = OP_NODE;
  struct Node expr3args[2] = {xnode, znode};
  struct OperatorNode e3op = {
    .op = PRODUCT,
    .nargs = 2,
    .args = expr3args,
    .value = 0.0,
  };
  union NodeData e3data = {.expr = &e3op};
  expr3.data = e3data;
  to_string(buffer, bsize, expr3);
  printf("\nExpression: %s", buffer);
  if (REVERSE){deriv = reverse_diff_expression(expr3, nvar);}
  else        {deriv = forward_diff_expression(expr3, nvar);}
  print_csrmatrix(deriv);
  free_csrmatrix(deriv);

  struct Node expr4; expr4.type = OP_NODE;
  struct Node expr4args[2] = {expr3, znode};
  struct OperatorNode e4op = {PRODUCT, 2, expr4args, 0.0};
  union NodeData e4data = {.expr = &e4op};
  expr4.data = e4data;
  to_string(buffer, bsize, expr4);
  printf("\nExpression: %s", buffer);
  if (REVERSE){deriv = reverse_diff_expression(expr4, nvar);}
  else        {deriv = forward_diff_expression(expr4, nvar);}
  print_csrmatrix(deriv);
  free_csrmatrix(deriv);

  x.value = -7.5;
  printf("\nx <- -7.5\n");
  if (REVERSE){deriv = reverse_diff_expression(expr4, nvar);}
  else        {deriv = forward_diff_expression(expr4, nvar);}
  print_csrmatrix(deriv);
  free_csrmatrix(deriv);

  return 0;
}
