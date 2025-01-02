#include <assert.h>

/*
 * Differentiate expression using reverse mode differentiation. Results
 * are returned in a CSR matrix. The number of variables is required
 * so we know how many columns to give the returned matrix.
 */
struct CSRMatrix reverse_diff_expression(struct Node expr, int nvar);

/*
 * Differentiate expression with respect to provided variables. Store
 * the derivative values in `values`. The variable's position in
 * `wrt` corresponds to its derivative's position in `values`.
 */
// TODO: Should this be called reverse_diff_inplace?
int reverse_diff(struct Node expr, int nnz, int * wrt, double * values);

int _reverse_diff_constant(struct Node expr, int nnz, int * wrt, double * values);
int _reverse_diff_variable(struct Node expr, int nnz, int * wrt, double * values);
int _reverse_diff_operator(struct Node expr, int nnz, int * wrt, double * values);

// When evaluating local derivatives, there's no difference between
// forward and reverse.
// These methods currently evaluate every node from scratch, which is
// very inefficient.
int (* REVERSE_DIFF_OP[N_OPERATORS])(struct Node *, int, double *) = {
  _forward_diff_sum,
  _forward_diff_product,
  _forward_diff_subtraction,
  _forward_diff_division,
  _forward_diff_power,
  _forward_diff_neg,
  _forward_diff_sqrt,
  _forward_diff_exp,
  _forward_diff_log,
  _forward_diff_sin,
  _forward_diff_cos,
  _forward_diff_tan,
};

struct CSRMatrix reverse_diff_expression(struct Node expr, int nvar){
  int nrow = 1;
  int eidx = 0;

  int * in_expr = malloc(sizeof(int) * nvar);
  for (int i=0; i<nvar; i++){ in_expr[i] = -1; }
  struct VarListNode * varlist = NULL;
  int nnz = identify_variables(expr, eidx, in_expr, nvar, &varlist);
  free(in_expr);

  // We'll use an array of int to indicate the indices of variables
  // that appear in this constraint.
  int * wrt = malloc(sizeof(int) * nnz);
  double * deriv_values = malloc(sizeof(double) * nnz);
  struct VarListNode * tmp = varlist;
  for (int i=0; i<nnz; i++){
    wrt[i] = tmp->variable->index;
    tmp = tmp->next;
    deriv_values[i] = 0.0;
  }
  // Make sure we have reached the end of our linked list.
  assert(tmp == NULL);
  free_varlist(&varlist);

  // Set adjoint to 1 for the root node and differentiate down to the leaves.
  expr.adjoint = 1.0;
  reverse_diff(expr, nnz, wrt, deriv_values);

  int * indptr = malloc(sizeof(int) * 2);
  indptr[0] = 0;
  indptr[1] = nnz;

  struct CSRMatrix deriv_matrix = {
    .nnz = nnz,
    .nrow = 1,
    .ncol = nvar,
    .indptr = indptr,
    .indices = wrt,
    .values = deriv_values,
  };
  return deriv_matrix;
}

int reverse_diff(struct Node expr, int nnz, int * wrt, double * values){
  switch(expr.type){
    case CONST_NODE:
      return _reverse_diff_constant(expr, nnz, wrt, values);
    case VAR_NODE:
      return _reverse_diff_variable(expr, nnz, wrt, values);
    case OP_NODE:
      return _reverse_diff_operator(expr, nnz, wrt, values);
  }
}

int _reverse_diff_constant(struct Node expr, int nnz, int * wrt, double * values){
  return 0;
}

int _reverse_diff_variable(struct Node expr, int nnz, int * wrt, double * values){
  int idx = expr.data.var->index;
  for (int i=0; i<nnz; i++){
    if (wrt[i] == idx){
      values[i] += expr.adjoint;
    }
  }
  return 0;
}

int _reverse_diff_operator(struct Node expr, int nnz, int * wrt, double * values){
  // This computes the local derivatives of the operator with respect to each
  // operand.
  double * deriv_op = malloc(sizeof(double) * expr.data.expr->nargs);
  REVERSE_DIFF_OP[expr.data.expr->op](expr.data.expr->args, expr.data.expr->nargs, deriv_op);

  // Update the adjoints for subexpressions
  for (int i=0; i<expr.data.expr->nargs; i++){
    // I assume expressions are not reused, so I can override any existing
    // adjoint. When/if I handle common subexpressions, I will need to
    // (a) initialize all adjoints to zero and (b) update adjoints with +=
    expr.data.expr->args[i].adjoint = deriv_op[i] * expr.adjoint;
    // Recursively differentiate arguments, updating derivative values when
    // we get to the leaves.
    reverse_diff(expr.data.expr->args[i], nnz, wrt, values);
  }

  free(deriv_op);
  return 0;
}
