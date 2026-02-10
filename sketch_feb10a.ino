/*
  Combined Sensor Reader with Moving Average
  Hardware: Arduino Nano 33 BLE Sense
  Sensors: HTS221 (Temp/Hum) & LSM9DS1 (Gyroscope)
*/

#include <Arduino_HTS221.h>
#include <Arduino_LSM9DS1.h>

// --- Configuration ---
const int SAMPLE_WINDOW = 10;  // Window size for moving average

// --- Global Variables for History ---
// Arrays to store the last 10 samples
float tempHistory[SAMPLE_WINDOW];
float humHistory[SAMPLE_WINDOW];
float gyroXHistory[SAMPLE_WINDOW];
float gyroYHistory[SAMPLE_WINDOW];
float gyroZHistory[SAMPLE_WINDOW];

int readIndex = 0;      // Current position in the array
int totalReadings = 0;  // Total samples taken (to handle the first 10 seconds)

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // 1. Initialize HTS221 (Temp/Hum)
  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }

  // 2. Initialize LSM9DS1 (IMU/Gyro)
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // Initialize arrays to 0
  for (int i = 0; i < SAMPLE_WINDOW; i++) {
    tempHistory[i] = 0;
    humHistory[i] = 0;
    gyroXHistory[i] = 0;
    gyroYHistory[i] = 0;
    gyroZHistory[i] = 0;
  }
  
  Serial.println("Sensors initialized. Starting sampling...");
  Serial.println("------------------------------------------------");
}

void loop() {
  // --- Step 1 & 2: Collect Data ---
  float currentTemp = HTS.readTemperature();
  float currentHum  = HTS.readHumidity();
  
  float currentGyroX, currentGyroY, currentGyroZ;
  IMU.readGyroscope(currentGyroX, currentGyroY, currentGyroZ);

  // --- Store Data in History Arrays ---
  tempHistory[readIndex] = currentTemp;
  humHistory[readIndex]  = currentHum;
  gyroXHistory[readIndex] = currentGyroX;
  gyroYHistory[readIndex] = currentGyroY;
  gyroZHistory[readIndex] = currentGyroZ;

  // --- Calculate Averages ---
  // If we haven't collected 10 samples yet, divide by the actual number of samples collected
  int count = (totalReadings < SAMPLE_WINDOW) ? totalReadings + 1 : SAMPLE_WINDOW;
  
  float avgTemp = calculateAverage(tempHistory, count);
  float avgHum  = calculateAverage(humHistory, count);
  float avgGyroX = calculateAverage(gyroXHistory, count);
  float avgGyroY = calculateAverage(gyroYHistory, count);
  float avgGyroZ = calculateAverage(gyroZHistory, count);

  // --- Step 4: Display Values ---
  Serial.print("T: "); Serial.print(currentTemp); Serial.print(" C (Avg: "); Serial.print(avgTemp); Serial.print(") | ");
  Serial.print("H: "); Serial.print(currentHum); Serial.print(" % (Avg: "); Serial.print(avgHum); Serial.println(")");
  
  Serial.print("Gyro X: "); Serial.print(currentGyroX); Serial.print(" (Avg: "); Serial.print(avgGyroX); Serial.print(") | ");
  Serial.print("Y: "); Serial.print(currentGyroY); Serial.print(" (Avg: "); Serial.print(avgGyroY); Serial.print(") | ");
  Serial.print("Z: "); Serial.print(currentGyroZ); Serial.print(" (Avg: "); Serial.print(avgGyroZ); Serial.println(")");
  
  Serial.println("-"); // Separator

  // --- Advance Buffer ---
  readIndex = (readIndex + 1) % SAMPLE_WINDOW; // Wrap around to 0 if we hit 10
  totalReadings++;

  // --- Step 3: Wait 1 Second ---
  delay(1000);
}

// Helper function to sum array and divide by count
float calculateAverage(float data[], int count) {
  float sum = 0;
  // We strictly iterate up to the SAMPLE_WINDOW size or current valid count
  // Note: When buffer is full, we sum the whole array.
  // When buffer is filling, we only sum the valid indices if we tracked them strictly,
  // but simpler here is to iterate the whole array (since unused slots are 0) 
  // *IF* we were not using a circular buffer logic. 
  // Because we are using circular buffer, the 'old' data is naturally overwritten.
  // For the very first pass (totalReadings < 10), we only sum the valid slots.
  
  int limit = (totalReadings < SAMPLE_WINDOW) ? totalReadings + 1 : SAMPLE_WINDOW;
  
  for(int i = 0; i < limit; i++) {
     sum += data[i];
  }
  return sum / limit;
}