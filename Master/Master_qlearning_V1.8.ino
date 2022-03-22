//version1.4-更新:

//1. 將 void loop()的setup_rewardtable和qlearning之間 移到 qlearning後
//2.void setup() solve eeprom.read problem
//3.新增 global variable:ai_action ,因此 改變 void aifan(), float max() 
//4.將done_reward前的判斷式塞入邊界條件應對方法

//version1.6更新
//AIFAN補上LOOP
//新增button1234
//新增測試使用者滿意溫度
//bug:

//reward不能大於255，改用abs(110-gi-fi)

//version 1.7
//1.實裝 set使用者溫度function於loop內
//2.修改reward機制，並配合該機制於done_reward內新增new_state_index
//3.將loop內的switch換成if elseif 
//version1.8
//1.將設定q r sts的state從setup移到setup_comfortable_temp內
//2.將reward修正成( 100-abs_2(10-new_state_index) )

//version 1.8
//1.將done_reward的想吹熱但吹冷即想吹冷但吹熱的old_state改成 state_index，goal_temp改成10，將new_state改成new_state_index
//新增DHT11，將溫度改成體感溫度


#include <SoftwareSerial.h>   // 引用程式庫
#include <math.h>         //包含数学库
#include <EEPROM.h>
#include "DHT.h"
#define dhtPin 10      //讀取DHT11 Data
#define dhtType DHT11 //選用DHT11  
DHT dht(dhtPin, dhtType);
SoftwareSerial BT(8, 9); // 接收腳, 傳送腳
const int button1Pin = 2;//(黑)
const int button2Pin = 3;//(紅)
const int button3Pin = 4;//(棕)
const int button4Pin = 5;//(長)

int flag = 0;//  flag = 0 代表建立reword table , flag = 1 代表qlearning中 , flag = 2 代表智慧電扇
float goal_temp = 27.0; //使用者舒適溫度
struct table { 
  float state;
  float action[3] = {0}; /*action[0]代表弱, action[1]代表中 , action[2]代表強*/
};
float alpha = 0.3 , Gamma = 0.9;
table qtable[21];// qlearning table
table reward_table[21]; // 獎賞table
table STS_table[21]; //  the table that describe A State use B action to what state
int ai_action = 0;

void setup() {
  // put your setup code here, to run once:
  flag = EEPROM.read(63);//開頭先將flag寫入
  Serial.begin(9600);
  BT.begin(9600); 
  dht.begin();//啟動DHT
  randomSeed(analogRead(A1));
  pinMode(button1Pin, INPUT);  
  pinMode(button2Pin, INPUT);  
  pinMode(button3Pin, INPUT);  
  pinMode(button4Pin, INPUT);  


  if(flag == 1 ){ //將qtable讀入
    for(int i=0;i<21;i++)//新增的 但是program run第一次時還未寫入 先註銷
       for(int j=0;j<3;j++)
        qtable[i].action[j]=EEPROM.read(3*i+j);//EEPROM.read(address);
    flag=EEPROM.read(63);//新增的
  }
//  float set_state_temp = -2; //實驗方便用
//  for( int i = 0 ; i < 21 ; i++){ 
//    qtable[i].state = goal_temp + set_state_temp ;
//    reward_table[i].state = goal_temp + set_state_temp ;
//    STS_table[i].state = goal_temp + set_state_temp ;
//    set_state_temp += 0.2;   
//  }
}

void loop() {
      
      
//    set_comfortable_temp();
//    Serial.println("開始");
      setup_rewardtable();
//    Serial.println("reward_table做完了");
//    Serial.println("reward = ");
//    for(int i = 0 ; i < 21 ;i++ ){
//      for(int j = 0 ; j < 3 ; j++ ){
//        Serial.print(reward_table[i].action[j]);
//      }
//      Serial.println(" ");
//    }
//    Serial.println("STS = ");
//    for(int i = 0 ; i < 21 ;i++ ){
//      for(int j = 0 ; j < 3 ; j++ ){
//        Serial.print(STS_table[i].action[j]);
//      }
//      Serial.println(" ");
//    }
//  



//  if( flag == 0 || flag == 255 ){
//    set_comfortable_temp();
//    setup_rewardtable();
//    //做完後寫入eeprom
//    qlearning();
//    for(int i=0;i<21;i++)
//      for(int j=0;j<3;j++)
//        EEPROM.write(3*i+j,qtable[i].action[j]); //EEPROM.write(address,value) 其中 address範圍0-255, 每個address寫入極限100000次     
//  }
//  else if ( flag == 1 ){
//    AI_FAN();    
//  }
//  else{
//    Serial.println("flag狀態不明");    
//  }
  delay(1000);
   
}

