#include <Wire.h>
#include <math.h>
// Shift the device's documented slave address (0x3C) for write operation
// 1 bit right.This compensates for how the TWI library only wants the
// 7 most significant bits (with the high bit padded with 0)
#define HMC5883_WriteAddress 0x1E // i.e 0x3C >> 1
#define HMC5883_ModeRegisterAddress 0x02
#define HMC5883_ContinuousModeCommand 0x00
#define HMC5883_DataOutputXMSBAddress 0x03
int regb=0x01;
int regbdata=0x40;
int outputData[6];
String myPosition = "";
//Wi-Fi
#include <SPI.h>
#include <WiFi.h>
// 引用 Servo Library
#include <Servo.h>
// 建立一個 Servo 物件
Servo left_servo;
Servo right_servo;

char ssid[] = "1234"; //  your network SSID (name)
char pass[] = "12345678";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;

// Initialize the Wifi client library
WiFiClient client;
// server address:
char server[] = "140.136.150.79";
//IPAddress server(64,131,82,241);
unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
boolean lastConnected = false;                  // state of the connection last time through the main loop
const unsigned long postingInterval = 10 * 1000; // delay between updates, in milliseconds
//client.println("GET /map.php?C_North=25.014051&C_East=121.463814&D_North=25.0363278&D_East=121.432772");

//濟時樓 25.0358126,121.4304974
//聖言樓 25.0354515,121.4315136
//輔大正門 25.0326725,121.4340881
//老船橋頭凍檸茶輔大店(514) 25.0345301,121.4303575

String D_Norths [] = {"25.0358126", "25.0354515", "25.0326725", "25.0345301"};
String D_Easts  [] = {"121.4304974", "121.4315136", "121.4340881","121.4303575"};
String C_Norths [] = {"25.0358126", "25.0354515", "25.0326725", "25.0345301"};
String C_Easts  [] = {"121.4304974", "121.4315136", "121.4340881","121.4303575"};

String D_North = D_Norths[0];
String D_East = D_Easts[0];
String C_North = C_Norths[0];
String C_East = C_Easts[0];
String dctn = "";

// give it a name:
int leds[] = {10, 11, 12, 13};
String dir[] = {"north","northeast","east","southeast", "south", "southwest", "west", "northwest"};
int MAX_LENGTH = 4;

int pointNum=0;
int curPoint = 0;
String N[100];
String E[100];
String closer = "";

// 初始化起始面對方向
String real_dir_str = "north";
boolean first = true;

// 按鈕選取地點
const int NUM_OF_BUTTON = 4;
const int buttonPin[ NUM_OF_BUTTON ] = { 2, 3, 4, 6 };     // the number of the pushbutton pin
// variables will change:
int buttonState[ NUM_OF_BUTTON ] = {0};         // variable for reading the pushbutton status

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  Serial1.begin(9600);
    //Initialize serial and wait for port to open:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if ( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  // print out the status
  printWifiStatus();
  
  // initialize the digital pin as an output(LED).
//  for(int led = 0; led < MAX_LENGTH; led++)
//    pinMode(leds[led], OUTPUT);

  for(int i = 0; i < NUM_OF_BUTTON; i++)
    pinMode(buttonPin[i], INPUT);
  
 //馬達的PIN
 left_servo.attach(9);  // 右邊馬達接在 pin 9
 right_servo.attach(5);  // 右邊馬達接在 pin 5
}
char * str1;
char * arr1;
char * delimeter = ",";
char * theCheck = "0";
String GPSLL[2];

void loop()
{
  while(arrayGPS() != true){
    // 需要得到GPS定位
    delay(1);
  }
  C_North = GPSLL[0];
  C_East = GPSLL[1];
  
  Serial.println("Set Location From GPS");
  if(!first){
    Serial.println("Not First");
    Serial.println(curPoint);
    Serial.println(N[curPoint]);
    Serial.println(E[curPoint]);
    D_North = N[curPoint];
    D_East  = E[curPoint];
    tcpConnection(C_North, C_East, D_North, D_East, false);
  } else{
    for(int i=0 ; true; i++){
      // Button 選取目的地
      int check_bt = i % NUM_OF_BUTTON;
      buttonState[check_bt] = digitalRead(buttonPin[check_bt]);
      if(buttonState[check_bt] == HIGH){
        // 一定要選取目的地才跳出
        D_North = D_Norths[check_bt];
        D_East = D_Easts[check_bt];
        break;
      }
      delay(500);
    }
    tcpConnection(C_North, C_East, D_North, D_East, true);
  }
}

