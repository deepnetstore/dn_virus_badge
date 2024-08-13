# Deepnet virus badge 2024

ESP32-S3 Interactive Art Badge
Welcome to the info page of the DN Virus Badge 2024.

Features:
    OLED Display connected via i2c  
    3 Input Buttons (side mounted)  
    LED Matrix Grid   
    LED Ring  
    Boot and Reset Buttons  
    Battery and Charging (optional)  
    Motion Sensor with LED time out settings.  
    Bonus 'Extras' 🙂  

The Game:
Its a very simple game, (because I am not that sofisticated... or had enough time..)  
each unit broadcasts using the esp-now protocal with a basic struct of data including a random integer value and the health level of the broadcasting unit.  
We decided to use this to represent and over simplified model of biological virus transmissions, as radio waves of this protocol only transmit a moderate distance.  
Any device that receives a broadcast message will then play a game similar to the classic card game of war. The broadcasting unit will randomly chose a number within a set range, while the receiving unit will have a slight advantage and select from a slightly larger number range.  
The devices will then lose or gain health points based on the win or loss of the game. The value of health gained or lost will be randomized with in a set range.  
After this game, the devices will play one more game based on if they are considered 'infected' or not.  
An infected device is simply one with a health value of 0 or less. The lowest health value is -100, while the highest value is 100.  
The device will display its health value in the OLED display as will as with the LED colors set on the matrix grid leds. -100 health will light the grid with RED while a health value of 100 will turn the LEDS green. 😎  
Score is assigned for select events and will only continue to increase. This could be a fun thing to compare with other users at the end of the event. Devices that are powered on and actively playing longest will have the best chance for high scores.  

Hacking:
This device is using a very simple protocol and transmiting a fairly basic data structure.  
We encourage you to sniff the air and see what kind of transmissions could affect outcomes in the game. Good luck!  
If you want to re-program it after defcon, the pin numbers are listed on the inside of the PCB. The code used to show leds on the grid and ring will be provided in the deepnet.store github.