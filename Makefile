model:
	python model.py --model=unary

test-parse: model
	gcc -o test-parse src/test-parse.c
	./test-parse model.nl

test-diff: model
	gcc -o test-diff src/test-diff.c
	./test-diff model.nl
