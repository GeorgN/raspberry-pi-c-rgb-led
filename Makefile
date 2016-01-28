all: main.c
	gcc -Wall -O2 -std=c99 -std=gnu99 -std=gnu11 -c gpio/gpio.c -o gpio.o
	gcc -Wall -O2 -std=c99 -std=gnu99 -std=gnu11 -c gpio/pwm.c -o pwm.o
	gcc -Wall -O2 -std=c99 -std=gnu99 -std=gnu11 -c main.c -o main.o
	g++ -o rgbLed gpio.o pwm.o main.o -lpthread -s  
	rm gpio.o pwm.o main.o

