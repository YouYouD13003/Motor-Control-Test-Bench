# Motor-Control-Test-Bench
Motor Control Test Bench - STM32 Microcontroller
This project involves the development of a test bench system based on an STM32 microcontroller, designed to control and monitor a motor using various integrated features such as ADC, PWM, and SPI communication. The system is capable of real-time monitoring and adjustment of motor speed, making it suitable for industrial applications.

Key Features:

ADC Watchdog: Monitors the ADC value to ensure it stays within a defined range. Triggers an emergency reset using an independent watchdog if the threshold is exceeded.
PWM Control: Motor speed is controlled through a PWM signal, which is adjusted using a potentiometer connected to the ADC.
Real-Time Speed Display: The current motor speed, represented by the ADC value, is displayed on a 7-segment display via SPI communication.
LED Indicators: Includes 8 LEDs that light up in a gauge-like effect, corresponding to the motor's PWM speed and the displayed speed. This provides a dynamic visual representation of the motor's performance.
Low Power Mode: Implemented power-saving techniques to ensure efficient operation, including external interrupts to manage power states.
Emergency Handling: Includes an independent watchdog that resets the system in case of an emergency, ensuring reliable operation in industrial environments.
User Interface: Features LED indicators and push buttons for additional control and status monitoring.
