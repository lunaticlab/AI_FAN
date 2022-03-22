#include <SoftwareSerial.h>   // 引用程式庫
#include <Servo.h>
#include<string.h>
// 建立一個 Servo 物件

Servo myservo;
 
// 定義連接藍牙模組的序列埠
SoftwareSerial BT(8, 9); // 接收腳, 傳送腳

char blue_value[5] = {0}  ;
float motor_angle = 1;
float angle = 0;
int available_count = 0;
 
void setup() {
  Serial.begin(9600);
  // 設定HC-05藍牙模組，AT命令模式的連線速率。
  BT.begin(9600);
  myservo.attach(7);  // Servo 接在 pin 7
}
 
void loop() {
  
    while ( angle <= 180 ){
       while( BT.available() > 0 ){
       Serial.println("等");
       if( (blue_value[available_count] = BT.read() ) >= '0' && blue_value[available_count]  <= '9'){
//              Serial.println( blue_value[available_count] );
             available_count++;
             if( available_count == 4 ){
                    motor_angle = 0;
                    available_count = 0;
                    int cnt = 1;
                    for( int i = 1 ; i < 4 ; i++ ){
                        motor_angle += ((int)blue_value[i]-48 ) / cnt;
                        cnt *= 10 ;
                    }
//               Serial.println(motor_angle);
               break;
              }
       }
    }
    Serial.println("跑");
    Serial.println(motor_angle);
    angle += motor_angle;
    myservo.write(angle);
    delay(15);
    }

    while ( angle >=0 ){
       while( BT.available() > 0 ){
       if( (blue_value[available_count] = BT.read() ) >= '0' && blue_value[available_count]  <= '9'){
//              Serial.println( blue_value[available_count] );
             available_count++;
             if( available_count == 4 ){
                    motor_angle = 0;
                    available_count = 0;
                    int cnt = 1;
                    for( int i = 1 ; i < 4 ; i++ ){
                        motor_angle += ((int)blue_value[i]-48 ) / cnt;
                        cnt *= 10 ;
                    }
//               Serial.println(motor_angle);
               break;
              }
       }
    }
    Serial.println(motor_angle);
    angle -= motor_angle;
    myservo.write(angle);
    delay(15);
    }

}
