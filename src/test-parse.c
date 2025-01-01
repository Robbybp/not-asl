#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "expr.h"
#include "nl.h"
#include "sparse.h"
#include "forward_diff.h"
#include "reverse_diff.h"

const bool REVERSE = true;

int test(int narg, char ** argv){
  // Index is the right choice here, because variables will want to store bounds
  // somewhere.
  // (Although the variables could store their own bounds...)
  struct Variable x = {0, 1.1};
  struct Variable y = {1, -2.2};
  struct Variable z = {2, 3.3};
  double p = 5.43;

  union NodeData xdata = {.var = &x};
  union NodeData ydata = {.var = &y};
  union NodeData zdata = {.var = &z};
  union NodeData pdata = {.value = p};

  struct Node xnode = {VAR_NODE, xdata};
  struct Node ynode = {VAR_NODE, ydata};
  struct Node znode = {VAR_NODE, zdata};
  struct Node pnode = {CONST_NODE, pdata};

  struct Node expr1_args[4] = {xnode, ynode, znode, pnode};
  struct OperatorNode expr1 = {SUM, 4, expr1_args};
  union NodeData e1data = {.expr = &expr1};
  struct Node e1node = {OP_NODE, e1data};

  struct Node expr2_args[2] = {xnode, znode};
  struct OperatorNode expr2 = {PRODUCT, 2, expr2_args};
  union NodeData e2data = {.expr = &expr2};
  struct Node e2node = {OP_NODE, e2data};

  struct Node expr3_args[2] = {e1node, e2node};
  struct OperatorNode expr3 = {SUBTRACTION, 2, expr3_args};
  union NodeData e3data = {.expr = &expr3};
  struct Node e3node = {OP_NODE, e3data};

  struct Node expr4_args[2] = {pnode, e3node};
  struct OperatorNode expr4 = {DIVISION, 2, expr4_args};
  union NodeData e4data = {.expr = &expr4};
  struct Node e4node = {OP_NODE, e4data};

  union NodeData expnode_data = {.value = 10.5};
  struct Node expnode = {CONST_NODE, expnode_data};
  struct Node expr5_args[2] = {znode, expnode};
  struct OperatorNode expr5 = {POWER, 2, expr5_args};
  union NodeData e5data = {.expr = &expr5};
  struct Node e5node = {OP_NODE, e5data};

  // TODO: Verify that this e1node is at a different memory location
  // than other "instances" of e1node.
  struct Node expr6_args[2] = {e2node, e1node};
  struct OperatorNode expr6 = {POWER, 2, expr6_args};
  union NodeData e6data = {.expr = &expr6};
  struct Node e6node = {OP_NODE, e6data};

  struct Node expr7_args[2] = {e5node, e6node};
  struct OperatorNode expr7 = {DIVISION, 2, expr7_args};
  union NodeData e7data = {.expr = &expr7};
  struct Node e7node = {OP_NODE, e7data};

  printf("Testing evaluation functionality\n");

  printf("x: %1.2f\n", evaluate(xnode));
  printf("y: %1.2f\n", evaluate(ynode));
  printf("z: %1.2f\n", evaluate(znode));
  printf("p: %1.2f\n", evaluate(pnode));

  double e1_value = evaluate(e1node);
  double e2_value = evaluate(e2node);
  printf("expr1: v%d %s v%d = %1.2f\n", expr1.args[0].data.var->index, OPERATOR_DATA[expr1.op].opstring, expr1.args[1].data.var->index, e1_value);
  printf("expr2: v%d %s v%d = %1.2f\n", expr2.args[0].data.var->index, OPERATOR_DATA[expr2.op].opstring, expr2.args[1].data.var->index, e2_value);

  double e4_value = evaluate(e4node);
  printf("expr4: = %1.2f\n", e4_value);

  printf("Testing expression/node printing functionality\n");

  char p_str[8];
  to_string(p_str, 8, pnode);
  printf("p = %s\n", p_str);

  char x_str[8];
  char z_str[8];
  to_string(x_str, 8, xnode);
  to_string(z_str, 8, znode);
  printf("x = %s = %1.2f\n", x_str, evaluate(xnode));
  printf("z = %s = %1.2f\n", z_str, evaluate(znode));

  char e1_str[81];
  char e2_str[81];
  char e3_str[81];
  char e4_str[161];
  to_string(e1_str, 81, e1node);
  to_string(e2_str, 81, e2node);
  to_string(e3_str, 81, e3node);
  to_string(e4_str, 81, e4node);
  // How valuable would it be to have a version of to_string that does its
  // own memory allocation?
  printf("e1 = %s = %1.2f\n", e1_str, evaluate(e1node));
  printf("e2 = %s = %1.2f\n", e2_str, evaluate(e2node));
  printf("e3 = %s = %1.2f\n", e3_str, evaluate(e3node));
  printf("e4 = %s = %1.2f\n", e4_str, evaluate(e4node));

  // This is quite reasonable so far
  // - Can evaluate and print binary operators
  // - And potentially custom variable names.
  //   This is probably not a priority for now
  // - Then my priority is to read from nl files
  // - It would be nice to construct expressions via operator overloading
  //   But this is not supported by C. This is, I suppose, one of the
  //   "killer features" of C++ that would make it worth adopting.
  //   This is when we cross the line between a "solver" or "solver library"
  //   to a modeling environment.

  char e5_str[81];
  char e6_str[81];
  char e7_str[81];
  to_string(e5_str, 81, e5node);
  to_string(e6_str, 81, e6node);
  to_string(e7_str, 81, e7node);
  printf("e5 = %s = %1.2f\n", e5_str, evaluate(e5node));
  printf("e6 = %s = %1.2f\n", e6_str, evaluate(e6node));
  printf("e7 = %s = %1.2f\n", e7_str, evaluate(e7node));

  return 0;
}

