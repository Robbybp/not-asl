/*
 * Differentiate an expression with respect to provided variables at the current
 * values of all variables. Derivative values are stored in a provided array.
 *
 * struct Node expr:
 *
 *     Root node of expression to differentiate.
 *
 * struct VarListNode * wrt:
 *
 *     Linked list. Each node contains a pointer to a variable. These are the variables
 *     with respect to which we differentiate. We need this so we can eventually
 *     construct a sparse derivative matrix. TODO: Should we construct these during
 *     the AD algorithm?
 *
 * double * values:
 *
 *     Array of derivative values corresponding to each variable. Only indices
 *     corresponding to variables in `wrt` have meaningful values.
 *
 * int nvar:
 *
 *     Number of variables (total, not just in this expression).
 *
 */
// TODO: enum for forward/backward mode, or separate functions
int differentiate(struct Node expr, struct VarListNode * wrt, double * values, int nvar);

// Can I make this work?
//struct CSRMatrix differentiate_expression(struct Node expr);

// Need intermediate storage
// Can I do this with O(1) storage?
//
int _differentiate_constant(struct Node expr, struct VarListNode * wrt, double * values, int nvar);
int _differentiate_variable(struct Node expr, struct VarListNode * wrt, double * values, int nvar);
int _differentiate_expression(struct Node expr, struct VarListNode * wrt, double * values, int nvar);

int differentiate(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
  switch(expr.type){
    case CONST_NODE:
      return _differentiate_constant(expr, wrt, values, nvar);
    case VAR_NODE:
      return _differentiate_variable(expr, wrt, values, nvar);
    case EXPR_NODE:
      return _differentiate_expression(expr, wrt, values, nvar);
  }
}

int _differentiate_constant(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
  for (int i=0; i<nvar; i++){
    values[0] = 0.0;
  }
  return nvar;
}

int _differentiate_variable(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
  struct VarListNode * node = wrt;
  int j = 0;
  while (node){
    if (expr.data.var->index == node->variable->index){
      if (values[j] == 0){
        values[j] = 1.0;
      } else {
        printf("ERROR: Derivative WRT var %d already has a value", j);
        exit(-1);
      }
    }
    j += 1;
    node = node->next;
  }
  return nvar;
}

int _differentiate_expression(struct Node expr, struct VarListNode * wrt, double * values, int nvar){
  // expr = f(arg1, arg2, ...)
  // df/d(wrt) = f'(arg1(wrt), arg2(wrt), ...) * (d(arg1)/d(wrt) + d(arg2)/d(wrt) + ...)
  //
  // TODO: Branch on operator to evaluate local derivative
  // TODO: This should be a vector in the space of arguments
  double d_expr = 1.0;

  for (int i=0; i<expr.data.expr->nargs; i++){
    double * arg_values = malloc(nvar * sizeof(double));
    for (int j=0; j<nvar; j++){arg_values[j] = 0.0;}
    differentiate(expr.data.expr->args[i], wrt, arg_values, nvar);

    for (int j=0; j<nvar; j++){
      values[j] += d_expr * arg_values[j];
    }
  }
  return 0;
}

int _differentiate_sum(struct Node * args, int nargs, double * deriv);
int _differentiate_sum(struct Node * args, int nargs, double * deriv){
  // NOTE: Expressions are not evaluated here. If I add side-effects to evaluate,
  // need to make sure this happens in this case.
  for (int j=0; j<nargs; j++){
    deriv[j] = 1.0;
  }
  return 0;
}

int _differentiate_product(struct Node * args, int nargs, double * deriv);
int _differentiate_product(struct Node * args, int nargs, double * deriv){
  for (int j=0; j<nargs; j++){
    // TODO: Re-use expression values that have been previously evaluated
    deriv[j] = 1.0;
    for (int jj=0; jj<nargs; jj++){
      if (j != jj){
        deriv[j] *= evaluate(args[jj]);
      }
    }
  }
  return 0;
}

int _differentiate_subtraction(struct Node * args, int nargs, double * deriv);
int _differentiate_subtraction(struct Node * args, int nargs, double * deriv){
  if (nargs != 2){
    printf("ERROR: Wrong number of nodes for subtraction node\n");
  }
  deriv[0] = 1.0;
  deriv[1] = -1.0;
  return 0;
}