boolean arrayGPS() {
  int i = 0;
  String arr2[20];
  int counter = 0;
 
  if(Serial1.available()){
      String s = readLine();
      //s ="$GPGGA,081836,A,2503.63278,N,12143.2772,E,000.0,360.0,130998,011.3,E*62";
      str1 = &s[0];
      //Serial.print(str1);
      if (findGPGGA(str1)) {
        arr1 = strtok(str1, delimeter);  
        for (i = 0; arr1 != NULL; i++) {
          arr1 = strtok (NULL, delimeter);
          arr2[i] = arr1;
        }
        
        if (arr2[1] != theCheck && arr2[1] != "00") {
        // 有接到衛星的資料  
          if(arr2[2].charAt(0) == 'N'){
            // 北緯 數值為正數
            GPSLL[0] = arr2[1];
          } else{
            // 南緯 數值為負數
            GPSLL[0] ="-"+ arr2[1];
          }
          if(arr2[4].charAt(0) == 'E'){
            // 東經 數值為正數
            GPSLL[1] = arr2[3];
          }
          else{
            // 西經 數值為負數
            GPSLL[1] ="-" + arr2[3];
          }
          GPSLL[0] = shift_float(GPSLL[0]);
          GPSLL[1] = shift_float(GPSLL[1]);
          Serial.println(GPSLL[0] + "," + GPSLL[1]);
          return true;
        } else{
        // 沒有接到衛星資料
        }
      }
  }
  return false;
}
boolean findGPGGA(String str1) {
  if (str1.charAt(1) == 'G' & str1.charAt(2) == 'P' & str1.charAt(3) == 'G' & str1.charAt(4) == 'G' & str1.charAt(5) == 'A'){
    Serial.println(str1);
    return true;
  }
  else{
    return false;
  }
}
String readLine() {
  String s1 =  "";
  char c1;
  while (c1 != '\n') {
    if (Serial1.available()) {
      c1 = Serial1.read();
      s1 += c1;
    }
  }
  return s1;
}

