#include <stellarino.h>
#include <gait.h>

/*
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
*/

int main()
{
    int i, n = 0;
    float a, b, c;

    init();
    StagSystem stag;

    resetMicros();

    for (n = 0; n < 20; n++)
    {
        stag.moveLegs(15, 0, 0, 100);
        for (i = 0; i < 4; i++)
        {
            stag.getLeg(i, &a, &b, &c);
            puts("Leg ");
            puti(i+1);
            puts(" Angle1: ");
            putf(a, 3);
            puts(" Angle2: ");
            putf(b, 3);
            puts(" Angle3: ");
            putf(c, 3);
            putln();
        }
        putln();
        delay(50);
    }
}