/* I think I have a reasonable node/expression data type.
 *
 * Now I can work on reading from an nl file. And making whatever modifications
 * to the expression data structures are necessary to support this.
 *
 * Separate the current main routine into a test routine. Ideally, that can be
 * run with `make test`. I don't even have an alternative main routine yet.
 * I can add a makefile to compile and run different programs when I have a
 * working routine that reads an nl file.
 *
 * What do I need to do to read an nl file?
 * 1) Accept the file name as an argument
 * 2) Open the file (And make sure it exists)
 * 3) Do something with the contents
 *
 * What data structures do I want after reading the nl file?
 * - Array of variables, initialized with proper values
 * - Array of constant nodes? Although this doesn't really make sense as
 *   these aren't stored as pointers...
 * - Array of constraint expressions
 * - Objective function expression (and sense)?
 * This is an incomplete description of the model. To make this more complete,
 * I need:
 * - Variable bounds
 * - Constraint bounds
 *
 */

int main(int narg, char ** argv){
  // test(narg, argv);
  printf("Running executable %s\n", argv[0]);
  if (narg >= 2){
    printf(".nl file: %s\n", argv[1]);
  }else{
    printf("No file provided. Please provide an nl file.\n");
    return -1;
  }

  FILE * fp1 = fopen(argv[1], "r");
  struct NLHeader header = read_nl_header(fp1);
  fclose(fp1);

  FILE * fp2 = fopen(argv[1], "r");
  // TODO: arrays for variable and constraint bounds as well
  struct Variable * variables = malloc(header.nvar * sizeof(struct Variable));
  // Initialize index of variables, so we can distinguish them.
  for (int i=0; i<header.nvar; i++){variables[i].index = i;}
  read_nl_variables(fp2, variables, header.nvar);
  fclose(fp2);
  // Initialize values of variables so we can tell them apart
  for (int i=0; i<header.nvar; i++){variables[i].value = 1.0 + (i+1) / 10.0;}
  // Set this variable to -1 to test what happens when we get nan
  //variables[0].value = -1.0;
  // Set this variable to 800 to test what happens when exp causes an overflow
  //variables[0].value = 800.0;

  FILE * fp3 = fopen(argv[1], "r");
  struct Node * constraint_expressions = malloc(header.ncon * sizeof(struct Node));
  read_nl_constraints(fp3, constraint_expressions, header.ncon, variables, header.nvar);
  fclose(fp3);

  int nvar = header.nvar;
  int ncon = header.ncon;

  printf("%s is binary formatted: %s\n", argv[1], header.binary ? "true" : "false");
  printf("%s has %d variables\n", argv[1], nvar);
  printf("%s has %d constraints\n", argv[1], ncon);

  for (int i = 0; i < nvar; i++){
    printf("Variable %2d: value = %f\n", i, variables[i].value);
  }

  for (int i = 0; i < ncon; i++){
    char con_str[82];
    to_string(con_str, 82, constraint_expressions[i]);
    printf("Constraint %2d: body = %s\n", i, con_str);
  }
  for (int i = 0; i < ncon; i++){
    double value = evaluate(constraint_expressions[i]);
    printf("Constraint %2d: value = %f\n", i, value);
  }

  // Identify the variables that participate in each expression
  int * in_expr_lookup = malloc(nvar * sizeof(int));
  // Initialize to -1, i.e. the var has not appeared anywhere yet.
  for (int i=0; i<nvar; i++){in_expr_lookup[i] = -1;}
  struct VarListNode ** varlists = malloc(ncon * sizeof(struct VarListNode *));
  // Initialize to NULL, i.e. an empty varlist.
  for (int i=0; i<ncon; i++){varlists[i] = NULL;}
  int * nvar_in_con = malloc(ncon * sizeof(int));

  //union NodeData nodedata = {.var=&variables[0]};
  //struct Node expr = {VAR_NODE, nodedata};
  //int nvar_in_con = identify_variables(expr, 0, in_expr_lookup, nvar, &varlists[0]);

  // Populate lists of variables per constraint.
  int jac_nnz = 0;
  for (int i=0; i<ncon; i++){
    nvar_in_con[i] = identify_variables(
      constraint_expressions[i],
      i,
      in_expr_lookup,
      nvar,
      &varlists[i]
    );
    jac_nnz += nvar_in_con[i];
  }

  for (int i=0; i<ncon; i++){
    struct VarListNode * node = varlists[i];
    printf("Constraint %d contains %d variable(s):", i, nvar_in_con[i]);
    while(node){
      printf(" v%d,", node->variable->index);
      node = node->next;
    }
    printf("\n");
  }

  for (int i=0; i<ncon; i++){
    struct CSRMatrix deriv;
    if (REVERSE){
      deriv = reverse_diff_expression(constraint_expressions[i], nvar);
    }else{
      deriv = forward_diff_expression(constraint_expressions[i], nvar);
    }
    printf("Constraint %d derivative:", i);
    print_csrmatrix(deriv);
    printf("\n");
  }

  return 0;

  // Construct indptr and indices for CSR matrix
  // indptr has length nrow+1
  int jac_nrow = ncon;
  int jac_ncol = nvar;
  int * jac_indptr = malloc((jac_nrow+1) * sizeof(int));
  int * jac_indices = malloc(jac_nnz * sizeof(int));
  double * jac_values = malloc(jac_nnz * sizeof(double));
  int k = 0; // k is the nnz index
  for (int i=0; i<jac_nrow; i++){
    struct VarListNode * varnode = varlists[i];
    jac_indptr[i] = k; // indptr[i] stores the index of the first nz in row i
    while (varnode){
      // Here we are making an important assumption: That variables know their indices
      int j = varnode->variable->index;

      jac_indices[k] = j;
      jac_values[k] = 0.0; // Initialize nz value to zero

      // We update the nnz index *after* setting the column index and nz value
      k += 1;
      varnode = varnode->next;
    }
  }
  // Add the last row-index bound
  jac_indptr[ncon] = k;

  struct CSRMatrix jacobian = {
    jac_nnz,
    jac_nrow,
    jac_ncol,
    jac_indptr,
    jac_indices,
    jac_values,
  };

  print_csrmatrix(jacobian);

  // Converting varlists[0] into an array
  struct Variable * vararray = malloc(sizeof(struct Variable) * nvar_in_con[0]);
  struct VarListNode * tempnode = varlists[0];
  for (int i=0; i<nvar_in_con[0]; i++){
    vararray[i] = *(tempnode->variable);
    tempnode = tempnode->next;
  }

  // Free linked lists of variables
  for (int i=0; i<ncon; i++){
    free_varlist(&varlists[i]);
  }

  free(in_expr_lookup);
  free(varlists);
  free(nvar_in_con);

  for (int i = 0; i < header.ncon; i++){
    // Free memory used by each constraint expression
    free_expression(constraint_expressions[i]);
  }
  // Free the arrays of constraint expression head nodes and variables
  free(constraint_expressions);
  free(variables);

  return 0;
}
