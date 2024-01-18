CC=gcc -Wall -Werror -Wextra -g -std=c11
SOURCES=cat.c
OBJECTS=$(SOURCES:.c=.o)
FILES=cat.c cat.h
EXE=s21_cat

all: $(EXE)

$(EXE): $(SOURCES)
	$(CC) $(SOURCES) -c 
	$(CC) $(OBJECTS) -o $(EXE)

rebuild: clean all

clean:
	rm -rf *.o $(EXE)

cf:
	clang-format -style=Google -i $(FILES)

check:
	clang-format -style=Google -n $(FILES)

cppc:
	cppcheck --enable=all --suppress=missingIncludeSystem $(FILES)

leaks: all
	leaks -atExit -- ./$(EXE) cat.h

test: all
	bash test_cat.sh