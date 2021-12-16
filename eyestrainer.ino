#include <M5Core2.h>
#include <utility/M5Timer.h>
#include <driver/i2s.h>
#include <VL53L1X.h>
#include <Wire.h>





//setting the speaker
extern const unsigned char previewR [120264];

#define CONFIG_I2S_BCK_PIN 12 
#define CONFIG_I2S_LRCK_PIN 0
#define CONFIG_I2S_DATA_PIN 2
#define CONFIG_I2S_DATA_IN_PIN 34

#define Speak_I2S_NUMBER I2S_NUM_0  
#define MODE_MIC 0  
#define MODE_SPK 1
#define DATA_SIZE 1024

bool InitI2SSpeakOrMic(int mode){  //Init I2S.  
    esp_err_t err = ESP_OK;

    i2s_driver_uninstall(Speak_I2S_NUMBER); // Uninstall the I2S driver.  
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER),  // Set the I2S operating mode.  
        .sample_rate = 44100, // Set the I2S sampling rate.  
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // Fixed 12-bit stereo MSB.  
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // Set the channel format. 
        .communication_format = I2S_COMM_FORMAT_I2S,  // Set the format of the communication.
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Set the interrupt flag.  
        .dma_buf_count = 2, //DMA buffer count.
        .dma_buf_len = 128, //DMA buffer length.  
    };
    if (mode == MODE_MIC){
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
    }else{
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
        i2s_config.use_apll = false;  //I2S clock setup. 
        i2s_config.tx_desc_auto_clear = true; // Enables auto-cleanup descriptors for understreams. 
    }
    // Install and drive I2S. 
    err += i2s_driver_install(Speak_I2S_NUMBER, &i2s_config, 0, NULL);

    i2s_pin_config_t tx_pin_config;
    tx_pin_config.bck_io_num = CONFIG_I2S_BCK_PIN;  // Link the BCK to the CONFIG_I2S_BCK_PIN pin. 
    tx_pin_config.ws_io_num = CONFIG_I2S_LRCK_PIN;  //          ...
    tx_pin_config.data_out_num = CONFIG_I2S_DATA_PIN;  //       ...
    tx_pin_config.data_in_num = CONFIG_I2S_DATA_IN_PIN; //      ...
    err += i2s_set_pin(Speak_I2S_NUMBER, &tx_pin_config); // Set the I2S pin number.  
    err += i2s_set_clk(Speak_I2S_NUMBER, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO); // Set the clock and bitwidth used by I2S Rx and Tx. 
    return true;
}

void SpeakInit(void){ 
  M5.Axp.SetSpkEnable(true);
  InitI2SSpeakOrMic(MODE_SPK);
}

void DingDong(void){
  size_t bytes_written = 0;
  i2s_write(Speak_I2S_NUMBER, previewR, 120264, &bytes_written, portMAX_DELAY);
}




//setting the timer 
M5Timer M5T;

int mytimerA = -1;
int mytimerB = -1;

int mycountA = 0;
int mycountB = 0;

void myTimerACB(void)
{
  mycountA++;
}

void myTimerBCB(void)
{
  mycountB++;
}


//setting the distance sensor
VL53L1X sensor;
const uint8_t type = 2;// 1=mm , 2= cm; 3=inch (1 mm = 0.03937 inch)
char *unit[]={"mm","cm","in"};// variable for unit, mm, cm or in

float distanceCm, distanceIn;

double distance;



