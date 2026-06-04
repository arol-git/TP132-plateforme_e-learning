all: lms

lms: main.o syllabus.o qcm.o progression.o contenu.o persistence.o utils.o
	gcc main.o syllabus.o qcm.o progression.o contenu.o persistence.o utils.o -o lms

main.o: main.c syllabus.h qcm.h contenu.h progression.h persistence.h utils.h
	gcc -c main.c 

syllabus.o: syllabus.c syllabus.h utils.h
	gcc -c syllabus.c

qcm.o: qcm.c qcm.h syllabus.h utils.h
	gcc -c qcm.c 

progression.o: progression.c progression.h syllabus.h utils.h
	gcc -c progression.c 

contenu.o: contenu.c contenu.h syllabus.h qcm.h utils.h
	gcc -c contenu.c 

persistence.o: persistence.c persistence.h syllabus.h qcm.h utils.h
	gcc -c persistence.c

utils.o: utils.c utils.h
	gcc -c utils.c 

run:
	./lms

clean:
	rm -f *.o lms

propre: clean
	rm -f database.json

.PHONY: all clean propre run