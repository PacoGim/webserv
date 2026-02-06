NAME = webserv
STD_OUT = /dev/stdout
CONFIG_PATH = ../config/config.json
WEB			= web_demo
CGI_BIN		= ubuntu_cgi_tester
CGI_PATH	= ../script_tester
WEB_PATH	= ../$(WEB)
CXX         ?= g++
#CXXFLAGS    ?= -Wall -Wextra -Werror -std=c++98 -MMD -MP -O3 -fomit-frame-pointer -march=native -flto -g // optimisations conflict only with valgrind

# CXXFLAGS    ?= -Wall -Wextra -Werror -std=c++98 -MMD -MP -O0 -g -fsanitize=address -fsanitize=undefined -fsanitize=leak
CXXFLAGS    ?= -Wall -Wextra -Werror -std=c++98 -MMD -MP -O3 -fomit-frame-pointer -march=native -flto -DNDEBUG

SRCS 		= 	entry_point.cpp					\
 				MemoryPool/ClientPool.cpp		\
 				Http/Method.cpp					\
 				Http/Url.cpp					\
 				Http/Version.cpp				\
 				Http/Headers.cpp				\
 				Http/Request.cpp				\
 				Helper/String.cpp   			\
 				Helper/Logger.cpp				\
 				Helper/Loggable.cpp				\
 				Component/Component.cpp			\
 				Component/Socket.cpp			\
 				Component/ClientHttp.cpp		\
 				Component/ServerHttp.cpp		\
 				Component/EventLoop.cpp			\
 				Types/State.cpp					\
 				Helper/Number.cpp				\
 				Helper/Time.cpp					\
 				JSON/Json.cpp					\
 				JSON/JsonParser.cpp				\
 				JSON/JsonValidator.cpp			\
 				JSON/JsonValue.cpp				\
 				JSON/JsonImp/JsonArray.cpp		\
 				JSON/JsonImp/JsonBool.cpp		\
 				JSON/JsonImp/JsonNull.cpp		\
 				JSON/JsonImp/JsonNumber.cpp		\
 				JSON/JsonImp/JsonObject.cpp		\
 				JSON/JsonImp/JsonString.cpp		\
 				Configuration/MainConfig.cpp	\
 				Configuration/ServerConfig.cpp	\
 				Configuration/Location.cpp		\
 				Http/Router.cpp					\
 				Http/Response.cpp				\
 				Helper/Mime.cpp					\
 				Cache/SharedData.cpp			\
 				Cache/DataCached.cpp			\
 				Cache/File.cpp					\
				Component/SignalHandler.cpp		\
				Component/PipeCgi.cpp			\
				Types/Event.cpp					\
				Http/Body.cpp					\
				Helper/LoggerV2.cpp				\
				Cookie/User.cpp					\
				Cookie/Session.cpp

OBJ_DIR     = objects
OBJS        = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
DEPS        = $(OBJS:.o=.d)


UBUNTU_URL  = https://cdn.intra.42.fr/document/document/35869/ubuntu_tester
CGI_URL    	= https://cdn.intra.42.fr/document/document/35867/ubuntu_cgi_tester

DOWNLOADS   = /goinfre/download
SCRIPT_DIR  = /goinfre/script_tester
UBUNTU_BIN  = $(SCRIPT_DIR)/ubuntu_tester
CGI_BIN     = $(SCRIPT_DIR)/ubuntu_cgi_tester

# wrk -t1 -c8192 -d10s --timeout 12s --latency http://127.0.0.1:7777

all: copy_folder $(UBUNTU_BIN) $(CGI_BIN)
	+$(MAKE) $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	cp $(NAME) ../$(NAME)
	mkdir -p /goinfre/download/

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Règle pour ubuntu_tester
$(UBUNTU_BIN):
	@echo "➡️  Téléchargement de ubuntu_tester si nécessaire..."
	mkdir -p $(SCRIPT_DIR)
	curl -L -o $@ $(UBUNTU_URL) || wget -O $@ $(UBUNTU_URL)
	chmod +x $@

# Règle pour cgi_tester
$(CGI_BIN):
	@echo "➡️  Téléchargement de cgi_tester si nécessaire..."
	mkdir -p $(SCRIPT_DIR)
	curl -L -o $@ $(CGI_URL) || wget -O $@ $(CGI_URL)
	chmod +x $@

# Règle pour les téléchargements généraux
copy_folder:
	mkdir -p $(DOWNLOADS)
	cp -r $(WEB_PATH) /goinfre
	cp -r $(CGI_PATH) /goinfre/script_tester

fast:
	@CORES=$$(nproc 2>/dev/null || sysctl -n hw.ncpu); \
	echo "Compilation en parallèle avec $$CORES jobs..."; \
	$(MAKE) -j$$CORES


clean:
	rm -rf $(OBJ_DIR)
	rm -f /tmp/webserv*

fclean: clean
	rm -f $(NAME)
	rm -f $(UBUNTU_BIN) $(CGI_BIN)

re: fclean fast

log: fast
	INFO= DEBUG= LOG= COLOR= CHRONO= FLUSH= /usr/bin/time -v ./$(NAME) $(CONFIG_PATH) > $(STD_OUT)

debug: fast
	INFO= DEBUG= COLOR= CHRONO= FLUSH= /usr/bin/time -v ./$(NAME) $(CONFIG_PATH) > $(STD_OUT)

info: fast
	INFO= COLOR= CHRONO= FLUSH= /usr/bin/time -v ./$(NAME) $(CONFIG_PATH) > $(STD_OUT)

run: fast
	INFO= DEBUG= COLOR= CHRONO= /usr/bin/time -v ./$(NAME) $(CONFIG_PATH) > $(STD_OUT)

#
#val: fast
#	INFO= DEBUG= COLOR= CHRONO= FLUSH= valgrind --track-fds=yes --track-origins=yes --leak-check=full --show-leak-kinds=all ./$(NAME) $(CONFIG_PATH) > $(STD_OUT)

-include $(DEPS)

.PHONY: all fast clean fclean re log debug info run val copy_folder
