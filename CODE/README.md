# How to upload code to the DN_Virus Badge
#### This project uses PlatformIO

Please follow the guides here [https://docs.platformio.org/en/latest/core/installation/index.html] to set up PlatformIO.  
We recommend using the VSCode installation method.  

Once PlatformIO is installed, both the code and the file system will need to be built and flashed.  
First build and flash the code, then build and upload the file system.  

### Note: BOOT MODE
In order to flash the ESP32S3, the BOOT button must be pressed and then held while pressing and releaing the Reset button.  
These buttons can be found on the right side of the badge. If there is any issue uploading, retry the previous reset step.  

Follow this guide to upload code using PlatformIO: [https://randomnerdtutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/#5]  
Follow this guide to build and upload the filesystem: [https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/]  
