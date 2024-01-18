CC=gcc -Wall -Werror -Wextra -g -std=c11
SOURCES=grep.c
OBJECTS=$(SOURCES:.c=.o)
FILES=grep.c grep.h
EXE=s21_grep

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
	leaks -atExit -- ./$(EXE) file grep.h
	
test: all
	bash test_grep.sh

pairs: all
	bash test_pairs.sh