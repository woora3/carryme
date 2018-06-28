// HCSR04Ultrasonic/examples/UltrasonicDemo/UltrasonicDemo.pde
#include <Ultrasonic.h>
int speakerPin = 4;
int buttonPin = 5;
#define TRIGGER_PIN  2
#define ECHO_PIN     3
#define G_LED 13
char notes[] = "ccggaagffeeddc ";
unsigned long beats[] = {1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4};
int length = sizeof(notes);
int tempo = 30;
Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);
int buttonState = 0;
int D = 75;
void setup()
{
  Serial.begin(9600);
  pinMode(speakerPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(G_LED, OUTPUT);
}

void loop()
{
  buttonState = digitalRead(buttonPin); //讀取按鈕的狀態
  if (buttonState == HIGH) {            //按鈕按下時為高電位
    //digitalWrite(G_LED, HIGH);         //輸出高電位給LED
    D = 75;
  }
  else {
    //digitalWrite(G_LED, LOW);          //輸出低電位給LED
    D = 120;
  }
  /*
  Serial.println(buttonState);
  Serial.println(D);
  delay(500);
  */
  float cmMsec, inMsec;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM); // 計算距離，單位: 公分
  if (cmMsec < D ) {
    playNote(speakerPin, notes[0], beats[0] * tempo);
  }
  Serial.print("CM: ");
  Serial.println(cmMsec);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
  delay(50);
}
void playNote(int OutputPin, char note, unsigned long duration) {
  // 音符字元與對應的頻率由兩個矩陣表示
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 261, 294, 330, 349, 392, 440, 494, 523 };
  // 播放音符對應的頻率
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      tone(OutputPin, tones[i], duration);
      //下方的 delay() 及 noTone ()，測試過後一定要有這兩行，整體的撥放出來的東西才不會亂掉，可能是因為 Arduino 送出tone () 頻率後會馬上接著執行下個指令，不會等聲音播完，導致撥出的聲音混合而亂掉
      delay(duration);
      noTone(OutputPin);
    }
  }
}
