faketty: faketty.c
	$(CC) -std=c89 -Wall -Wextra -shared -fPIC -pie -o $@ $< -ldl
