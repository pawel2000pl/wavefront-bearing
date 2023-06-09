all: clean bin/wf examples

bin:
	mkdir -p bin

models:
	mkdir -p models
	
bin/wf: bin
	gcc --std=c11 -Wall -O3 src/wf.c -o bin/wf -lm

clean:
	rm -rf bin
	rm -rf models
	
examples: bin/wf models
	bash -c "find examples -name '*.in' -exec bash -c './bin/wf < {} > models/\`basename -s .in {}\`.obj 2> models/\`basename -s .in {}\`.stl' \;"

.PHONY: clean all examples
