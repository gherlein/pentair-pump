SRC := ser.c
OUT := pump
OUTARM := pump-arm

all:
	gcc ${SRC} -o ${OUT}

arm:
	arm-linux-gnueabihf-gcc ${SRC} -o ${OUT}

clean:
	-@rm -Rf *~ ${OUT}
