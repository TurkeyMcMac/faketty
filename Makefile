faketty: faketty.c
	gcc -std=c89 -Wall -Wextra -shared -fPIC -pie -o $@ $< -ldl
