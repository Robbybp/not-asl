struct VarListNode {
  struct VarListNode * next;
  struct Variable * variable;
};

struct CSRMatrix {
  int nnz;
  int nrow;
  int ncol;
  int * indptr;
  int * indices;
  double * values;
};

int identify_variables(struct Node expr, int eidx, int * in_expr, int nvar, struct VarListNode ** head);
void free_varlist(struct VarListNode ** head);
void print_csrmatrix(struct CSRMatrix);

int identify_variables(
  struct Node expr,
  int eidx,
  int * in_expr,
  int nvar,
  struct VarListNode ** head
){
  switch(expr.type){
    case CONST_NODE:
      return 0;
    case VAR_NODE:
    {
      if (expr.data.var->index >= nvar){printf("Variable index out of bounds.\n"); exit(-1);}
      int last_expr_containing_var = in_expr[expr.data.var->index];
      if (last_expr_containing_var == eidx){
        // We have already encountered this variable in this expression.
        // We don't need to increment the variable counter or update the
        // variable list.
        return 0;
      }else{
        // Mark that the variable was encountered in this constraint.
        in_expr[expr.data.var->index] = eidx;
        // Allocate a new node.
        struct VarListNode * newvarnode = malloc(sizeof(struct VarListNode));
        newvarnode->variable = expr.data.var;
        if (*head){
          // If we already have a nodelist, insert this variable at the
          // head.
          // Attach head to this new node
          newvarnode->next = (*head);
        }else{
          // This should be done automatically, but just to be explicit here.
          newvarnode->next = NULL;
        }
        // Re-assign head as this new node
        *head = newvarnode;
        return 1;
      }
    }
    case OP_NODE:
    {
      int varcount = 0;
      for (int i=0; i<expr.data.expr->nargs; i++){
        varcount += identify_variables(
          expr.data.expr->args[i],
          eidx,
          in_expr,
          nvar,
          head
        );
      }
      return varcount;
    }
  }
}

void free_varlist(struct VarListNode ** head){
  struct VarListNode * node = *head;
  struct VarListNode * next;
  while(node){
    next = node->next;
    free(node);
    node = next;
  }
}

void print_csrmatrix(struct CSRMatrix csr){
  printf("\n==========\n");
  printf("CSR Matrix\n");
  printf("==========\n");
  printf("shape = %d x %d\n", csr.nrow, csr.ncol);
  printf("NNZ   = %d\n", csr.nnz);
  printf("----------\n");
  for (int i=0; i<csr.nrow; i++){
    // TODO: Allocate appropriate number of characters to %d
    printf("Row %d:", i);
    for (int k=csr.indptr[i]; k<csr.indptr[i+1]; k++){
      printf(" (%d, %1.3f)", csr.indices[k], csr.values[k]);
    }
    printf("\n");
  }
  printf("==========\n");
}
