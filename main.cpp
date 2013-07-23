#include "stellarino.h"

int main() {
	int a = 1500;

	init();
	pinMode(PD0, OUTPUT_SERVO);

	puts("Enabling UART 2.\n");
	enableUART(2, 115200);

	UARTputs(2, "Ready.\n");
	puts("I told Amanda that I'm ready.\n");
	while (1) {
		puts("I'm waiting for a number.\n");
		a = UARTgeti(2);
		servoWrite(PD0, a);
		puts("I got the number ");
		puti(a);
		putln();
		UARTputs(2, "Moved to ");
		UARTputi(2, a);
		UARTputln(2);
	}
}
