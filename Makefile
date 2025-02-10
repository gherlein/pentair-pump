SRC := ser.c
OUT := pump
OUTARM := pump-arm

all:
	gcc ${SRC} -o ${OUT}

arm:
	arm-linux-gnueabihf-gcc ${SRC} -o ${OUT}

clean:
	-@rm -Rf *~ ${OUT}

git:
	git add -A
	git commit -m "auto commit"
	git push