void setup_rewardtable(){ //建立rewardtable之主程式
  int timecount = 0; // 依次timecount++代表吹10秒
  int state_index; // 某溫度(state)下在table的哪個一index，如reward_table[10] = 28.4，則若此時某state溫度為28.4則該state對應之state_index為10
  float old_state = Temperature(0);

  while( timecount++ < 90 ) { //即做15分鐘
//    Serial.print("old_state " );
//    Serial.println( old_state );
    int action = chooseAction_reward( old_state ); //選動作
//    Serial.print("action = " );
//    Serial.println( action );
//    BT.print(3);
    BT.print(action + 1); //傳送給從端要求他吹action , 注意傳送的可能為int型態轉成char型態，還未測試
    delay(10000);//吹十秒
    float new_state = Temperature(action);
//    Serial.print("new_state = ");
//    Serial.println( new_state );
    done_reward( old_state , new_state, action ); // 知道old_state吹action10秒後得到new_state，並將資訊填入rewardtable和 STStable
    old_state = new_state ;
    Serial.print("timecount =  " );
    Serial.println( timecount );
    Serial.println(" ");
  }
}

float Temperature(int action){
  float total = 0;
  float a_temp = 0;
  float V = 0;
  //平均20次
  for (int i= 0 ; i < 20 ; i++ ){
    float temp = dht.readTemperature();
    Serial.print("temp = ");
    Serial.println(temp);
    total += temp;
    delay(50); 
  }
  a_temp = total/20;
  Serial.print("average_temp = ");
  Serial.println(a_temp);
  Serial.print("action = ");
  Serial.println(action);
  switch(action){ //-1關 0弱 1中 2強
    case -1:
      V = 0;
      break;
    case 0:
      V = 4;
      break;
    case 1:
      V = 4.4;
    case 2:
      V = 5;
      break;
  }
  float RH = dht.readHumidity();
  float e = RH/100*6.105*exp(17.27*a_temp/(237.7+a_temp));
  float AT = 1.07*a_temp+0.2*e-0.65*V-2.7 ;
  Serial.print("體感溫度 = ");
  Serial.println(AT);
  return AT;
}

int chooseAction_reward( float old_state ){ //選擇哪一個action
  // algorithm 若先選action reward為0的，若皆不為0隨機選強中弱
  int action = -1; 
  int state_index = FindIndex( old_state );
  if(state_index == -1)//新增的 低於table，太冷 
    return -1;
  else if(state_index == -2)
    return 2;
  for( int i = 0 ; i < 3 ; i++ ){
    if( reward_table[ state_index ].action[i] == 0){
      return i;
    }   
  }
  return random(3);

}

void done_reward( float old_state , float new_state , int action_index  ) {
  int reward = 0;
  int state_index = FindIndex(old_state);
  int new_state_index = FindIndex( new_state) ;
//  Serial.print("donereward old_state=");
//  Serial.println( goal_temp-new_state );
//  Serial.print("donereward new_state=");
//  Serial.println(new_state);
//  Serial.print("state_index = ");
//  Serial.println(state_index); //　在監控視窗裡顯示"temp"的值  
  if( state_index == -1 ) //old_state超出適當範圍，不宜寫入reward_table
  {
    STS_table[state_index].action[action_index]=qtable[0].state;
    reward_table[state_index].action [action_index] = -1;
    return ;
    }
   else if ( state_index == -2 )
   {
    STS_table[state_index].action[action_index]=qtable[20].state;
    reward_table[state_index].action [action_index] = -1;
    return;
   }  
  STS_table[state_index].action[action_index] = new_state; //存入delta_table
  //想吹冷但越吹越熱
  if( state_index > 10 && new_state_index > state_index )
    reward_table[state_index].action [action_index] = -1;
  //想吹熱但越吹越冷
  else if ( state_index < 10 && new_state_index < state_index )
    reward_table[state_index].action [action_index] = -1;
  else
    reward_table[state_index].action[action_index] = ( 100-abs_2(10-new_state_index)*10 );
  Serial.println("reward_table = ");
  for(int i = 0 ; i < 21 ;i++ ){
    for(int j = 0 ; j < 3 ; j++ ){
      Serial.print(reward_table[i].action[j]);
      Serial.print("  ");
    }
    Serial.println("");
  }
}

