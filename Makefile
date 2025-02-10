SRC := ser.c
OUT := ser

all:
	gcc ${SRC} -o ${OUT}

arm:
	arm-linux-gnueabihf-gcc ${SRC} -o ${OUT}

clean:
	-@rm -Rf *~ ${OUT}