//座標正規化
String shift_float(String str){
  char delimeter = '.';
  int slice = str.indexOf(delimeter);
  char str_buffer[str.length()];
  String integer_str = str.substring(0, slice-2);
  String rational_str = str.substring(slice-2, slice) + str.substring(slice+1);
  Serial.println(integer_str + "," + rational_str);
  rational_str.toCharArray(str_buffer, str.length());
  return integer_str + "." + rational_str;
}
// Open new TCP conneciton to server
void tcpConnection(String C_North, String C_East, String D_North, String D_East, boolean isFirst) {
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  dctn = "";
  if (client.available()) {
    // 連線成功，並且接到字串
    while (client.available()) {
      char c = client.read();
      dctn += c;
      Serial.write(c);
    }
    char *k = &dctn[0];
    // 字串處理
    if(!isFirst){
        Serial.println("Not First");
        closer = strtok(k, delimeter);
        if(closer == "true" &&  curPoint < pointNum){
          curPoint++;
        }else{
            // 到了終點要叫個 ，待完成。
        }
        arr1 = strtok (NULL, delimeter);  
    } else{
        Serial.println("First");
        String PointNumStr = strtok(k, delimeter);
        pointNum = PointNumStr.toInt();  
        Serial.println("total:");
        Serial.println(pointNum);
        for (int i = 0; i < pointNum; i++) {
          N[i] = strtok (NULL, delimeter);
          E[i] = strtok (NULL, delimeter);
        }
        first = false;
    }
    //dctn = dctnToken(dctn);
    real_dir_str = getMyPosition();
    Serial.println("__________________________Real Direction___________________________");
    Serial.println(real_dir_str);
    dctn = compute_direction(real_dir_str, dctn);
    Serial.println(real_dir_str);
    servo(dctn);
  }
  if (client.connected()  && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:

  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
  
  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if (!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    httpRequest(C_North, C_East, D_North, D_East, first);
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}


// this method makes a HTTP connection to the :
void httpRequest(String C_North, String C_East, String D_North, String D_East, boolean isFirst) {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    if(!isFirst){
      client.println("GET /map.php?C_North="+C_North+"&C_East="+C_East+"&D_North="+D_North+"&D_East="+D_East);
    }else{
      client.println("GET /first.php?C_North="+C_North+"&C_East="+C_East+"&D_North="+D_North+"&D_East="+D_East);
    }
    client.println("Host: 220.133.50.209");
    client.println("User-Agent: 220.133.50.209");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
int tagIndex(String str1) {
  int index = 2;
  for (; index < str1.length(); index++) {
    if (str1.charAt(index - 2) == '<' & str1.charAt(index - 1) == 'b' & str1.charAt(index) == '>') {
      return index;
    }
  }
  return -1;
}
/*String dctnToken(String dctn){
    return answer;
}*/
/*
// 顯示方向LED
void blink_LED(String cur_dir){
  int dir_index = 0;
  for(int i = 0; i < MAX_LENGTH; i++){
    if(cur_dir.indexOf(dir[i]) != -1){
      dir_index = i;
      digitalWrite(leds[dir_index], HIGH);   // turn the LED on (HIGH is the voltage level)
    }
  }
  delay(2000);                          // wait for a second
  for(int led = 0 ; led < MAX_LENGTH; led++)
    digitalWrite(leds[led], LOW);    // turn the LED off by making the voltage LOW
  delay(1000);     
}*/
//依據馬達反應方向
void servo(String Myposition){
  Serial.println();
  Serial.println(Myposition);
  left_servo.attach(7, 500, 2400);
  if(Myposition == "Turn_left"){
    // Turn left
    for(int y = 0;y <=3;y++){
      for(int i = 500; i <= 2400; i+=100){
        left_servo.writeMicroseconds(i); // 直接以脈衝寬度控制
        delay(10);
      }
      for(int i = 2400; i >= 800; i-=100){
        left_servo.writeMicroseconds(i);
        delay(10);
      }
    }
   } else if(Myposition == "Turn_right"){
     // Turn right
     for(int y = 0;y <=3;y++){
      for(int i = 500; i <= 2400; i+=100){
        right_servo.writeMicroseconds(i); // 直接以脈衝寬度控制
        delay(10);
      }
      for(int i = 2400; i >= 800; i-=100){
        right_servo.writeMicroseconds(i);
        delay(10);
      }
     }
   }else{
      //Forward 
      for(int y = 0;y <=3;y++){
          for(int i = 500; i <= 2400; i+=100){
          left_servo.writeMicroseconds(i); // 直接以脈衝寬度控制
          delay(10);
          right_servo.writeMicroseconds(i); // 直接以脈衝寬度控制
          delay(10);
        }
        for(int i = 2400; i >= 800; i-=100){
          left_servo.writeMicroseconds(i); // 直接以脈衝寬度控制
          delay(10);
          right_servo.writeMicroseconds(i);
          delay(10);
        } 
      }
      
   }
}

String compute_direction(String real_dir_str, String assign_dir_str){
  String dir[] = {"north", "northeast", "east", "southeast", "south", "southwest", "west", "northwest"};
  String action[] = {"Forward", "Turn_left", "Turn_right"};
  
  // 八方位的數字
  int real_dir = 0;
  int assign_dir = 0;
  // 避免影響運算的暫存值
  int temp_real_dir = 0;
  int temp_assign_dir = 0;
  // 指示下一步
  int assign_step;
  
  // 字串換算數字
  for(int i = 0; i < 8; i++){
    if(real_dir_str == dir[i]){
      real_dir = i;
    }
    if(assign_dir_str == dir[i]){
      assign_dir = i;
    }
  }

  if(real_dir != assign_dir){
    // 座標轉換，以實際面對方位為0座標
    temp_real_dir = 0;
    temp_assign_dir = assign_dir - real_dir;
    if(temp_assign_dir < 0) temp_assign_dir += 8;
    
    // 判斷在左半邊或右半邊，決定向左或向右
    temp_real_dir += 4;
    if(temp_real_dir > temp_assign_dir) assign_step = 2;
    else assign_step = 1;
  }else{
   // 兩者方向相等，向前
   assign_step = 0;
  }
  // 回傳下一步的方向
  return action[assign_step];
}

String getMyPosition(){
   int i,x,y,z;
   double angle;
   String myPosition = "";
   Wire.beginTransmission(HMC5883_WriteAddress);
   Wire.write(regb);
   Wire.write(regbdata);
   Wire.endTransmission();
   delay(1000);
   Wire.beginTransmission(HMC5883_WriteAddress); //Initiate a transmission with HMC5883 (Write address).
   Wire.write(HMC5883_ModeRegisterAddress); //Placethe Mode Register Address in write-buffer.
   Wire.write(HMC5883_ContinuousModeCommand); //Placethe command for Continuous operation Mode in write-buffer.
   Wire.endTransmission(); //writethe -buffer to HMC5883 and end the I2C transmission.
   delay(100);
   Wire.beginTransmission(HMC5883_WriteAddress); //Initiate a transmission with HMC5883 (Write address).
   Wire.requestFrom(HMC5883_WriteAddress,6); //Request 6 bytes of data from the address specified.
   delay(500);
   //Read the value of magnetic components X,Y and Z if(6 <= Wire.available()) // If the number of bytes available for reading be <=6.{
   for(i=0;i<6;i++)
   {
   outputData[i]=Wire.read(); //Store the data in outputData buffer}}
   x=outputData[0] << 8 | outputData[1]; //Combine MSB and LSB of X Data output register
   z=outputData[2] << 8 | outputData[3]; //Combine MSB and LSB of Z Data output register
   y=outputData[4] << 8 | outputData[5]; //Combine MSB and LSB of Y Data output register
  angle= atan2((double)y,(double)x) * (180 / 3.14159265)+ 180; // angle in degrees
   
  //Print the approximate direction
   if((angle < 22.5) || (angle > 337.5 ))
   myPosition = "south";
   if((angle > 22.5) && (angle < 67.5 ))
   myPosition = "southwest";
   if((angle > 67.5) && (angle < 112.5 ))
   myPosition = "west";
   if((angle > 112.5) && (angle < 157.5 ))
   myPosition = "northwest";
   if((angle > 157.5) && (angle < 202.5 ))
   myPosition = "north";
   if((angle > 202.5) && (angle < 247.5 ))
   myPosition = "northeast";
   if((angle > 247.5) && (angle < 292.5 ))
   myPosition = "east";
   if((angle > 292.5) && (angle < 337.5 ))
   myPosition = "southeast";
   delay(100);
   }
   return myPosition;
}

