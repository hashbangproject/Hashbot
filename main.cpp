#include <stellarino.h>
#include <gait.h>
#include <messaging.h>

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

static const unsigned char legPins[12] =
{
    PB5, PB0, PD0,  // Leg 1
    PB1, PD1, PD3,  // Leg 2
    PB2, PB3, PC4,  // Leg 3
    PC5, PB7, PB6   // Leg 4
};

/*
static const float jointMinMaxAngles[12][2] =
{
    {0.2f, 0.205f}, {0.5f, 1.0f}, {1.4f, 1.5f},     // Leg 1
    {0.2f, 0.205f}, {0.5f, 1.0f}, {1.4f, 1.5f},     // Leg 2
    {0.2f, 0.205f}, {-1.0f, -0.5f}, {-1.5f, 1.4f},  // Leg 3
    {0.2f, 0.205f}, {-1.0f, -0.5f}, {-1.5f, 1.4f}   // Leg 4
};
*/

static const float angleToPulseFactor = 752.0f;

// Servo Pulse ~= servoPulseOffset + angleInRadians*angleToPulseFactor
static const short servoPulseOffset[12] =
{
    1350, 930, 410,     // Leg 1
    1350, 930, 410,     // Leg 2
    1350, 2060, 2590,   // Leg 3
    1350, 2060, 2590    // Leg 4
};

StagSystem g_stag;
uint8_t g_msgBody[256];

extern "C"
{
void stagInterrupt(void);
void statusInterrupt(void);
}

int main()
{
    init();
    resetMicros();
    enableMessaging();

    // Set up the pins
    int i;
    for (i = 0; i < 12; i++) pinMode(legPins[i], OUTPUT_SERVO);

    // We will use WTIMER0 for the interrupts
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_WTIMER0);
    ROM_TimerConfigure(WTIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | TIMER_CFG_B_PERIODIC));

    // Set up the stag walking timer interrupt
    ROM_TimerPrescaleSet(WTIMER0_BASE, TIMER_A, 79999); // 1 ms per tick
    TimerIntRegister(WTIMER0_BASE, TIMER_A, stagInterrupt);
    ROM_TimerLoadSet(WTIMER0_BASE, TIMER_A, 50);        // 50 ms per cylce
    ROM_TimerIntEnable(WTIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerEnable(WTIMER0_BASE, TIMER_A);

    // Set general sensor/status interrupt
    ROM_TimerPrescaleSet(WTIMER0_BASE, TIMER_B, 79999); // 1 ms per tick
    TimerIntRegister(WTIMER0_BASE, TIMER_B, statusInterrupt);
    ROM_TimerLoadSet(WTIMER0_BASE, TIMER_B, 500);       // 500 ms per cycle
    ROM_TimerIntEnable(WTIMER0_BASE, TIMER_TIMB_TIMEOUT);
    ROM_TimerEnable(WTIMER0_BASE, TIMER_B);

    g_stag.setSpeed(30, 0);

    uint8_t msgLength;
    MessageType msgType;

    // Main message handing loop
    while (1)
    {
        switch (getMessage(&msgType, &msgLength, g_msgBody))
        {
        case 0: // Success
            handleMessage(msgType, msgLength, g_msgBody);
            break;
        case 1: // Timeout
            sendError(TIMEOUT_ERROR);
            break;
        case 2: // CRC Error
            sendError(CRC_ERROR);
            break;
        }
    }
}

void stagInterrupt(void)
{
    g_stag.moveLegs();

    float a, b, c;

    for (int i = 0; i < 4; i++)
    {
        g_stag.getLeg(i, &a, &b, &c);

        /*
        puts("Leg ");
        puti(i+1);
        puts(" Value1: ");
        puti(servoPulseOffset[i*3] + a*angleToPulseFactor);
        puts(" Value2: ");
        puti(servoPulseOffset[i*3 + 1] + b*angleToPulseFactor);
        puts(" Value3: ");
        puti(servoPulseOffset[i*3 + 2] + c*angleToPulseFactor);
        putln();
        */

        servoWrite(legPins[i*3], servoPulseOffset[i*3] + a*angleToPulseFactor);
        servoWrite(legPins[i*3 + 1], servoPulseOffset[i*3 + 1] + b*angleToPulseFactor);
        servoWrite(legPins[i*3 + 2], servoPulseOffset[i*3 + 2] + c*angleToPulseFactor);
    }

    putln();

    ROM_TimerIntClear(WTIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void statusInterrupt(void)
{
    // TODO: Send battery voltage, other status info

    ROM_TimerIntClear(WTIMER0_BASE, TIMER_TIMB_TIMEOUT);
}
