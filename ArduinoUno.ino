#include "SdFat.h"
#include "sdios.h"
#define SD_FAT_TYPE 2
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else   // SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN
#define SPI_CLOCK SD_SCK_MHZ(50)

#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS


SdExFat sd;
ExFile file;
void setup() {
/*
  TESTING VARIABLES AND PROCEDURES COMMENTED OUT
  
  String myStr2 = "A Brand New Test!";
  int myStr2_len = (myStr2.length() + 1);
  char buffer2[myStr2_len];
  myStr2.toCharArray(buffer2, myStr2_len);
*/
  Serial.begin(9600);
  if (!sd.begin(SD_CONFIG)) {
    Serial.println("Initialization failed!");
  } else {
    Serial.println("Initialized!");
  }

  if (sd.exists("data.txt")) {
    sd.remove("data.txt");
    Serial.println("Deleted last run!");
  }
  
/*
 MORE TESTING PROCEDURES COMMENTED OUT

  if (sd.exists("test.txt")) {
    sd.remove("test.txt");
    Serial.println("Deleted test.txt!");
  }

  if (!file.open("test.txt", O_WRONLY | O_CREAT | O_APPEND)) {
    Serial.println("Could not open for writing!");
  } 
  file.write("A test line for test.txt\n");
  file.close();

  if (!file.open("test.txt", O_WRONLY | O_APPEND)) {
    Serial.println("Could not open for writing!");
  } 
  file.write("A second test line for test.txt\n");
  file.close();

  if (!file.open("test.txt", O_WRONLY | O_APPEND)) {
    Serial.println("Could not open for writing!");
  } 
  file.write(buffer2, myStr2_len);
  file.close();

  if (!file.open("test.txt", O_RDONLY)) {
    Serial.println("Could not open for reading!");
  }
  file.rewind();
  while (file.available()) {
    n = file.fgets(buf, sizeof(buf));
    if (n <= 0) {
      Serial.println("fgets failed");
    } else {
      Serial.print(buf);
    }
  }
*/
}


void loop() {
   String myStr;
   char a = ' ';
   while (Serial.available() > 0) {
     myStr = Serial.readStringUntil(a);
     int myStr_len = (myStr.length() + 1);
     char buffer[myStr_len];
     myStr.toCharArray(buffer, myStr_len);
     Serial.println(buffer);
     if (!file.open("data.txt", O_WRONLY | O_CREAT | O_APPEND)) {
       Serial.println("Could not open for writing!");
     } else {
       file.write(buffer);
       file.close();
     }
   }
}
