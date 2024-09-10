
SRCS =	${wildcard *.cpp}

OBJS = ${SRCS:.c=.o}

NAME = IRCServer

CXXFLAGS = -Wall -Werror -Wextra -g -std=c++98

CXX = c++
RM = rm

all: $(NAME)

$(NAME): $(SRCS)
	@$(CXX) $(CXXFLAGS) $(SRCS) -o $(NAME)

clean:
	@rm -rf $(OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all re clean fclean