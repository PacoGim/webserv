RST="\033[0m"
RED="\033[31m"
GREEN="\033[32m"
BLUE="\033[34m"
MAGENTA="\033[35m"

all:
	@+$(MAKE) -C version_3_0 fast
	@echo $(BLUE)"Projects built\n"$(RST)
clean:
	@+$(MAKE) -C version_3_0 clean
	@echo $(MAGENTA)"Projects cleaned up temporary files\n"$(RST)

fclean:
	@+$(MAKE) -C version_3_0 fclean
	@echo $(RED)"Projects cleaned up binaries\n"$(RST)

re:
	@+$(MAKE) -C version_3_0	re
	@echo $(GREEN)"Projects rebuilt\n"$(RST)

log:
	@+$(MAKE) -C version_3_0 log

debug:
	@+$(MAKE) -C version_3_0 debug

info:
	@+$(MAKE) -C version_3_0 info

run:
	@+$(MAKE) -C version_3_0 run

.PHONY: all clean fclean re log debug info run


