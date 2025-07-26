NAME = hotc
CC = cc
CFLAGS = -Wall -Wextra -Werror $(INCLUDES) -g
LINK_FLAGS =

INCLUDES = -I includes/
LIB_FOLDER = lib/
OBJ_DIR = bin_obj/
BIN_DIR = bin/
EXAMPLES_DIR = examples/

LIB_SRC = $(wildcard $(LIB_FOLDER)*.c)
LIB_OBJS = $(patsubst $(LIB_FOLDER)%.c, $(OBJ_DIR)%.o, $(LIB_SRC))

# OS compatibility
ifeq ($(OS),Windows_NT)
	Color_Off =
	IRed =
	IGreen =
	IYellow =
	ICyan =
	OK = [OK]
	RM = del /Q /S
	MKDIR = mkdir
	MKDIR_BIN = if not exist "$(subst /,\,$(BIN_DIR))" mkdir "$(subst /,\,$(BIN_DIR))"
	MKDIR_OBJ = if not exist "$(subst /,\,$(OBJ_DIR))" mkdir "$(subst /,\,$(OBJ_DIR))"

	EXE = .exe
	STATIC_LIB = $(BIN_DIR)hotc.lib
	ARCHIVER = ar
	ARFLAGS = rcs
else
	Color_Off = \033[0m
	IRed = \033[0;91m
	IGreen = \033[0;92m
	IYellow = \033[0;93m
	ICyan = \033[0;96m
	OK = ✔︎
	RM = rm -rf
	MKDIR = mkdir -p
	MKDIR_BIN = $(MKDIR) $(BIN_DIR)
	MKDIR_OBJ = $(MKDIR) $(OBJ_DIR)

	EXE =
	STATIC_LIB = $(BIN_DIR)libhotc.a
	ARCHIVER = ar
	ARFLAGS = rcs
endif

# Messages
MSG1 = @echo $(IGreen)Compiled Successfully $(OK)$(Color_Off)
MSG2 = @echo $(IYellow)Cleaned Successfully $(OK)$(Color_Off)
MSG3 = @echo $(ICyan)Cleaned $(NAME) Successfully $(OK)$(Color_Off)

# Default: build static library
all: $(STATIC_LIB)

# Create static library
$(STATIC_LIB): $(LIB_OBJS)
	@$(MKDIR_BIN)
	@$(ARCHIVER) $(ARFLAGS) $@ $^
	$(MSG1)

# Compile library object files
$(OBJ_DIR)%.o: $(LIB_FOLDER)%.c
	@$(MKDIR_OBJ)
	@$(CC) $(CFLAGS) -o $@ -c $<

# Pattern rule for examples: make ex-foo
ex-%: $(EXAMPLES_DIR)%/main.c $(STATIC_LIB)
	@$(MKDIR_BIN)
ifeq ($(OS),Windows_NT)
	@if not exist "$(BIN_DIR)$*" mkdir "$(BIN_DIR)$*"
else
	@$(MKDIR) $*
endif
	
	@$(CC) $(CFLAGS) $(INCLUDES) $< $(STATIC_LIB) -o $(BIN_DIR)$*/$*$(EXE)
	$(MSG1)
	@echo Running Example [$*]:
	@$(BIN_DIR)$*/$*$(EXE)

# Clean rules
clean:
	@$(RM) $(subst /,\,$(OBJ_DIR))/*.o
	$(MSG2)

fclean: clean
	@$(RM) $(subst /,\,$(OBJ_DIR))
	@$(RM) $(subst /,\,$(BIN_DIR))
	@$(RM) $(NAME)$(EXE)
	$(RM) $(STATIC_LIB)
	$(MSG3)

re: fclean all