void Dist();
void PrintInfo();
void setup() {
  // put your setup code here, to run once:
  M5.begin();
  M5.Rtc.begin();
  Wire.begin();
  //Serial.begin(9600);

  //initializing the distance sensor
  sensor.init();
  sensor.setTimeout(500);
  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(50000);
  sensor.startContinuous(50);

  
  //Initial name of program
  M5.Lcd.setTextSize(4);  
  M5.Lcd.setCursor(0,100);
  M5.Lcd.printf("EyesTrainer");
  SpeakInit();
  DingDong();
  delay(5000);
  M5.Lcd.clear();

  //Draws a pair of eyes using circles
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.drawCircle(90,110,60, BLACK);
  M5.Lcd.drawCircle(230,110,60, BLACK);
  M5.Lcd.drawCircle(90 ,110,20, BLACK);
  M5.Lcd.fillCircle(90, 110, 20, BLACK); 
  M5.Lcd.drawCircle(230,110,20, BLACK);
  M5.Lcd.fillCircle(230,110,20, BLACK); 

  M5.Lcd.drawCircle(100,100,10,  WHITE);
  M5.Lcd.fillCircle(100,100,10, WHITE);
  M5.Lcd.drawCircle(240,100,10,  WHITE);
  M5.Lcd.fillCircle(240,100,10, WHITE);

  delay(3000);
  M5.Lcd.clear();

  //message about what program is about
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setCursor(100,5);
  M5.Lcd.setTextDatum(TC_DATUM);
  M5.Lcd.print("EyeStrainer");
  M5.Lcd.setCursor(0,50);
  M5.Lcd.setTextSize(2);
  M5.Lcd.print("Hi, Buddy!\n\n");
  delay(2000);
  M5.Lcd.print("I'm your Personal\n\n");
  delay(2000);
  M5.Lcd.print("Eye Care Assistant.....");
  delay(5000);
  M5.Lcd.clear();
  
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,5);
  M5.Lcd.setTextColor(RED);
  M5.Lcd.print("To reduce eyestrain this\nprogram:\n\n");
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.printf("1. Tracks and corrects\nyour distance from the\nscreen.\n\n\n");
  M5.Lcd.printf("2. Adopts the 20-20-20\nrule.\n\n\n");
  delay(15000);
  M5.Lcd.clear();

  
  mytimerA = M5T.setTimer(1000, myTimerACB,1200);
  mytimerB = M5T.setTimer(1000, myTimerBCB, 20);
  
}

void loop() {
  // put your main code here, to run repeatedly:
    M5.update();
    distance = sensor.read();
    
    M5T.disable(mytimerB);
    M5T.run();
    PrintInfo();
    if(M5T.isEnabled(mytimerA)==false){
      DingDong();
    }
  while(M5T.isEnabled(mytimerA)==false){
      M5T.enable(mytimerB);
      M5T.disable(mytimerA);
      M5T.run();
      distance = sensor.read();
      PrintInfo();
     
      if(M5T.isEnabled(mytimerB)==false){
        DingDong();
        mycountA=0; 
        mycountB=0;
        M5T.enable(mytimerA);
        mytimerA = M5T.setTimer(1000, myTimerACB,1200);
        mytimerB = M5T.setTimer(1000, myTimerBCB, 20);
  
       while(M5T.isEnabled(mytimerB)==false){
        M5T.run();
        distance = sensor.read();
        PrintInfo();
  }
 } 
}
}


void Dist(){
  if(distance > 400.0){
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(3);
    M5.Lcd.print("Distance: ");            
    distanceCm =(float) (distance/10.0);//convert distanc to cm
    M5.Lcd.printf("%.1f", distanceCm);
    M5.Lcd.print(unit[type-1]);
  }
  else{
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.print("Alert!You're too close to the\nscreen!");
    DingDong();
    }
}

void PrintInfo(){
      Dist();
      M5.Lcd.drawFastHLine(0, 75, 320, WHITE); 
       
      M5.Lcd.setCursor(0, 80);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(BLUE);
      M5.Lcd.printf("Screen Time: %s\n", M5T.isEnabled(mytimerA) ? "enabled " : "disabled");
      M5.Lcd.setTextSize(3);
      M5.Lcd.setCursor(0, 110);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.printf("Screen Time:%04ds", mycountA);

      M5.Lcd.setCursor(0, 160);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(BLUE);
      M5.Lcd.printf("Break Time: %s\n", M5T.isEnabled(mytimerB) ? "enabled " : "disabled");
      M5.Lcd.setTextSize(3);
      M5.Lcd.setCursor(0, 190);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.printf("Break Time: %04ds", mycountB);
      delay(500);
      M5.Lcd.clear();
  
}