//
void qlearning(){ //訓練1000次
  //開頭隨機從21個state中選其中一個
  float old_state  = qtable[ random(21)].state ;
  int oldstate_index = FindIndex(old_state);
  
  float new_state = 0 ;
  int newstate_index = 0;
  
  int q_count = 0 ; //進行幾次qlearing更新
  while( q_count < 10000){
    int action_index =  chooseAction_q(old_state ); 
    new_state = STS_table[ oldstate_index ].action[ action_index ];
    
    float extrome=random(1000)/1000;//新增
    if(extrome>=0.9)//新增greedy
      new_state=Max( oldstate_index);
      
    newstate_index = FindIndex( new_state );  
    qtable[ oldstate_index ].action[ action_index ] = ( 1 - alpha )*qtable[ oldstate_index ].action [action_index] 
                                                      + alpha*( reward_table[oldstate_index].action[action_index]
                                                      + Gamma*Max( newstate_index) );   
    old_state = new_state;
    oldstate_index = newstate_index; 
  }
  flag = 1;//新增
  EEPROM.write(63,1) ;//version1.3 第五點
}

int chooseAction_q ( float old_state ){
  int positive_count = 0;
  int positive_value[3] = {-1};
  int state_index = FindIndex (old_state) ;
  if( state_index == -1)//低於table，太冷
    return 0;
  if( state_index == -2)
    return 2;  
  for( int i = 0 ; i < 3 ; i++){
    if( qtable[ state_index ].action[i] >= 0){
      positive_value[ positive_count ] = i;
      positive_count++;
    }
    return positive_value[ random( positive_count ) ];
  }
}
void AI_FAN(){

  while( digitalRead(button4Pin) == LOW ) {
    float old_temp = Temperature(ai_action);
    int state_index=FindIndex(old_temp);
    if(state_index == -1)
      BT.print(1);
    else if(state_index == -2)
      BT.print(3);
    else{
      ai_action = Max(state_index );
      BT.print(ai_action+1);
    }
    delay(10000);
  }
  flag = 0;
  EEPROM.write(63,0) ;
  
}

int FindIndex ( float s){ //此輸入一個state(溫度)給此function，則此function會回傳該溫度是在table的哪一個index
  if( s < qtable[0].state ){
    return -1;
    Serial.println("溫度小於qlearning範圍");
  }
  if( s > qtable[20].state+0.2 ){ //version1.3 第六點
    return -2;
    Serial.println("溫度大於qlearning範圍");
  }
//  Serial.print("s = ");
//  Serial.println(s);

  //s為適當溫度
  for( int index = 0 ; index < 21 ; index++){
    if ( s >= qtable[index].state && s < qtable[index].state+0.2 ){
//      Serial.print("index = ");
//      Serial.println(index);
      return index;
    }
  }
}

float Max( int newstate_index ){//找尋該某state下的最大q值
  int max_index = 0;
  for( int action_index = 0 ; action_index < 3 ; action_index++ ){
    if ( qtable[newstate_index].action[action_index] > qtable[newstate_index].action[max_index] )
      max_index = action_index;
  }
  ai_action=max_index;
  return qtable[newstate_index].action[max_index];
}

float  abs_2 ( float value ) {
  if ( value >= 0 ){
    return value;
  }
  else{
    return value*-1;
  }
}

void set_comfortable_temp(){ //button1是減弱 (黑) //button2是增強(紅) //button3(棕)是確認
  Serial.println("button1是減弱 (黑) //button2是增強(紅) //button3(棕)是確認") ;
  Serial.println("關=0 , 弱=1 , 中=2 , 強=3 ");
  int fan_speed = 1;
  int down = 0;
  int up = 0;
  while ( digitalRead(button3Pin) != HIGH ){
    down = digitalRead(button1Pin);
    up = digitalRead(button2Pin);
    if( down == 1 && fan_speed-1 >= 0 ){
      fan_speed -= 1;
      Serial.print("目前風速: ");
      Serial.println(fan_speed);
      BT.print(fan_speed);
    }
    else if ( up == 1 && fan_speed+1 <= 3 ){
      fan_speed += 1;
      Serial.print("目前風速: ");
      Serial.println(fan_speed);
      BT.print(fan_speed);      
    }
    delay(1000);    
  }
  goal_temp = Temperature(fan_speed);
  Serial.println("設定好了，goal temp = ");
  Serial.println(goal_temp);
  //設定sts 和 reward_table的state值
  float set_state_temp = -2;
  for( int i = 0 ; i < 21 ; i++){ 
    qtable[i].state = goal_temp + set_state_temp ;
    reward_table[i].state = goal_temp + set_state_temp ;
    STS_table[i].state = goal_temp + set_state_temp ;
    set_state_temp += 0.2;   
  }
}
