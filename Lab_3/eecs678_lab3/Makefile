STUDENT_ID=3125041

DIR=bash-4.2
STR=execute
NUM_FILES=10

build:
	gcc -Wall -g finder.c -o finder

find:
	/bin/bash finder.sh $(DIR) $(STR) $(NUM_FILES)

test:
	/bin/bash finder.sh $(DIR) $(STR) $(NUM_FILES) > tmp1
	./finder $(DIR) $(STR) $(NUM_FILES) > tmp2
	-diff tmp1 tmp2
	rm -f tmp1 tmp2

pipe:
	gcc -Wall -g pipe.c -o pipe

clean:
	rm -f finder pipe tmp1 tmp2

tar:
	make clean
	mkdir $(STUDENT_ID)-lab-03
	cp -r bash-4.2 Makefile finder.sh finder.c $(STUDENT_ID)-lab-03
	tar cvzf $(STUDENT_ID)-lab-03.tar.gz $(STUDENT_ID)-lab-03
	rm -rf $(STUDENT_ID)-lab-03
