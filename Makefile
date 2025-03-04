# Calc.

PROJECT_NAME = Calculator.app

# Creating directories for objects, tests, binary, coverage files.
SRC_DIR = ./src
OBJ_DIR = ./objs
TESTS_DIR = ./tests
REPORT_DIR = ./report
GCOV_DIR = ./gcov
RESOURCES_DIR = ./resources
REPORT_DIR = ./report
GCOV_DIR = ./gcov
ARCHIVE_DIR = ./$(PROJECT_NAME)
INSTALL_DIR = ./build/Calculator-v1.0/

# Standard linux commands.
FLAGS_RM := -rm -rf
# -p: no error if existing, make parent directories as needed
FLAGS_MK := -mkdir -p
# -r: recursively copy directories
FLAGS_CP := -cp -r

# Main list flags.
CC ?= gcc
CFLAGS = -Wall -Wextra -std=c2x
GTKFLAGS = 
GTKLIBS = -rdynamic
LIBM =

# Coverage GCOV.
FLAGS_GCOV = -fprofile-arcs -ftest-coverage

# Separate options for MacOS.
OS := $(shell uname -s)
ifeq ($(OS), Darwin)
	GTKFLAGS += `pkg-config --cflags gtk+-3.0 glib-2.0 gobject-2.0`
	GTKLIBS += `pkg-config --libs gtk+-3.0 glib-2.0 gobject-2.0`
	OPEN_CMD = open
	FLAGS_LCHECK +=
	BREW := ${HOME}/homebrew
	FLAGS_GCOV +=
	LIBM +=
endif

# Separate options for Linux.
ifeq ($(OS), Linux)
	GTKFLAGS += `pkg-config --cflags gtk+-3.0 libsoup-3.0 json-glib-1.0 glib-2.0 gobject-2.0`
#	GTKFLAGS += `pkg-config --cflags gtk+-3.0 glib-2.0 gobject-2.0`
	GTKLIBS += `pkg-config --libs gtk+-3.0 libsoup-3.0 json-glib-1.0 glib-2.0 gobject-2.0`
	OPEN_CMD = xdg-open
#	FLAGS_LCHECK += -lpthread -lrt -lsubunit
	FLAGS_LCHECK += `pkg-config --cflags --libs check`
	BREW = .
	LIBM += -lm
#   FLAGS_GCOV += -Wno-error=coverage-invalid-line-number -Wno-coverage-invalid-line-number
endif

