#include <Wire.h>
#include <constants.h>

int typeOfSensor = 0;
int maxCurrent = 0;
int noOfChannel = 0;

unsigned int currentSensorBuff[36];

void setupCurrentSensor() {
  // Start I2C transmission
  Wire.beginTransmission(PECMAC125A_ADDR);
  // Command header byte-1
  Wire.write(0x92);
  // Command header byte-2
  Wire.write(0x6A);
  // Command 2 is used to read no of sensor type, Max current, No. of channel
  Wire.write(0x02);
  // Reserved
  Wire.write(0x00);
  // Reserved
  Wire.write(0x00);
  // Reserved
  Wire.write(0x00);
  // Reserved
  Wire.write(0x00);
  // CheckSum
  Wire.write(0xFE);
  // Stop I2C transmission
  Wire.endTransmission();

  // Request 6 bytes of data
  Wire.requestFrom(PECMAC125A_ADDR, 6);

  // Read 6 bytes of data
  if (Wire.available() == 6) {
    currentSensorBuff[0] = Wire.read();
    currentSensorBuff[1] = Wire.read();
    currentSensorBuff[2] = Wire.read();
    currentSensorBuff[3] = Wire.read();
    currentSensorBuff[4] = Wire.read();
    currentSensorBuff[5] = Wire.read();
  }

  typeOfSensor = currentSensorBuff[0];
  maxCurrent = currentSensorBuff[1];
  noOfChannel = currentSensorBuff[2];

  // Output data to serial monitor
  Serial.print("Type Of Sensor : ");
  Serial.println(typeOfSensor);
  Serial.print("Max Current : ");
  Serial.print(maxCurrent);
  Serial.println(" Amp");
  Serial.print("No. Of Channel : ");
  Serial.println(noOfChannel);

  delay(300);
}

float readCurrentSensor(int channel) {
  // Start I2C Transmission
  Wire.beginTransmission(PECMAC125A_ADDR);
  // Command header byte-1
  Wire.write(0x92);
  // Command header byte-2
  Wire.write(0x6A);
  // Command 1
  Wire.write(0x01);
  // Start Channel No.
  Wire.write(channel);
  // End Channel No.
  Wire.write(channel);
  // Reserved
  Wire.write(0x00);
  // Reserved
  Wire.write(0x00);
  // CheckSum
  Wire.write((0x92 + 0x6A + 0x01 + channel + channel + 0x00 + 0x00) & 0xFF);
  // Stop I2C Transmission
  Wire.endTransmission();
  vTaskDelay(500);

  // Request 3 bytes of data
  Wire.requestFrom(PECMAC125A_ADDR, 3);

  // Read 3 bytes of data
  // msb1, msb, lsb
  int msb1 = Wire.read();
  int msb = Wire.read();
  int lsb = Wire.read();
  float current = (msb1 * 65536) + (msb * 256) + lsb;

  // Convert the data to ampere
  current = current / 1000;

  return current;
}