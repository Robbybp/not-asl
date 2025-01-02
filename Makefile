model.nl:
	python model.py --model=unary

test-parse: model.nl src/test-parse.c
	gcc -g -o test-parse src/test-parse.c
	./test-parse model.nl

test-diff: model.nl src/test-diff.c
	gcc -g -o test-diff src/test-diff.c
	./test-diff model.nl

clean:
	rm -f test-parse test-diff model.nl