# Script: search the directory for all source files .c
SOURCE = $(wildcard $(SRC_DIR)/*.c)

# Script: create object files .o
OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(SOURCE:%.c=%.o)))

# Script: search the directory for all source files .check
TESTS = $(TESTS_DIR)/000.include
TESTS += $(wildcard $(TESTS_DIR)/*.check)

# Script: create object files .o .d .gcov with gcov flags.
GSOURCE = src/comp_equation.c src/comp_stack.c
GOBJECTS = $(addprefix $(GCOV_DIR)/, $(notdir $(GSOURCE:%.c=%.o)))

# Targets.
all: resources.c $(PROJECT_NAME)

install: $(PROJECT_NAME)
	$(FLAGS_MK) $(INSTALL_DIR)
	cp -r $(PROJECT_NAME) $(INSTALL_DIR)

uninstall:
	$(FLAGS_RM) $(INSTALL_DIR)

run: all
	-./$(PROJECT_NAME)

$(PROJECT_NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBM) $(GTKLIBS) 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(FLAGS_MK) $(OBJ_DIR)
	$(CC) -g -c $(CFLAGS) $^ -o $@ $(LIBM) $(GTKFLAGS)

$(GCOV_DIR)/%.o: $(SRC_DIR)/%.c
	@$(FLAGS_MK) $(GCOV_DIR)
	$(CC) $(FLAGS_GCOV) -c -MD $< -o $@ $(LIBM) $(GTKFLAGS) $(GTKLIBS)

$(TESTS_DIR)/tests.c: $(TESTS)
	checkmk $(TESTS) > $@

gcov_lib: $(GOBJECTS)
	ar rcs $(GCOV_DIR)/$@.a $(GOBJECTS)
	ranlib $(GCOV_DIR)/$@.a

resources.c:
	glib-compile-resources --generate-source --target=src/$@ --sourcedir=resources/ --c-name resources resources/ui.gresource.xml  

test: $(TESTS_DIR)/tests.c
	$(CC) -g -o $(TESTS_DIR)/tests_$(PROJECT_NAME) $(TESTS_DIR)/tests.c comp_equation.c comp_stack.c $(LIBM) $(FLAGS_LCHECK) $(GTKFLAGS) $(GTKLIBS)
	$(TESTS_DIR)/tests_$(PROJECT_NAME)

main:
	$(CC) $(CFLAGS) -g $(GSOURCE) $(TESTS_DIR)/main.c -o $(TESTS_DIR)/$@.bin $(LIBM) $(GTKFLAGS) $(GTKLIBS)

gcov_report:  gcov_lib $(TESTS_DIR)/tests.c
	@$(FLAGS_MK) $(GCOV_DIR)
	$(CC) $(TESTS_DIR)/tests.c -c -o $(GCOV_DIR)/tests.o $(FLAGS_LCHECK) $(LIBM) $(GTKFLAGS) $(GTKLIBS)
	$(CC) $(GOBJECTS) $(GCOV_DIR)/tests.o $(FLAGS_GCOV) $(FLAGS_LCHECK) $(LIBM) $(GTKFLAGS) $(GTKLIBS) -o $(GCOV_DIR)/test_cov
	-./$(GCOV_DIR)/test_cov
	lcov -t "tests_$(PROJECT_NAME)" -o $(GCOV_DIR)/tests_$(PROJECT_NAME).info -c -d $(GCOV_DIR)
	genhtml -o $(REPORT_DIR) $(GCOV_DIR)/tests_$(PROJECT_NAME).info
	$(FLAGS_RM) *.gcno *.gcda test_cov tests_$(PROJECT_NAME).info
	-$(OPEN_CMD) $(REPORT_DIR)/index-sort-f.html

dvi:
	open README.md

dist:
	@$(FLAGS_MK) $(ARCHIVE_DIR)
	cp -r *.c *.h Makefile *.md tests $(RESOURCES_DIR) $(ARCHIVE_DIR)
	tar -cvzf $(PROJECT_NAME).tar $(ARCHIVE_DIR)
	$(FLAGS_RM) $(ARCHIVE_DIR)

style_fix:
	cp linters/.clang-format $(SRC_DIR)
	cp linters/.clang-format $(TESTS_DIR)
	clang-format -i $(SRC_DIR)/*.c $(SRC_DIR)/*.h $(TESTS_DIR)/*.c
	$(FLAGS_RM) $(SRC_DIR)/.clang-format $(TESTS_DIR)/.clang-format

valgrind: main
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose $(TESTS_DIR)/main.bin

leaks: main
	leaks -atExit -- $(TESTS_DIR)/main.bin

check:
#   cppcheck -q --enable=all --inconclusive --platform=unix32 --std=c11 --template='{id} {file}:{line} {message}' --suppress=missingIncludeSystem $(SOURCE) *.h
	@echo "----- cppcheck --------"	
	cppcheck -q --enable=all --inconclusive --std=c11 --template='{id} {file}:{line} {message}' --suppress=missingIncludeSystem $(SOURCE) *.h
	@echo "----- style check -----"
	cp ../materials/linters/.clang-format $(SRC_DIR)/
	clang-format -n --style=Google *.c *.h
	rm .clang-format
	@echo "----- check done ------"

clean:
	$(FLAGS_RM) $(SRC_DIR)/resources.c $(RESOURCES_DIR)/*.glade~ *.tar
	$(FLAGS_RM) $(PROJECT_NAME)
	$(FLAGS_RM) $(OBJ_DIR)
	$(FLAGS_RM) $(TESTS_DIR)/tests.c $(TESTS_DIR)/tests_$(PROJECT_NAME) $(TESTS_DIR)/main.bin
	$(FLAGS_RM) /build
	$(FLAGS_RM) $(GCOV_DIR)	
	$(FLAGS_RM) $(REPORT_DIR)

rebuild: clean all

.PHONY: all clean
