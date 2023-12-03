CC=gcc -o
sv=server/server
cl=client/client

all: ${sv}.c ${cl}.c
	${CC} ${sv}.o ${sv}.c
	${CC} ${cl}.o ${cl}.c

server: ${sv}.c
	${CC} ${sv}.o ${sv}.c

client: ${cl}.c
	${CC} ${cl}.o ${cl}.c

clean:
	rm ${sv}.o
	rm ${cl}.o