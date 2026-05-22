# 
# This Makefile is for the ISOS project and ensure compatibility with the CI.
# Make sure to include this file in your root Makefile (i.e., at the top-level of your repository).
#

# TODO
# Initialize this variable to point to the directory holding your header if any.
# Otherwise, the CI will consider the top-level directory.
INCLUDE_DIR=./include

# TODO
# Initialize this variable with a space separated list of the paths to the loader source files (not the library).
# You can use some make native function such as wildcard if you want.
SRC_FILES=./src/isos_loader.c ./src/my_dl.c ./src/mylib.c ./src/my_argp.c ./src/my_elfh.c ./src/my_ph.c ./src/mmapp.c ./src/relocate.c
 
# TODO
# Uncomment this and initialize it to the correct path(s) to your source files if your project sources are not located in `src`.
#vpath %.c path/to/src

CC=gcc
CFLAGS=-O2 -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict -fanalyzer -I$(INCLUDE_DIR)
CLANG=-Wall -Wextra -Wuninitialized -Wpointer-arith -Wcast-qual -Wcast-align -I$(INCLUDE_DIR)
LDFLAGS=-ldl
OBJECTS=$(SRC_FILES:.c=.o)

all: isos_loader

libmylib.so: ./src/mylib.c
	gcc $(CFLAGS) -shared -nostdlib -fvisibility=hidden -e tableS -o libmylib.so ./src/mylib.c

isos_loader_clean: $(OBJECTS)
	gcc -o isos_loader_clean $^ 

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@


encrypter: ./src/encrypter.c
	gcc -o encrypter ./src/encrypter.c

isos_loader: isos_loader_clean encrypter libmylib.so
	cp isos_loader_clean isos_loader
	./encrypter isos_loader libmylib.so


check-clang:
	clang $(CLANG) $(SRC_FILES)

tidy:
	clang-tidy $(SRC_FILES) -- -I$(INCLUDE_DIR)

sanitize-address: encrypter libmylib.so
	clang $(CLANG) -fsanitize=address -g $(SRC_FILES) -o isos_loader_sa
	./encrypter isos_loader_sa libmylib.so
 
sanitize-ub: encrypter libmylib.so
	clang $(CLANG) -fsanitize=undefined -g $(SRC_FILES) -o isos_loader_ub 
	./encrypter isos_loader_ub libmylib.so
 
sanitize-memory: encrypter libmylib.so
	clang $(CLANG) -fsanitize=memory -g $(SRC_FILES) -o isos_loader_sm 
	./encrypter isos_loader_sm libmylib.so


check-all: all check-clang tidy sanitize-address sanitize-ub sanitize-memory
	@echo ""
	@echo ""
	@echo "--- Do not forget to execute those different sanitized programs with different control flow paths! ---"
	@echo
 
clean:
	rm -f isos_loader libmylib.so $(OBJECTS) isos_loader_sm isos_loader_ub isos_loader_sa a.out isos_loader_clean encrypter
 
.PHONY: all clean check-clang tidy sanitize-address sanitize-ub sanitize-memory check-all







