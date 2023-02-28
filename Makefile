CC = gcc -Wall -Wextra -O3

recentmost : recentmost.c
	$(CC) -o $@ $^

test : recentmost
	find ~ -type f | ./recentmost 20
