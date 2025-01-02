A hobby implementation of sparse forward and reverse automatic differentiation
on algebraic expression trees.

### Status

This is a project I started for my own education and is not intended for
general use.

### Feature roadmap

- [x] Expression data type; functions to print and evaluate
- [x] Basic `.nl` expression parser
- [x] `identify_variables` function and CSR matrix data structure
- [x] Forward-mode (first-order) AD
- [x] Reverse-mode (first-order) AD
- [ ] IPOPT interface
- [ ] `.sol` file writer
- [ ] Second-order AD
- [ ] Support for common subexpressions
- [ ] Support for AMPL external functions
