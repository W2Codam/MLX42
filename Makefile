# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: W2Wizard <w2.wizzard@gmail.com>              +#+                      #
#                                                    +#+                       #
#    Created: 2022/02/26 21:32:49 by W2Wizard      #+#    #+#                  #
#    Updated: 2022/03/29 18:08:51 by W2Wizard      ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

NAME		:= libmlx42.a
LIB_DIR		:= lib
SRC_DIR		:= src
SHADER_DIR	:= shaders
INCLUDE_DIR	:= include

override HEADERS += -I $(INCLUDE_DIR)

CFLAGS := -Wextra -Wall -Wunreachable-code -Wno-char-subscripts
ifndef NOWARNING
	CFLAGS += -Werror
endif
ifdef DEBUG
	CFLAGS += -g
else
	CFLAGS	+= -Ofast -D NDEBUG
endif

# Recursive wildcard/find function
rwildcard = $(sort $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2) $(wildcard $1/$2)))

SHDR	:= $(call rwildcard,$(SHADER_DIR),default.*)
SHDR	:= $(SHDR:$(SHADER_DIR)/default.%=$(SRC_DIR)/mlx_%_shader.c)
LIB		:= $(call rwildcard,$(LIB_DIR),*.c)
SRCS	:= $(call rwildcard,$(SRC_DIR),*.c) $(LIB) $(SHDR)
OBJS	:= $(SRCS:%.c=%.o)

ifeq ($(OS), Windows_NT)
	ifdef CYGWIN
		include Makefile_Unix.mk
	else
		include Makefile_WindowsNT.mk
	endif
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		include Makefile_Unix.mk
	else ifeq ($(UNAME_S), Darwin)
		# Default
		DYLIB_EXISTS := test -e /usr/local/lib/libglfw.3.dylib || echo "false"

		# Generic Homebrew path
		ifneq ($(DYLIB_EXISTS), false)
			BREW_GLFW_PREFIX := $(shell brew --prefix glfw)
			DYLIB_EXISTS := test -e $(BREW_GLFW_PREFIX)/lib/libglfw.3.dylib || echo "false"
			ifneq ($(DYLIB_EXISTS), false)
				HEADERS += -I $(BREW_GLFW_PREFIX)/include
			endif
		endif

		include Makefile_Unix.mk
	else
		$(error OS: $(OS) - Is not supported!)
	endif
endif

#//= Make Rules =//#
all: $(SHDR) $(NAME)

# Run make as part of the recipe to allow for multi-threading to be used (-j)
re: fclean
	@$(MAKE) -e

#//= Misc =//#
.DEFAULT_GOAL := all
.PHONY: all clean fclean re
