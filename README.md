# dn_virus

https://deepnet.store/pages/dn_virus

ESP32-S3 Interactive Art Badge
~~~
Features:
• ESP32-S3FN8
• Lots of LEDs
• 6-Axis Motion Sensor
• Ships with Interactive 'Game'
• OLED Display connected via I2C
• 3 Side-Mounted Input Buttons
• Boot and Reset Buttons
• USB-C Charging | Battery (optional)
• Motion Sensor with LED Timeout Settings
• Bonus 'Extras' 🙂 🦠
~~~
The Game:
This simple game uses the ESP-NOW protocol to transmit a basic set of data, including a random integer value and the health level of the broadcasting unit. It is designed as an oversimplified model of biological virus transmission.

Here's how it works:

Broadcasting:  
Each unit transmits data using the ESP-NOW protocol, which only covers a moderate distance.
Data includes a random integer value and the health level of the broadcasting unit.

Game Mechanics:  
When a device receives a broadcast message, it plays a game similar to the classic card game "War".
The broadcasting unit selects a random number within a set range.
The receiving unit, with a slight advantage, selects from a slightly larger range.
Health points are gained or lost based on the game's outcome, randomized within a set range.

Infection Status:  
If a device's health value is 0 or less, it is considered 'infected'.
Health values range from -100 to 100.
The OLED display and LED matrix show the health status. -100 health lights the grid red, while 100 health turns the LEDs green. 😎

Scoring:  
Points are assigned for specific events and increase continuously, allowing for comparison with other users at the event.
Devices powered on and actively playing the longest have the best chance for high scores.

~~~
Pin Definitions:
Boot: GPIO0
Neopixel Grid: GPIO11
Neopixel Ring: GPIO17
Button DOWN [SW3]: GPIO36
Button CENTER [SW2]: GPIO37
Button UP [SW1]: GPIO38
I2C SDA: GPIO33
I2C SCL: GPIO34
IMU Interrupt1: GPIO26
IMU Interrupt2: GPIO35
IMU Addr: 0xD5U
Battery Level: GPIO14
On Board LED: GPIO15
On Board LED: GPIO15
~~~

To update your badge, follow the directions found in CODE/README.md

