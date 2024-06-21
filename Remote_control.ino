#include <stdio.h>
#include <BleKeyboard.h>
#include <string.h>
#include <temp_sensor.h>    // Chip温度监控
#include <esp_task_wdt.h>
#include <ESPAsyncWebServer.h>    // 包含异步Web服务器库文件
#include <WiFi.h>
#define LED1_BUILTIN 12
#define LED2_BUILTIN 13
#define LED3_BUILTIN 7       //LED端口配置
const int rs = 11, latchPin = 4, clockPin = 5, dataPin = 8;     //LCD端口配置
unsigned long startTime;        //定义变量以存储启动时的毫秒数
unsigned long lastUpdateTime;  // 用于跟踪上一次更新的时间
unsigned long updateInterval;  // 更新间隔为1秒
unsigned long Total_Key = 0;  // 总按键计数
const int PASSWORD_LENGTH = 8;  // 定义 WiFi 密码的长度
unsigned long buttonGPressStartTime = 0;      //L5输入G键按键开始时间
unsigned long lastDisconnectTime = 0;     //记录最后一次BLE掉线的时间

char version[] = "2.3.26";          //*************************版本信息*************************
const char *ssid = "Remote control"; //*************************热点名称*************************
const char *BLE_Address = "ecda3bd25a4a"; //*************************BLE地址*************************

int LED = 1, Connect_Check = 2, KeyA = 485, KeyB = 485, KeyC = 485, KeyD = 485, KeyMode = 485, KeyE = 485, KeyF = 485, KeyG = 485, Key_wifi = 485,
    Fast_Key1 = 50, Fast_Key2 = 50, Fast_Key3 = 50, Fast_Key4 = 50, Mode = 1, B_Wait = 0, Firest_Offline = 0,
    Lat_Off = 1, Lock = 0, L_Check = 0, Dsd = 0, sfLED = 1, Sb_Set = 0, K_last = 0, T_last = 0, B_Count = 0,
    Op_Sp1 = 1, Op_Sp2 = 1, Op_Sp6 = 1, sP = 0, ExitS_CK = 0, LED_Ban = 0, Key_Speed = 2, Key_Count = 0,
    Ke_Dc = 0, Kp_inde = 0, KC_Ban = 0, Kfls = 0, Op_Sp8 = 1, Op_Sp9 = 1, sP_Op = 0, KC_WifiC = 0,
    Op_Sp15, Settings_Opened = 0, Sp15_P, Op_Sp18 = 1, State_LED3 = 0, delay_display_wifi_password = 0,
    input_count_L5 = 0, Op_Sp19 = 1, Op_Sp20 = 1, disconnectCount = 0;
char wifi_Password[PASSWORD_LENGTH + 1];
bool buttonGPressed = false, tag_clear = false, error_display = false, show_L5 = true, LockBottom_L5 = false, fast_L5 = false, IBtran = false, FastIned_L5 = false;

// 定义全局变量存储 IP 地址和字符串表示
IPAddress ipAddress;
String ipAddressString;
String IPpart1, IPpart2;
String BApart1, BApart2;

//L5输入字符串定义
String inputBuffer = ""; // 输入缓冲区
String lastinputBuffer = ""; // 上一个输入


BleKeyboard bleKeyboard("Remote control", "ID_Devices", 100);        //实例化BleKeyboard对象，“Remote control”为键盘名称；"ID_Devices"为制造商
void(* resetFunc) (void) = 0;     //重启函数声明

//****************************************************************Wifi Web Page定义****************************************************************
AsyncWebServer server(80);        // 创建WebServer对象, 端口号80
// 使用端口号80可以直接输入IP访问，使用其它端口需要输入IP:端口号访问
// 一个储存网页的数组
const char index_html[] PROGMEM = R"rawliteral(
<!-- HTML页面内容，包括按钮和JavaScript代码 -->
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="utf-8">
  <title>Remote control Wifi控制</title>
</head>
<body>
  <div>
    <h1>Remote control Wifi控制</h1>
    <h5>您已开启Remote control的Wifi控制功能，可通过该页面进行遥控。</h5>
    <hr />
  </div>

  <div>
    <h4>可使用的功能按键</h4>
  </div> 

  <!-- 添加按钮时，请按照下面的格式添加注释和按钮代码 -->
  <!--
  <div>
    <button type="button" style="height:40px;width:100px;" onclick="sendCommand(1)">Button 1</button>
    <button type="button" style="height:40px;width:100px;" onclick="sendCommand(2)">Button 2</button>
    ...
  </div>
  -->

    <div>
        <table>
            <tr>
                <td></td>
                <td><button type="button" style="height:40px;width:100px;" onclick="sendCommand(1)"> Up </button></td>
                <td></td>
            </tr>
            <tr>
                <td><button type="button" style="height:40px;width:100px;" onclick="sendCommand(2)"> Left </button></td>
                <td><button type="button" style="height:40px;width:100px;" onclick="sendCommand(3)"> Down </button></td>
                <td><button type="button" style="height:40px;width:100px;" onclick="sendCommand(4)"> Right </button></td>
            </tr>
        </table>
    </div>
    <br>

    <div>
        <table>
            <tr>
                <td><button type="button" style="height:40px;width:100px;" onclick="sendCommand(5)"> V- </button></td>
                <td><button type="button" style="height:40px;width:100px;" onclick="sendCommand(6)"> V+ </button></td>
                <td><button type="button" style="height:40px;width:100px;" onclick="sendCommand(7)"> Pause/Play </button></td>
            </tr>
        </table>
    </div>
    <br>

    <div>
        <table>
            <tr>
                <td><button type="button" style="height:40px;width:100px;" onclick="sendCommand(8)"> Win </button></td>
                <td><button type="button" style="height:40px;width:100px;" onclick="sendCommand(9)"> Alt+Tab </button></td>
                <td><button type="button" style="height:40px;width:100px;" onclick="sendCommand(10)"> Alt+F4 </button></td>
                <td><button type="button" style="height:40px;width:100px;" onclick="sendCommand(11)"> Enter </button></td>
            </tr>
        </table>
    </div>
    <br>

    <div>
        <h6>*以上功能按键仅在同时满足蓝牙已连接和远程控制未被锁定时可用。</h6>
    </div> 
</body>

<script>
  // 按下按钮会运行这个JS函数
  function sendCommand(buttonNumber) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/sendCommand?button=" + buttonNumber, true);
    xhr.send();
  }
</script>)rawliteral";
//****************************************************************Wifi Web Page定义结束****************************************************************

//****************************************************************L5输入定义****************************************************************
// 定义摩尔斯代码
const char* morseCode[] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", 
                           "-.--", "--..", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----", ".-.-.-", "--..--", "---...", "-.-.-.", "..--..", "-...-", ".----.", 
                           "-..-.", "-.-.--", "-....-", "..--.-", ".-..-.", "-.--.", "-.--.-", "...-..-", ".--.--", ".--.-.", "--.-..", ".--..", "-..--", "...--.", ".-..-", "..-.--", "-.---", "---.--",
                           "-.---."};

// 定义字符集合
const char* letters = "abcdefghijklmnopqrstuvwxyz1234567890.,:;?='/!-_\"()$&@+* ABCDEF"; // 字母、数字和一些常见符号，A:win+space，B:shift+del，C:backspace，D:Esc，E:Tab，F:F5
//****************************************************************L5输入定义结束****************************************************************

//**********************************************LCD函数**********************************************
void LCD_Write(int D_Da, int rs_Mode) {         //发送数据至595
  digitalWrite(rs, rs_Mode);
  delay(2);

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, D_Da);
  digitalWrite(latchPin, HIGH);

  delayMicroseconds(100);
  digitalWrite(latchPin, LOW);
}

void LCD_Clear() {
  LCD_Write(0x01, 0);  // clear display, set cursor position to zero
  delay(2);  // this command takes a long time!
}

void LCD_Begin() {
  LCD_Write(0x01, 0);
  LCD_Write(0x02, 0);
  LCD_Write(0x06, 0);
  LCD_Write(0x0c, 0);
  LCD_Write(0x14, 0);
  LCD_Write(0x38, 0);
}

void LCD_SetCursor(int Line,int Column) {
  if(Line == 1){
    LCD_Write((0x80|(Column-1)), 0);
  }else{
    LCD_Write((0x80|(Column-1)+0x40), 0);
  }
}

void LCD_Print(const char* str) {  
  int len = strlen(str);    
  for (int i = 0; i < len; i++){    
    LCD_Write(str[i], 1);  
  }   
}

void LCD_Makechar(int Location, uint8_t Charmap[]) {       //CGRAM输入自定义字符数据
  Location &= 0x7;        // we only have 8 locations 0-7
  LCD_Write((0x40 | (Location << 3)), 0);
  for (int i=0; i<8; i++) {
    LCD_Write(Charmap[i], 1);
  }
}

//******************************LCD自定义字符区******************************
void Custom_characters(int Type) {       //自定义字符
  byte CG_dat1[] = {0x1a,0x17,0x12,0x1b,0x0,0x1a,0x12,0x13};  //Ctrl
  byte CG_dat2[] = {0x4,0xa,0xe,0xa,0x12,0x17,0x12,0x1b};  //Alt
  byte CG_dat3[] = {0x2,0x7,0xa,0x1e,0x12,0x1e,0x6,0x1e};  //Shift
  byte CG_dat4[] = {0x11,0x15,0xa,0x0,0xa,0x5,0x1d,0xd};  //Win
  byte CG_dat5[] = {0x11,0x19,0x1d,0x1f,0x1d,0x19,0x11,0x0};  //暂停/播放
  byte CG_dat6[] = {0x0,0x1f,0x1,0x1d,0x5,0x15,0x0,0x0};  //wifi远程遥控
  byte CG_dat7[] = {0x0,0xe,0x1f,0x1f,0x1f,0x1f,0xe,0x0};  //选择
  byte CG_dat8[] = {0x0,0xe,0x11,0x11,0x11,0x11,0xe,0x0};  //未选择
  byte CG_dat9[] = {0x0,0x0,0x0,0x0,0x0,0x0,0x18,0x18};  //一个点（页面提示）
  byte CG_dat10[] = {0x0,0x0,0x0,0x0,0x0,0x0,0x1b,0x1b};  //两个点（页面提示）
  byte CG_dat11[] = {0x0,0x0,0x1f,0x0,0x15,0x0,0x0,0x0};  //直流
  byte CG_dat12[] = {0x0,0x11,0xa,0x4,0xa,0x11,0x0,0x0};  //错号
  byte CG_dat13[] = {0x0,0x1,0x2,0x2,0x4,0x14,0x8,0x0};  //对勾
  byte CG_dat14[] = {0x4,0x8,0x1f,0x8,0x4,0x11,0x1f,0x0};  //退出
  byte CG_dat15[] = {0x0,0x0,0x0,0x0,0x0,0x11,0x1f,0x0};  //空格
  byte CG_dat16[] = {0x1b,0x12,0x1b,0x11,0x1b,0xe,0x8,0xe};  //Esc键
  byte CG_dat17[] = {0xa,0x4,0xa,0x15,0x11,0x11,0x1f,0x0};  //删除文件
  byte CG_dat18[] = {0x1,0x5,0x9,0x1f,0x9,0x5,0x1,0x0};  //退格键
  byte CG_dat19[] = {0x0,0x8,0x1f,0x8,0x2,0x1f,0x2,0x0};  //Tab键
  byte CG_dat20[] = {0x2,0x4,0xc,0x1f,0x6,0x4,0x8,0x0};  //快速输入模式标志（闪电）
  byte CG_dat21[] = {0x11,0xa,0x4,0xa,0x11,0x0,0x1f,0x0};  //错误
  byte CG_dat22[] = {0x4,0x4,0x4,0x0,0x4,0x0,0x1f,0x0};  //信息（提示）
  
  switch(Type){       //类型切换
  case 0:
    LCD_Makechar(0, CG_dat1);     //输出0x0-0x5
    LCD_Makechar(1, CG_dat2);
    LCD_Makechar(2, CG_dat3);
    LCD_Makechar(3, CG_dat4);
    LCD_Makechar(4, CG_dat5);
    LCD_Makechar(5, CG_dat6);
    break;
  case 1:
    LCD_Makechar(0, CG_dat9);     //输出0x0-0x7
    LCD_Makechar(1, CG_dat10);
    LCD_Makechar(2, CG_dat11);
    LCD_Makechar(3, CG_dat12);
    LCD_Makechar(4, CG_dat13);
    LCD_Makechar(5, CG_dat14);
    LCD_Makechar(6, CG_dat7);
    LCD_Makechar(7, CG_dat8);
    break;
  case 2:     //mode5专用
    LCD_Makechar(0, CG_dat15);     //输出0x0-0x6
    LCD_Makechar(1, CG_dat3);
    LCD_Makechar(2, CG_dat17);
    LCD_Makechar(3, CG_dat18);
    LCD_Makechar(4, CG_dat16);
    LCD_Makechar(5, CG_dat19);
    LCD_Makechar(6, CG_dat20);
    break;
  case 3:     //系统提示专用
    LCD_Makechar(0, CG_dat21);     //输出0x0-0x1
    LCD_Makechar(1, CG_dat22);
  }
}
//******************************LCD自定义字符区结束******************************
//**********************************************LCD函数结束**********************************************

void FlashLED_3() {         //LED3闪烁
  if(LED_Ban == 0){     //如果LED未被禁用
    digitalWrite(LED3_BUILTIN, LED);
    delay(100);
    digitalWrite(LED3_BUILTIN, !LED);
    if(sfLED == 1){
      digitalWrite(LED3_BUILTIN, LED);
      delay(100);
      digitalWrite(LED3_BUILTIN, !LED);
    }
    if(State_LED3 == 1){
      delay(300);
      digitalWrite(LED3_BUILTIN, LED);
    }
  }
}

void Change_LED3(int control_code) {      //LED3控制
  if(control_code == 1){
    digitalWrite(LED3_BUILTIN, LED);
    State_LED3 = 1;
  }else{
    digitalWrite(LED3_BUILTIN, !LED);
    State_LED3 = 0;
  }
}

void LED1_2_Countrl(int LED_Num, int Countrl_Code){     //LED1或2控制
  if(LED_Ban == 0){     //如果LED未被禁用
    switch(LED_Num){
    case 1:
      digitalWrite(LED1_BUILTIN, Countrl_Code);
      break;
    case 2:
      digitalWrite(LED2_BUILTIN, Countrl_Code);
    }
  }
}

void Keys_Press_Count(int Key_inde, int KP_m){      //键按下计数
  if(KC_Ban == 0){     //如果键按下计数未被禁用
    if(KP_m == 0){      //按下键模式
      if(Kp_inde == 0){
        Kp_inde = Key_inde;
      }
      if(Kp_inde != Key_inde && Kp_inde != 0){
        Key_Count = 0;
        Kp_inde = Key_inde;
      }
      Key_Count = Key_Count + 1;
      Ke_Dc = 0;
    }else if(KP_m == 1){
      if(Ke_Dc < 500){
        Ke_Dc = Ke_Dc + 1;
        Kfls = 0;
        delay(2);
      }else if(Ke_Dc == 500){
        if(Kfls == 0){
          Key_Count = 0;
          Kp_inde = 0;
          Sb_Set = 0;

          Kfls = 1;
        }
      }
    }
  }
  if(KP_m == 0){      //按下键模式
    Total_Key++;     //总按键次数计数
  }
}

void Display_last(int iK_last, int iT_last){
  LCD_SetCursor(2, 1);
  if(iK_last == 0){     //“上一个”显示
    LCD_Print("(Last)");
  }else{
    switch(iK_last){         //对于键
    case 1:
      LCD_Print("A>");
      break;
    case 2:
      LCD_Print("B>");
      break;
    case 3:
      LCD_Print("C>");
      break;
    case 4:
      LCD_Print("D>");
      break;
    case 5:
      LCD_Print("E>");
      break;
    case 6:
      LCD_Print("F>");
      break;
    case 7:
      LCD_Print("G>");
      break;
    case 8:
      LCD_Write(0x5, 1);      //wifi远程遥控
      LCD_Print(">");
    }

    switch(iT_last){         //对于遥控项目
    case 1:
      LCD_Print("Up");
      break;
    case 2:
      LCD_Print("Down");
      break;
    case 3:
      LCD_Print("Enter");
      break;
    case 4:
      LCD_Write(0x1, 1);      //ALT+F4
      LCD_Print("+F4");
      break;
    case 5:
      LCD_Write(0x4, 1);      //暂停/播放
      break;
    case 6:
      LCD_Print("V-");
      break;
    case 7:
      LCD_Print("V+");
      break;
    case 8:
      LCD_Print("Left");
      break;
    case 9:
      LCD_Print("Right");
      break;
    case 10:
      LCD_Write(0x3, 1);      //Win
      break;
    case 11:
      LCD_Write(0x1, 1);      //ALT+Tab
      LCD_Print("+Tab");
    }
  }
}

//**********************************************Wifi遥控函数开始**********************************************
void Config_Callback(AsyncWebServerRequest *request){        // 下发处理回调函数
  if (request->hasParam("value")) // 如果有值下发
  {
    String HTTP_Payload = request->getParam("value")->value();    // 获取下发的数据
    Serial.printf("[%lu]%s\r\n", millis(), HTTP_Payload.c_str()); // 打印调试信息
  }
  request->send(200, "text/plain", "OK"); // 发送接收成功标志符
}

void sendCommand_Callback(AsyncWebServerRequest *request){       // 处理按钮按下的回调函数
  if (request->hasParam("button")) {
    int buttonNumber = request->getParam("button")->value().toInt();
    if(Lock == 0 && bleKeyboard.isConnected()){     //判断是否应控制
      switch(buttonNumber){
      case 1:
          bleKeyboard.press(KEY_UP_ARROW);
          bleKeyboard.releaseAll();
          Serial.println("Wifi_Up");
          LED1_2_Countrl(2, 1);    //动作指示灯（2）亮/灭
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(1, 8);
          Keys_Press_Count(8, 0);
          Keys_Press_Count(8, 1);
          break;
      case 2:
          bleKeyboard.press(KEY_LEFT_ARROW);
          bleKeyboard.releaseAll();
          Serial.println("Wifi_Left");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(8, 8);
          Keys_Press_Count(9, 0);
          Keys_Press_Count(9, 1);
          break;
      case 3:
          bleKeyboard.press(KEY_DOWN_ARROW);
          bleKeyboard.releaseAll();
          Serial.println("Wifi_Down");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(2, 8);
          Keys_Press_Count(10, 0);
          Keys_Press_Count(10, 1);
          break;
      case 4:
          bleKeyboard.press(KEY_RIGHT_ARROW);
          bleKeyboard.releaseAll();
          Serial.println("Wifi_Right");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(9, 8);
          Keys_Press_Count(11, 0);
          Keys_Press_Count(11, 1);
          break;
      case 5:
          bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
          Serial.println("Wifi_V-");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(6, 8);
          Keys_Press_Count(12, 0);
          Keys_Press_Count(12, 1);
          break;
      case 6:
          bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
          Serial.println("Wifi_V+");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(7, 8);
          Keys_Press_Count(13, 0);
          Keys_Press_Count(13, 1);
          break;
      case 7:
          bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
          Serial.println("Wifi_Pause/Play");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(5, 8);
          Keys_Press_Count(14, 0);
          Keys_Press_Count(14, 1);
          break;
      case 8:
          bleKeyboard.press(KEY_LEFT_GUI);
          bleKeyboard.releaseAll();
          Serial.println("Wifi_Win");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(10, 8);
          Keys_Press_Count(15, 0);
          Keys_Press_Count(15, 1);
          break;
      case 9:
          bleKeyboard.press(KEY_LEFT_ALT);
          bleKeyboard.press(KEY_TAB);
          bleKeyboard.releaseAll();
          Serial.println("Wifi_Alt+Tab");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(11, 8);
          Keys_Press_Count(16, 0);
          Keys_Press_Count(16, 1);
          break;
      case 10:
          bleKeyboard.press(KEY_LEFT_ALT);
          bleKeyboard.press(KEY_F4);
          bleKeyboard.releaseAll();
          Serial.println("Wifi_Alt+F4");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(4, 8);
          Keys_Press_Count(17, 0);
          Keys_Press_Count(17, 1);
          break;
      case 11:
          bleKeyboard.press(KEY_RETURN);
          bleKeyboard.releaseAll();
          Serial.println("Wifi_Enter");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(3, 8);
          Keys_Press_Count(18, 0);
          Keys_Press_Count(18, 1);
      }
    }else{
      FlashLED_3();
      if(Settings_Opened == 0 && bleKeyboard.isConnected()){     //如果不在设置页面且BLE已连接（主页）
        for (int i = 0; i < 2; i++) {
          LCD_SetCursor(1, 5);
          LCD_Print("    ");
          delay(300);
          LCD_SetCursor(1, 5);
          LCD_Print("Lock");
          delay(300); 
        }
      }
    }
    request->send(200, "text/plain", "OK"); // 发送接收成功标志符
    return;
  }
  request->send(400, "text/plain", "Bad Request"); // 发送错误响应
}

void generateWifiPassword(){      // 生成随机 WiFi 密码的函数
  const char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";  // 字符集合，包含小写字母和数字
  int charsetSize = sizeof(charset) - 1;  // 获取字符集的长度
  String password;  // 生成随机密码
  for (int i = 0; i < PASSWORD_LENGTH; ++i) {    // 从字符集中随机选择一个字符
    char randomChar = charset[random(0, charsetSize)];
    password += randomChar;    // 将字符添加到密码中
  }
  // 将密码复制到全局数组中
  strncpy(wifi_Password, password.c_str(), PASSWORD_LENGTH);
  wifi_Password[PASSWORD_LENGTH] = '\0'; // 添加 null 结尾

  Serial.println("Generated WiFi Password: " + password);
}

void Wifi_Control(int WifiC){     //Wifi控制
  if(WifiC == 1 && KC_WifiC != 1) {
    Serial.println("Enable Wifi");
    KC_WifiC = 1;
    WiFi.mode(WIFI_STA);   // 将WiFi设置为Station模式
    // 启动WiFi热点
    generateWifiPassword();   //生成新的wifi密码
    WiFi.softAP(ssid, wifi_Password);
    // 添加HTTP主页，当访问的时候会把网页推送给访问者
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });
    
    server.on("/sendCommand", HTTP_GET, sendCommand_Callback); // 绑定按钮按下的处理函数
    server.on("/Up", HTTP_GET, Config_Callback);   // 绑定配置下发的处理函数
    server.begin();  // 初始化HTTP服务器
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }else if(WifiC == 0 && KC_WifiC != 0){  // 在不需要时关闭 Wifi
    Serial.println("Disable Wifi");
    KC_WifiC = 0;
    WiFi.softAPdisconnect(true);  // true parameter indicates to also shut down the WiFi module
  }
}
//**********************************************Wifi遥控函数结束**********************************************

void S_last(int iST_last, int iSK_last) {     //应被调用
  Sb_Set = 0;

  T_last = iST_last;
  K_last = iSK_last;
}

void A_loading() {        //“Loading”动画
  LCD_SetCursor(2, 1);
  LCD_Print("Loading");
  delay(1000);
  LCD_Print("_");
  delay(1000);
  LCD_SetCursor(2, 8);
  LCD_Print(" ");
  delay(500);
  esp_task_wdt_reset();
} 

void Device_Name() {      //设备名称
  LCD_Clear();
  LCD_SetCursor(1, 1);
  LCD_Print("Remote");
  LCD_SetCursor(2, 1);
  LCD_Print("control");
}

void PageTip_Set() {        //“设置”页面中的提示
  LCD_SetCursor(2, 1);
  if(sP == 5 || sP == 13){       //转换符号为判断
    LCD_Write(0x3, 1);
    LCD_Print("F    G");
    LCD_Write(0x4, 1);
  }else if(sP == 0){       //转换左符号为退出
    LCD_Write(0x5, 1);
    LCD_Print("F    G");
    LCD_Write(0x7e, 1);
  }else{
    LCD_Write(0x7f, 1);
    LCD_Print("F    G");
    LCD_Write(0x7e, 1);
  }
  
  LCD_SetCursor(2, 4);
  switch(sP){
  case 0:       //页面标识号
    LCD_Write(0x0, 1);
    break;
  case 1:
    LCD_Write(0x1, 1);
    break;
  case 9:
    LCD_Write(0x1, 1);
    LCD_Write(0x0, 1);
  }
}

void System_Message(int Message_type, const char* Message){     //系统信息显示
  LCD_Clear();
  Custom_characters(3);      //自定义字符集3
  LCD_SetCursor(1, 1);
  switch(Message_type){   //第一行文案切换
  case 0:
    LCD_Write(0x0, 1);
    LCD_Print("Error");
    LCD_SetCursor(2, 7);
    LCD_Print("G");
    LCD_Write(0x7e, 1);
    break;
  case 1:
    LCD_Write(0x1, 1);
    LCD_Print("Message");
  }
  LCD_SetCursor(2, 1);
  LCD_Print(">");
  LCD_Print(Message);
}


void handleBLEDisconnect(){      // BLE频繁掉线探针
    unsigned long currentTime = millis();
    
    if(currentTime - lastDisconnectTime <= 30000){    // 检查是否在30秒内
      disconnectCount++;
    }else{
      disconnectCount = 1;        // 超过30秒，重置计数器
    }
    lastDisconnectTime = currentTime;    // 更新最后一次掉线时间
    
    if(disconnectCount == 3){    // 检查是否在30秒内第三次掉线
      Serial.println("Error_0x65(BLE disconnected 3 times in 30 seconds)");
      Change_LED3(1);
      System_Message(0, "0x65");
      while(1) {
        delay(2);
        esp_task_wdt_reset();
        if(digitalRead(0) == 0){       //按下G
          disconnectCount = 0;      //清除掉线计数

          LCD_Clear();
          Custom_characters(0);      //自定义字符集0
          Change_LED3(0);
          break;
        }
      }
    }
}

void Page_settings() {        //“设置”界面****************************************************************
  //*********启动屏*********
  Settings_Opened = 1;
  LCD_Clear();
  FlashLED_3();
  Serial.println("");   //换行
  Serial.println("Settings page");
  LCD_Print("Settings");
  Key_Count = 0;      //键盘计数清除
  Kp_inde = 0;
  A_loading();
  Custom_characters(1);      //自定义字符集1
  //*********启动屏结束*********
  Sb_Set = 0;
  int sO1_Max = 9, sO2_Max = 5, sO9_Max = 3;     //最多选项
  while(1) {
    if(Sb_Set == 0){
      if(sP_Op == 0){     //当打开动态刷新的页面时不自动清屏
        LCD_Clear();
      }

      switch(sP){     //********************************************“设置”页面UI********************************************
      case 0:
        LCD_Write(Op_Sp1 + 0x30, 1);      //设置选项序号1
        LCD_Print(".");
        switch(Op_Sp1){
        case 1:
          LCD_Print("Wifi_C");
          break;  
        case 2:
          LCD_Print("Key_Sp");
          break;
        case 3:
          LCD_Print("K_Coun");
          break;
        case 4:
          LCD_Print("LED_Op");
          break;
        case 5:
          LCD_Print("Show_5");
          break;  
        case 6:
          LCD_Print("InMo_5");
          break;  
        case 7:
          LCD_Print("Reset");
          break;
        case 8:
          LCD_Print("Re_str");
          break;
        case 9:
          LCD_Print("About");
        }
        PageTip_Set();
        break;
      case 1:
        LCD_Write(Op_Sp2 + 0x30, 1);      //设置选项序号2
        LCD_Print(".");
        switch(Op_Sp2){
        case 1:
          LCD_Print("Name");
          break;
        case 2:
          LCD_Print("Ver");
          break;
        case 3:
          LCD_Print("BLE_Ad");
          break;
        case 4:
          LCD_Print("I_Info");
          break;
        case 5:
          LCD_Print("Device");
        }
        PageTip_Set();
        break;
      case 2:         //设备名称
        Device_Name();
        break;
      case 3:         //固件版本
        LCD_Print("Version:");
        LCD_SetCursor(2, 1);
        LCD_Print("V");
        LCD_Print(version);
        break;
      case 4:         //BLE地址
        if(strlen(BLE_Address) > 8){ // 判断 BLE 地址字符串长度是否超过 8 个字符
          BApart1 = String(BLE_Address).substring(0, 8); // 获取前 8 个字符
          BApart2 = String(BLE_Address).substring(8);    // 获取剩余字符
        }else{
          BApart1 = BLE_Address; // 如果字符串长度不超过 8 个字符，直接使用整个字符串
        }

        // 显示结果
        LCD_Print(BApart1.c_str());
        LCD_SetCursor(2, 1);
        LCD_Print(BApart2.c_str());
        break;
      case 5:     //重启提示
        LCD_Print("Sure?");
        PageTip_Set();
        break;
      case 6:       //LED选项
        if(LED_Ban == 0){      //LED未禁用
          LCD_Write(0x6, 1);
          LCD_SetCursor(2, 1);
          LCD_Write(0x7, 1);
          Change_LED3(1);
          if(bleKeyboard.isConnected()){    //如果已连接
            LED1_2_Countrl(1, 1);    //BLE通信指示灯（1）亮
          }
        }else{
          LCD_Write(0x7, 1);
          LCD_SetCursor(2, 1);
          LCD_Write(0x6, 1);
          Change_LED3(0);
        }
        LCD_SetCursor(1, 2);
        LCD_Print("On");
        LCD_SetCursor(2, 2);
        LCD_Print("Off");

        switch(Op_Sp6){
        case 1:
          LCD_SetCursor(1, 7);
          break;
        case 2:
          LCD_SetCursor(2, 7);
        }
        LCD_Print("<<");
        break;
      case 7:     //一般按键速度调整
        char cKey_Speed[1];    //Key_Speed字符串
        sprintf(cKey_Speed, "%d", Key_Speed);

        LCD_Print("K_Sp:");
        LCD_SetCursor(1, 8);
        LCD_Print(cKey_Speed);
        LCD_SetCursor(2, 1);
        LCD_Print("-");
        LCD_SetCursor(2, 8);
        LCD_Print("+");
        LCD_SetCursor(2, 2);
        for(int i = 0; i < Key_Speed * 2; i++){     //指示条
          LCD_Write(0xff, 1);
        }  
        break;
      case 8:       //显示按键计数
        if(KC_Ban == 0){      //按键计数未禁用
          LCD_Write(0x6, 1);
          LCD_SetCursor(2, 1);
          LCD_Write(0x7, 1);
          Change_LED3(1);
        }else{
          LCD_Write(0x7, 1);
          LCD_SetCursor(2, 1);
          LCD_Write(0x6, 1);
          Change_LED3(0);
        }
        LCD_SetCursor(1, 2);
        LCD_Print("On");
        LCD_SetCursor(2, 2);
        LCD_Print("Off");

        switch(Op_Sp8){
        case 1:
          LCD_SetCursor(1, 7);
          break;
        case 2:
          LCD_SetCursor(2, 7);
        }
        LCD_Print("<<");
        break;
      case 9:       //设备页
        LCD_Write(Op_Sp9 + 0x30, 1);      //设置选项序号9
        LCD_Print(".");
        switch(Op_Sp9){
        case 1:
          LCD_Print("TKey_C");
          break;
        case 2:
          LCD_Print("DRun_T");
          break;
        case 3:
          LCD_Print("C_Temp");
        }
        PageTip_Set();
        break;
      case 10:       //操作按键按下操作计数
        Serial.print("Total press count:");
        Serial.println(Total_Key);

        LCD_Print("TKey_C:");
        LCD_SetCursor(2, 1);
        if(Total_Key > 9999999){
          LCD_Print("9999999+");
        }else{
          char c_Total_Key[7];    //Total_Key字符串
          sprintf(c_Total_Key, "%d", Total_Key);

          LCD_Print(c_Total_Key);
        }
        break;
      case 11:       //总开机时间
        {
        unsigned long currentTime = millis();        // 获取当前毫秒数
        if(sP_Op == 0){          
          lastUpdateTime = 0;  // 用于跟踪上一次更新的时间
          updateInterval = 1000;  // 更新间隔为1秒
          sP_Op = 1;        // 页面已打开
        }

        if(currentTime - lastUpdateTime >= updateInterval) {
          lastUpdateTime = currentTime;  // 更新上一次更新的时间
          LCD_Clear();

          //***主程序***
          unsigned long uptimeSeconds = (currentTime - startTime) / 1000;        // 计算开机时间（秒）
          LCD_Print("DRun_T:");
          LCD_SetCursor(2, 1);
          Serial.print("Running time:");
          Serial.print(uptimeSeconds);
          Serial.println("s");
          if(uptimeSeconds > 999999) {        // 判断开机时间是否超过6位数字
            LCD_Print("999999+s");          // 如果超过，打印6位数字和一个加号
          }else{
            char c_uptimeSeconds[6];    //uptimeSeconds字符串
            sprintf(c_uptimeSeconds, "%d", uptimeSeconds);

            LCD_Print(c_uptimeSeconds);          // 如果未超过，直接打印开机时间
            LCD_Print("s");
          }
          //*********
        }
        break;
        }
      case 12:       //输入信息
        LCD_Print("I_Info:");
        LCD_SetCursor(2, 1);
        LCD_Print("5V");
        LCD_Write(0x2, 1);
        LCD_Print("1A");
        break;
      case 13:      //重置设置项
        LCD_Print("Sure?");
        PageTip_Set();
        break;
      case 14:        //芯片温度
        {
        unsigned long currentTime = millis();        // 获取当前毫秒数
        if(sP_Op == 0){          
          lastUpdateTime = 0;  // 用于跟踪上一次更新的时间
          updateInterval = 1000;  // 更新间隔为1秒
          sP_Op = 1;        // 页面已打开
        }

        if(currentTime - lastUpdateTime >= updateInterval) {
          lastUpdateTime = currentTime;  // 更新上一次更新的时间
          LCD_Clear();

          //***主程序***
          float Ftsens_out;
          int Itsens_out;
          temp_sensor_read_celsius(&Ftsens_out);
          Itsens_out = static_cast<int>(Ftsens_out);
          char c_Itsens_out[3];    //Itsens_out字符串
          sprintf(c_Itsens_out, "%d", Itsens_out);

          LCD_Print("C_Temp:");
          LCD_SetCursor(2, 1);
          Serial.print("Chip temputer:");
          Serial.print(c_Itsens_out);
          Serial.println("C");
          LCD_Print(c_Itsens_out);
          LCD_Write(0xdf, 1);
          LCD_Print("C");
          //*********
        }
        break;
        }
      case 15:        //Wifi控制选项
        if(Sp15_P == 1){
          LCD_SetCursor(1, 1);
          LCD_Print("?Info");
          if(KC_WifiC == 1){      //Wifi控制启用情况
            LCD_SetCursor(2, 1);
            LCD_Write(0x6, 1);
            Change_LED3(1);
          }else{
            LCD_SetCursor(2, 1);
            LCD_Write(0x7, 1);
            Change_LED3(0);
          }
          LCD_SetCursor(2, 2);
          LCD_Print("On");
        }else if(Sp15_P == 2){
          if(KC_WifiC == 1){      //Wifi控制启用情况
            LCD_SetCursor(1, 1);
            LCD_Write(0x6, 1);
            LCD_SetCursor(2, 1);
            LCD_Write(0x7, 1);
            Change_LED3(1);
          }else{
            LCD_SetCursor(1, 1);
            LCD_Write(0x7, 1);
            LCD_SetCursor(2, 1);
            LCD_Write(0x6, 1);
            Change_LED3(0);
          }
          LCD_SetCursor(1, 2);
          LCD_Print("On");
          LCD_SetCursor(2, 2);
          LCD_Print("Off");
        }

        switch(Op_Sp15){
        case 0:
          LCD_SetCursor(1, 7);
          break;
        case 1:
          if(Sp15_P == 1){
            LCD_SetCursor(2, 7);
          }else if(Sp15_P == 2){
            LCD_SetCursor(1, 7);
          }
          break;
        case 2:
          LCD_SetCursor(2, 7);
        }
        LCD_Print("<<");
        break;
      case 16:      //IP地址显示
        ipAddress = WiFi.softAPIP();      // 获取 SoftAP 的 IP 地址
        ipAddressString = ipAddress.toString();     // 将 IPAddress 转换为字符串
        // 判断 IP 地址字符串长度是否超过 8 个字符
        if(ipAddressString.length() > 8){
          IPpart1 = ipAddressString.substring(0, 8);          // 获取前 8 个字符
          IPpart2 = ipAddressString.substring(8);          // 获取剩余字符
        }else{
          IPpart1 = ipAddressString;          // 如果字符串长度不超过 8 个字符，直接使用整个字符串
        }
        // 显示结果
        LCD_Print(IPpart1.c_str());
        LCD_SetCursor(2, 1);
        LCD_Print(IPpart2.c_str());
        break;
      case 17:      //Wifi-Ap密码
        LCD_Print("P_word:");
        LCD_SetCursor(2, 1);
        LCD_Print(wifi_Password);
        break;
      case 18:      //wifi控制信息选择
        LCD_Print("Pas_W");
        LCD_SetCursor(2, 1);
        LCD_Print("IP");

        switch(Op_Sp18){
        case 1:
          LCD_SetCursor(1, 7);
          break;
        case 2:
          LCD_SetCursor(2, 7);
        }
        LCD_Print("<<");
        break;
      case 19:      //显示L5设置
        if(show_L5){      //显示L5未禁用
          LCD_Write(0x6, 1);
          LCD_SetCursor(2, 1);
          LCD_Write(0x7, 1);
          Change_LED3(1);
        }else{
          LCD_Write(0x7, 1);
          LCD_SetCursor(2, 1);
          LCD_Write(0x6, 1);
          Change_LED3(0);
        }
        LCD_SetCursor(1, 2);
        LCD_Print("On");
        LCD_SetCursor(2, 2);
        LCD_Print("Off");

        switch(Op_Sp19){
        case 1:
          LCD_SetCursor(1, 7);
          break;
        case 2:
          LCD_SetCursor(2, 7);
        }
        LCD_Print("<<");
        break;
      case 20:      //L5输入模式
        if(!(fast_L5)){      //L5普通输入模式
          LCD_Write(0x6, 1);
          LCD_SetCursor(2, 1);
          LCD_Write(0x7, 1);
        }else{
          LCD_Write(0x7, 1);
          LCD_SetCursor(2, 1);
          LCD_Write(0x6, 1);
        }
        LCD_SetCursor(1, 2);
        LCD_Print("Norm");
        LCD_SetCursor(2, 2);
        LCD_Print("Fast");

        switch(Op_Sp20){
        case 1:
          LCD_SetCursor(1, 7);
          break;
        case 2:
          LCD_SetCursor(2, 7);
        }
        LCD_Print("<<");
      }

      if(sP_Op == 0){     //当打开动态刷新的页面时不自动还原
        Sb_Set = 1;
      }
    }

    if(delay_display_wifi_password == 1){
      delay_display_wifi_password = 0;
      delay(700);
      sP = 17;    //主动显示密码
      Sb_Set = 0;
      }

    if(digitalRead(18) == 0){       //按下Mode
      switch(sP){
      case 0:
        if(Op_Sp1 == 1){
          Op_Sp1 = sO1_Max;
        }else{
          Op_Sp1 = Op_Sp1 - 1;
        }
        break;
      case 1:
        if(Op_Sp2 == 1){
          Op_Sp2 = sO2_Max;
        }else{
          Op_Sp2 = Op_Sp2 - 1;
        }
        break;
      case 6:
        if(Op_Sp6 == 1){
          Op_Sp6 = 2;
        }else{
          Op_Sp6 = Op_Sp6 - 1;
        }
        break;
      case 7:
        if(Key_Speed != 1){
          Key_Speed = Key_Speed - 1;
        }
        break;
      case 8:
        if(Op_Sp8 == 1){
          Op_Sp8 = 2;
        }else{
          Op_Sp8 = Op_Sp8 - 1;
        }
        break;
      case 9:
        if(Op_Sp9 == 1){
          Op_Sp9 = sO9_Max;
        }else{
          Op_Sp9 = Op_Sp9 - 1;
        }
        break;
      case 15:
        if(Op_Sp15 == 1 - KC_WifiC){
          Op_Sp15 = 2;
        }else{
          Op_Sp15 = Op_Sp15 - 1;
        }
        if(Op_Sp15 == 0){
          Sp15_P = 1;
        }else if(Op_Sp15 == 2){
          Sp15_P = 2;
        }
        break;
      case 18:
        if(Op_Sp18 == 1){
          Op_Sp18 = 2;
        }else{
          Op_Sp18 = Op_Sp18 - 1;
        }
        break;
      case 19:
        if(Op_Sp19 == 1){
          Op_Sp19 = 2;
        }else{
          Op_Sp19 = Op_Sp19 - 1;
        }
        break;
      case 20:
        if(Op_Sp20 == 1){
          Op_Sp20 = 2;
        }else{
          Op_Sp20 = Op_Sp20 - 1;
        }
      }
      Sb_Set = 0;

      Serial.println("Seeings Key Mode");
      delay(KeyMode);
      KeyMode = 200;
    }else{
      KeyMode = 485;
    }

    if(digitalRead(19) == 0){       //按下E
      switch(sP){
      case 0:
        if(Op_Sp1 == sO1_Max){
          Op_Sp1 = 1;
        }else{
          Op_Sp1 = Op_Sp1 + 1;
        }
        break;
      case 1:
        if(Op_Sp2 == sO2_Max){
          Op_Sp2 = 1;
        }else{
          Op_Sp2 = Op_Sp2 + 1;
        }
        break;
      case 6:
        if(Op_Sp6 == 2){
          Op_Sp6 = 1;
        }else{
          Op_Sp6 = Op_Sp6 + 1;
        }
        break;
      case 7:
        if(Key_Speed != 3){
          Key_Speed = Key_Speed + 1;
        }
        break;
      case 8:
        if(Op_Sp8 == 2){
          Op_Sp8 = 1;
        }else{
          Op_Sp8 = Op_Sp8 + 1;
        }
        break;
      case 9:
        if(Op_Sp9 == sO9_Max){
          Op_Sp9 = 1;
        }else{
          Op_Sp9 = Op_Sp9 + 1;
        }
        break;
      case 15:
        if(Op_Sp15 == 2){
          Op_Sp15 = 1 - KC_WifiC;
        }else{
          Op_Sp15 = Op_Sp15 + 1;
        }
        if(Op_Sp15 == 0){
          Sp15_P = 1;
        }else if(Op_Sp15 == 2){
          Sp15_P = 2;
        }
        break;
      case 18:
        if(Op_Sp18 == 2){
          Op_Sp18 = 1;
        }else{
          Op_Sp18 = Op_Sp18 + 1;
        }
      case 19:
        if(Op_Sp19 == 2){
          Op_Sp19 = 1;
        }else{
          Op_Sp19 = Op_Sp19 + 1;
        }
        break;
      case 20:
        if(Op_Sp20 == 2){
          Op_Sp20 = 1;
        }else{
          Op_Sp20 = Op_Sp20 + 1;
        }
      }
      Sb_Set = 0;
      
      Serial.println("Seeings Key E");
      delay(KeyE);
      KeyE = 200;
    }else{
      KeyE = 485;
    }

    if(digitalRead(1) == 0){       //按下F
      if(sP == 0){
        FlashLED_3();
        Serial.println("Settings page exit");
        LCD_Clear();
        LCD_Print("Home");
        LCD_SetCursor(2, 1);
        LCD_Print("Loading");
        switch(Mode){
        case 5:
          Custom_characters(2);      //自定义字符集2
          break;
        default:
          Custom_characters(0);      //自定义字符集0
        }

        Op_Sp1 = 1;
        ExitS_CK = 1;
        Settings_Opened = 0;
        LockBottom_L5 = true;     //锁定L5
        break;
      }else{
        switch(sP){       //“←”的跳转
        case 1:
          Op_Sp2 = 1;
          sP = 0;
          break;
        case 2:
        case 3:
        case 4:
        case 12:
          sP = 1;
          break;
        case 5:
        case 7:
        case 13:
          sP = 0;
          break;
        case 6:
          sP = 0;
          Op_Sp6 = 1;
          Change_LED3(0);
          break;
        case 8:
          sP = 0;
          Op_Sp8 = 1;
          Change_LED3(0);
          break;
        case 9:
          Op_Sp9 = 1;
          sP = 1;
          break;
        case 10:
          sP = 9;
          break;
        case 11:
        case 14:
          sP = 9;
          sP_Op = 0;        // 页面已关闭
          break;
        case 15:
          sP = 0;
          Change_LED3(0);
          break;
        case 16:
        case 17:
          sP = 18;
          break;
        case 18:
          sP = 15;
          Op_Sp18 = 1;
          break;
        case 19:
          sP = 0;
          Op_Sp19 = 1;
          Change_LED3(0);
          break;
        case 20:
          sP = 0;
          Op_Sp20 = 1;
        }
        Sb_Set = 0;
      }
      
      Serial.println("Seeings Key F");
      delay(KeyF);
      KeyF = 200;
    }else{
      KeyF = 485;
    }

    if(digitalRead(0) == 0){       //按下G
      switch(sP){
      case 0:
        switch(Op_Sp1){
        case 1:
          sP = 15;
          if(KC_WifiC == 1){
            Sp15_P = 1;
            Op_Sp15 = 0;
          }else{
            Sp15_P = 2;
            Op_Sp15 = 1;
          }
          break;
        case 2:
          sP = 7;
          break;
        case 3:
          sP = 8;
          break;
        case 4:
          sP = 6;
          break;
        case 5:
          sP = 19;
          break;
        case 6:
          sP = 20;
          break;
        case 7:
          sP = 13;
          break;
        case 8:
          sP = 5;
          break;
        case 9:
          sP = 1;
        }
        break;
      case 1:
        switch(Op_Sp2){
        case 1:
          sP = 2;
          break;
        case 2:
          sP = 3;
          break;
        case 3:
          sP = 4;
          break;
        case 4:
          sP = 12;
          break;
        case 5:
          sP = 9;
        }
        break;
      case 5:
        FlashLED_3();
        Serial.println("Restart");
        System_Message(1, "Wait");

        delay(500);
        resetFunc();
        break;
      case 6:
        switch(Op_Sp6){
        case 1:
          LED_Ban = 0;
          Serial.println("LED was unbaned");
          break;
        case 2:    
          LED1_2_Countrl(1,0);    //关闭LED
          LED1_2_Countrl(2,0);
          LED_Ban = 1;
          Serial.println("LED was baned");
        break;
        }
      case 8:
        switch(Op_Sp8){
        case 1:
          KC_Ban = 0;
          Serial.println("Key_Count was enable");
          break;
        case 2:    
          Key_Count = 0;      //键盘计数禁用
          Kp_inde = 0;
          KC_Ban = 1;
          Serial.println("Key_Count was disable");
        }
        break;
      case 9:
        switch(Op_Sp9){
        case 1:
          sP = 10;
          break;
        case 2:
          sP = 11;
          break;
        case 3:
          sP = 14;
        }
        break;
      case 13:
        Serial.println("Seetings has been reset");
        LCD_Clear();
        Key_Speed = 2;      //设置按键速度为2
        KC_Ban = 0;       //设置按键计数为开
        LED_Ban = 0;       //设置LED为开
        Wifi_Control(0);          //wifi控制禁用
        show_L5 = true;          //显示L5
        fast_L5 = false;          //关闭快速L5输入
        if(bleKeyboard.isConnected()){    //如果已连接
          LED1_2_Countrl(1, 1);    //BLE通信指示灯（1）亮
        }

        FlashLED_3();
        System_Message(1, "Success");

        delay(2000);
        LCD_Clear();
        Custom_characters(1);      //自定义字符集1
        sP = 0;
        break;
      case 15:
        switch(Op_Sp15){
        case 0:
          sP = 18;
          break;
        case 1:
          if(KC_WifiC != 1){      //如果在未启用的状态下启用
            delay_display_wifi_password = 1;    //主动显示密码
          }
          Wifi_Control(1);
          break;
        case 2:          //wifi控制禁用
          Wifi_Control(0);
        }
        break;
      case 18:
        switch(Op_Sp18){
        case 1:
          sP = 17;
          break;
        case 2:    
          sP = 16;
        }
        break;
      case 19:
        switch(Op_Sp19){
        case 1:
          show_L5 = true;
          Serial.println("Show_L5 was enable");
          break;
        case 2:    
          show_L5 = false;      //显示L5禁用
          if(Mode == 5){
            Mode = 1;
          }
          Serial.println("Show_L5 was disable");
        }
        break;
      case 20:
        switch(Op_Sp20){
        case 1:
          fast_L5 = false;
          Serial.println("Fast_L5 was enable");
          break;
        case 2:    
          fast_L5 = true;      //快速L5模式启动
          Serial.println("Fast_L5 was disable");
        }
      }
      Sb_Set = 0;

      Serial.println("Seeings Key G");
      delay(KeyG);
      KeyG = 200;
    }else{
      KeyG = 485;
    }
    esp_task_wdt_reset();
  }
}         //“设置”界面结束****************************************************************

void sendInputBuffer() {      //摩斯密码解析发送
  // 如果输入缓冲区中的内容不为空
  if (inputBuffer != "") {
    // 在字符集合中查找输入的摩尔斯代码
    bool matched = false;
    for (int i = 0; i < strlen(letters); ++i) {
      if (strcmp(morseCode[i], inputBuffer.c_str()) == 0) {
        matched = true;
        String outputString;
        lastinputBuffer = letters[i];
        if(letters[i] == ' '){      //特殊字符处理  space
          bleKeyboard.press(' ');    
          outputString = "(space)"; 
        }else if (letters[i] == 'A'){      //shift
          bleKeyboard.press(KEY_LEFT_SHIFT);
          outputString = "(shift)"; 
        }else if (letters[i] == 'B'){      //shift+del
          bleKeyboard.press(KEY_LEFT_SHIFT);
          bleKeyboard.press(KEY_DELETE);
          outputString = "(shift+del)"; 
        }else if (letters[i] == 'C'){      //backspace
          bleKeyboard.press(KEY_BACKSPACE);
          outputString = "(backspace)"; 
        }else if (letters[i] == 'D'){      //Esc
          bleKeyboard.press(KEY_ESC);
          outputString = "(Esc)"; 
        }else if (letters[i] == 'E'){      //Tab
          bleKeyboard.press(KEY_TAB);
          outputString = "(Tab)"; 
        }else if (letters[i] == 'F'){      //F5
          bleKeyboard.press(KEY_F5);
          outputString = "(F5)"; 
        }else{
          bleKeyboard.press(letters[i]);      //蓝牙打印字符
          outputString = letters[i];
        }
          bleKeyboard.releaseAll();
          Keys_Press_Count(19, 0);
          Keys_Press_Count(19, 1);
          LED1_2_Countrl(2, 1);     //闪发送灯
          delay(300);
          LED1_2_Countrl(2, 0);
          Serial.println("");   //换行
          Serial.print("BLE input ");
          Serial.println(outputString);
        break;
      }
    }
    if (!matched) {
      error_display = true;
      tag_clear = true;
      Serial.println("Code error"); // 没有匹配的字符，打印错误信息
      FlashLED_3();   //闪灯报错
    }
    if(!fast_L5 || !matched){     //非快速输入模式或有错误
      inputBuffer = ""; // 清空输入缓冲区
      input_count_L5 = 0;
      FastIned_L5 = false;
    }else{
      IBtran = true;
      FastIned_L5 = true;     //本次输入L5发送过
    }
  }else{      //空输入
    error_display = true;
    tag_clear = true;
    Serial.println("Error");
    FlashLED_3();   //闪灯报错
  }
}

void setup() {
  pinMode(latchPin,OUTPUT);
  pinMode(dataPin,OUTPUT);
  pinMode(clockPin,OUTPUT);
  pinMode(rs,OUTPUT);       //LCD

  LCD_Begin();      //LCD初始化
  Custom_characters(0);      //自定义字符集0

  // 初始化看门狗，设置超时时间（以毫秒为单位）
  esp_task_wdt_init(5000, true); // 5000 毫秒，即 5 秒

  Serial.begin(115200);
  Serial.println("Remote control");
  Serial.print("V");
  Serial.println(version);

  temp_sensor_config_t temp_sensor = {      //开始温度监控
  .dac_offset = TSENS_DAC_L2,
  .clk_div = 6,
  };
  temp_sensor_set_config(temp_sensor);
  temp_sensor_start();

  // wifi_设置路由处理函数
  server.on("/getIP", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", WiFi.localIP().toString());
  });

  //*********启动屏*********
  Device_Name();   //Screen 1
  delay(2000);
  esp_task_wdt_reset();
  LCD_Clear();

  LCD_Print("V");   //Screen 2
  LCD_Print(version);
  A_loading();
  //*********启动屏结束*********
  startTime = millis();    //开机计时开始（ms）
  Serial.println("Timer started");

  pinMode(LED1_BUILTIN, OUTPUT);
  pinMode(LED2_BUILTIN, OUTPUT);
  pinMode(LED3_BUILTIN, OUTPUT);    //LED
  
  pinMode(2,INPUT_PULLUP);       
  pinMode(3,INPUT_PULLUP);
  pinMode(10,INPUT_PULLUP);
  pinMode(6,INPUT_PULLUP);      //无线输入
  pinMode(18,INPUT_PULLUP);
  pinMode(19,INPUT_PULLUP);
  pinMode(1,INPUT_PULLUP);
  pinMode(0,INPUT_PULLUP);     //主板按键输入

  Serial.println("Starting BLE work!");
  bleKeyboard.begin();       //开始键盘通信
  Serial.println("BLE Pairing...");
}

void loop() {
  if(sfLED == 1){
    FlashLED_3();
    sfLED = 0;
  }

  if(Connect_Check != 0){       //连接中提示
    if(B_Count < 100){
      if((digitalRead(18) == 0) && (digitalRead(19) == 0)){       //同时按下Mode, E
        delay(800);
        if((digitalRead(18) == 0) && (digitalRead(19) == 0)){       //再次检查
        Page_settings();
        Sb_Set = 0;
        }
      }

      delay(10);
      B_Count = B_Count + 1;
    }else{
      LCD_Clear();
      LCD_Print("Connect");
      LCD_SetCursor(2, 1);
      LCD_Print("Wait");
      if(B_Wait == 0){
        LCD_SetCursor(2, 5);
        LCD_Print(" ");
        B_Wait = 1;
      }else{
        LCD_SetCursor(2, 5);
        LCD_Print("_");
        B_Wait = 0;
      }

      B_Count = 0;
    }
  }else{
    if(Sb_Set == 0){
      LCD_Clear();
      LCD_SetCursor(1, 1);
      LCD_Print("L");
      LCD_Write(Mode + 0x30, 1);

      if(Mode != 5){
        if(Lock == 0 && Mode != 5){      //判断是否锁定（不在Mode5显示）
          LCD_SetCursor(1, 5);
          LCD_Print("    ");        
        }else{
          LCD_SetCursor(1, 5);
          LCD_Print("Lock");
        }
      }

      if(Mode == 5){      //如果是Mode5
        LCD_SetCursor(2, 1);
        if(error_display){
          LCD_Print("(Error)");
          error_display = false;
        }else{
          if(IBtran){
            LCD_Print(inputBuffer.c_str());
            IBtran = false;
          }else{
            LCD_Print("(Code)");
          }
        }
        if(fast_L5){      //快速输入模式标志显示
          LCD_SetCursor(1, 4);
          LCD_Write(0x6, 1);
        }
        LCD_SetCursor(1, 7);
        LCD_Print(">");
        tag_clear = true;
        if(lastinputBuffer == ""){    //如果为""则为黑方块
          LCD_Write(0xff, 1);
        }else{
          if(strcmp(lastinputBuffer.c_str(), " ") == 0){ // 比较字符串和字符'空格'
              LCD_Write(0x0, 1);
          } else if(strcmp(lastinputBuffer.c_str(), "A") == 0){ // 比较字符串和字符'A'
              LCD_Write(0x1, 1);
          } else if(strcmp(lastinputBuffer.c_str(), "B") == 0){ // 比较字符串和字符'B'
              LCD_Write(0x2, 1);
          } else if(strcmp(lastinputBuffer.c_str(), "C") == 0){ // 比较字符串和字符'C'
              LCD_Write(0x3, 1);
          } else if(strcmp(lastinputBuffer.c_str(), "D") == 0){ // 比较字符串和字符'D'
              LCD_Write(0x4, 1);
          } else if(strcmp(lastinputBuffer.c_str(), "E") == 0){ // 比较字符串和字符'E'
              LCD_Write(0x5, 1);
          } else if(strcmp(lastinputBuffer.c_str(), "F") == 0){ // 比较字符串和字符'F'
              LCD_SetCursor(1, 6);
              LCD_Print(">F5");
          } else {
              LCD_Print(lastinputBuffer.c_str());
          }
        }
        LCD_SetCursor(2, 1);


      }
      
      if(Mode != 5){
        if(Key_Count != 0){       //按键计数
          Display_last(K_last, 0);   //“上一个”显示，按键消息为空
          if(Key_Count <= 99){
            char cKey_Count[2];    //Key_Count字符串
            sprintf(cKey_Count, "%d", Key_Count);

            LCD_Print("(x");
            LCD_Print(cKey_Count);
            LCD_Print(")");
          }else{
            LCD_Print("(x99+)");
          }
        }else{
          Display_last(K_last, T_last);   //“上一个”显示
        }
      }
        Sb_Set = 1;
    }
  }

  //---------------------------------------------------已连接--------------------------------------------------------
  if(bleKeyboard.isConnected()){       //如果已配对
    Connect_Check = 0;
    if(Lat_Off == 1){           //开始配对使用
      if(Mode == 5){
        Custom_characters(2);      //自定义字符集2
      }else{
        Custom_characters(0);      //自定义字符集0
      }

      Serial.println("Start Enter");
      FlashLED_3();
    }
    Firest_Offline = 1;
    
    LED1_2_Countrl(1, 1);    //BLE通信指示灯（1）亮

    int Nn_KSp;      //*************************************各挡位按键速度*************************************
    switch(Key_Speed){
    case 1:
      Nn_KSp = 350;
      break;
    case 2:
      Nn_KSp = 200;
      break;
    case 3:
      Nn_KSp = 50;
    }

    //---------------------------------------------------公共按键区--------------------------------------------------------
    if(digitalRead(18) == 0){       //按下Mode
      delay(Fast_Key3);
      if((digitalRead(18) == 0) && (!(digitalRead(19) == 0))){
        Serial.println("");   //换行
        Serial.print("M");
        Serial.print(Mode);
        Serial.println(" Key Mode");

        int ModeNum, ModeMax = 5;     //Mode切换
        if(show_L5){
          ModeNum = ModeMax;
        }else{
          ModeNum = ModeMax - 1;
        }

        if(Mode < ModeNum){
          Mode = Mode + 1;
        }else{
          Mode = 1;
        }

        if(show_L5){    //如果L5启动
          switch(Mode){
          case 5:
            delay(300);

            LCD_Clear();
            LCD_Print("L5");
            LCD_SetCursor(2, 1);
            LCD_Print("Loading");
            Custom_characters(2);      //自定义字符集2
            LockBottom_L5 = true;     //锁定L5
            break;
          case 1:
            LCD_Clear();
            LCD_Print("L5");
            LCD_SetCursor(2, 1);
            LCD_Print("Exiting");
            Custom_characters(0);      //自定义字符集0
          }
        }
        Serial.print("Mode ");
        Serial.println(Mode);
        delay(KeyMode);
        KeyMode = 300;        //特殊延时
        Fast_Key3 = 0;
        inputBuffer = ""; // 清空输入缓冲区
        input_count_L5 = 0;
        FastIned_L5 = false;
        Key_Count = 0;      //键盘计数清除
        Kp_inde = 0;

        Sb_Set = 0;
      }
    }else{
      KeyMode = 485;
      Fast_Key3 = 50;
    }

    if((digitalRead(18) == 0) && (digitalRead(19) == 0)){       //设置，同时按下Mode, E
      delay(800);
      if((digitalRead(18) == 0) && (digitalRead(19) == 0)){       //再次检查
        inputBuffer = ""; // 清空输入缓冲区
        input_count_L5 = 0;
        FastIned_L5 = false;
        Page_settings();
        Sb_Set = 0;
      }
    }

    if(((digitalRead(2) == 1) || (digitalRead(3) == 1) || (digitalRead(10) == 1) || (digitalRead(6) == 1)) && Lock == 1 && Mode != 5){       //上锁数码管闪烁提示
      FlashLED_3();
      for (int i = 0; i < 2; i++) {
        LCD_SetCursor(1, 5);
        LCD_Print("    ");
        delay(300);
        LCD_SetCursor(1, 5);
        LCD_Print("Lock");
        delay(300); 
      }
    }

    if((digitalRead(1) == 1)){      //F未按下
      ExitS_CK = 0;
    }

    if((digitalRead(0) == 0) && (digitalRead(1) == 0) && L_Check == 0 && Mode != 5){       //同时按下F、G且不在Mode5，上锁
      delay(800);
      if((digitalRead(0) == 0) && (digitalRead(1) == 0) && L_Check == 0 && Mode != 5){       //再次检查
        L_Check = 1;
        Dsd = 1;
        FlashLED_3();
        if(Lock == 0){
          Lock = 1;     //禁用遥控
          Serial.println("Locked Remote control");
        }else{
          Lock = 0;     //启用遥控
          Serial.println("Unlocked Remote control");
        }
        Sb_Set = 0;
      }
    }else if((!(digitalRead(0) == 0)) && (!(digitalRead(1) == 0))){       //同时不按下F、G，防止多次触发
      L_Check = 0;
      Dsd = 0;
    }

    //---------------------------------------------------私有按键区--------------------------------------------------------
    if(Mode == 1){          //*************************1*************************
      if(digitalRead(2) == 1 && Lock == 0){       //按下A
        bleKeyboard.press(KEY_UP_ARROW);
        bleKeyboard.releaseAll();
        Serial.println("M1 Key A");
        LED1_2_Countrl(2, 1);    //动作指示灯（2）亮/灭
        delay(KeyA);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(1, 0);
        S_last(1, 1);
        KeyA = Nn_KSp;
      }else{
        KeyA = 485;
        Keys_Press_Count(1, 1);
      }

      if(digitalRead(3) == 1 && Lock == 0){       //按下B
        bleKeyboard.press(KEY_DOWN_ARROW);
        bleKeyboard.releaseAll();
        Serial.println("M1 Key B");
        LED1_2_Countrl(2, 1);
        delay(KeyB);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(2, 0);
        S_last(2, 2);
        KeyB = Nn_KSp;
      }else{
        KeyB = 485;
        Keys_Press_Count(2, 1);
      }

      if(digitalRead(10) == 1 && Lock == 0){       //按下C
        bleKeyboard.press(KEY_RETURN);
        bleKeyboard.releaseAll();
        Serial.println("M1 Key C");
        LED1_2_Countrl(2, 1);
        delay(KeyC);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(3, 0);
        S_last(3, 3);
        KeyC = Nn_KSp;
      }else{
        KeyC = 485;
        Keys_Press_Count(3, 1);
      }

      if(digitalRead(6) == 1 && Lock == 0){       //按下D
        bleKeyboard.press(KEY_LEFT_ALT);
        bleKeyboard.press(KEY_F4);
        bleKeyboard.releaseAll();
        Serial.println("M1 Key D");
        LED1_2_Countrl(2, 1);
        delay(KeyD);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(4, 0);
        S_last(4, 4);
        KeyD = Nn_KSp;
      }else{
        KeyD = 485;
        Keys_Press_Count(4, 1);
      }

      if(digitalRead(19) == 0){       //按下E
        delay(Fast_Key4);
        if((digitalRead(19) == 0) && (!(digitalRead(18) == 0))){
          bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
          Serial.println("M1 Key E");
          LED1_2_Countrl(2, 1);
          delay(KeyE);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(5, 0);
          S_last(5, 5);
          KeyE = Nn_KSp;
          Fast_Key4 = 0;
        }
      }else{
        KeyE = 485;
        Fast_Key4 = 50;
        Keys_Press_Count(5, 1);
      }

      if((digitalRead(1) == 0) && Dsd == 0 && ExitS_CK == 0){       //按下F
        delay(Fast_Key1);
        if((digitalRead(1) == 0) && (!(digitalRead(0) == 0))){      //同时按下反制检测
          bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
          Serial.println("M1 Key F");
          LED1_2_Countrl(2, 1);
          delay(KeyF);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(6, 0);
          S_last(6, 6);
          KeyF = 30;
          Fast_Key1 = 0;
        }
      }else{
        KeyF = 485;
        Fast_Key1 = 50;
        Keys_Press_Count(6, 1);
      }

      if((digitalRead(0) == 0) && Dsd == 0){       //按下G
        delay(Fast_Key2);
        if((digitalRead(0) == 0) && (!(digitalRead(1) == 0))){
          bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
          Serial.println("M1 Key G");
          LED1_2_Countrl(2, 1);
          delay(KeyG);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(7, 0);
          S_last(7, 7);
          KeyG = 30;
          Fast_Key2 = 0;
        }
      }else{
        KeyG = 485;
        Fast_Key2 = 50;
        Keys_Press_Count(7, 1);
      }
      
    }else if(Mode == 2){          //*************************2*************************
      if(digitalRead(2) == 1 && Lock == 0){       //按下A
        bleKeyboard.press(KEY_UP_ARROW);
        bleKeyboard.releaseAll();
        Serial.println("M2 Key A");
        LED1_2_Countrl(2, 1);    //动作指示灯（2）亮/灭
        delay(KeyA);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(1, 0);
        S_last(1, 1);
        KeyA = Nn_KSp;
      }else{
        KeyA = 485;
        Keys_Press_Count(1, 1);
      }

      if(digitalRead(3) == 1 && Lock == 0){       //按下B
        bleKeyboard.press(KEY_DOWN_ARROW);
        bleKeyboard.releaseAll();
        Serial.println("M2 Key B");
        LED1_2_Countrl(2, 1);
        delay(KeyB);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(2, 0);
        S_last(2, 2);
        KeyB = Nn_KSp;
      }else{
        KeyB = 485;
        Keys_Press_Count(2, 1);
      }

      if(digitalRead(10) == 1 && Lock == 0){       //按下C
        bleKeyboard.press(KEY_LEFT_ARROW);
        bleKeyboard.releaseAll();
        Serial.println("M2 Key C");
        LED1_2_Countrl(2, 1);
        delay(KeyC);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(3, 0);
        S_last(8, 3);
        KeyC = Nn_KSp;
      }else{
        KeyC = 485;
        Keys_Press_Count(3, 1);
      }

      if(digitalRead(6) == 1 && Lock == 0){       //按下D
        bleKeyboard.press(KEY_RIGHT_ARROW);
        bleKeyboard.press(KEY_F4);
        bleKeyboard.releaseAll();
        Serial.println("M2 Key D");
        LED1_2_Countrl(2, 1);
        delay(KeyD);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(4, 0);
        S_last(9, 4);
        KeyD = Nn_KSp;
      }else{
        KeyD = 485;
        Keys_Press_Count(4, 1);
      }

      if(digitalRead(19) == 0){       //按下E
        delay(Fast_Key4);
        if((digitalRead(19) == 0) && (!(digitalRead(18) == 0))){
          bleKeyboard.press(KEY_LEFT_GUI);
          bleKeyboard.releaseAll();
          Serial.println("M2 Key E");
          LED1_2_Countrl(2, 1);
          delay(KeyE);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(5, 0);
          S_last(10, 5);
          KeyE = Nn_KSp;
          Fast_Key4 = 0;
        }
      }else{
        KeyE = 485;
        Fast_Key4 = 50;
        Keys_Press_Count(5, 1);
      }

      if((digitalRead(1) == 0) && Dsd == 0 && ExitS_CK == 0){       //按下F
        delay(Fast_Key1);
        if((digitalRead(1) == 0) && (!(digitalRead(0) == 0))){
          bleKeyboard.press(KEY_LEFT_ALT);
          bleKeyboard.press(KEY_TAB);
          bleKeyboard.releaseAll();
          Serial.println("M2 Key F");
          LED1_2_Countrl(2, 1);
          delay(KeyF);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(6, 0);
          S_last(11, 6);
          KeyF = Nn_KSp;
          Fast_Key1 = 0;
        }
      }else{
        KeyF = 485;
        Fast_Key1 = 50;
        Keys_Press_Count(6, 1);
      }

      if((digitalRead(0) == 0) && Dsd == 0){       //按下G
        delay(Fast_Key2);
        if((digitalRead(0) == 0) && (!(digitalRead(1) == 0))){
          bleKeyboard.press(KEY_RETURN);
          bleKeyboard.releaseAll();
          Serial.println("M2 Key G");
          LED1_2_Countrl(2, 1);
          delay(KeyG);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(7, 0);
          S_last(3, 7);
          KeyG = Nn_KSp;
          Fast_Key2 = 0;
        }
      }else{
        KeyG = 485;
        Fast_Key2 = 50;
        Keys_Press_Count(7, 1);
      }

    }else if(Mode == 3){          //*************************3*************************
      if(digitalRead(2) == 1 && Lock == 0){       //按下A
        bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
        Serial.println("M3 Key A");
        LED1_2_Countrl(2, 1);    //动作指示灯（2）亮/灭
        delay(KeyA);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(1, 0);
        S_last(6, 1);
        KeyA = 30;
      }else{
        KeyA = 485;
        Keys_Press_Count(1, 1);
      }

      if(digitalRead(3) == 1 && Lock == 0){       //按下B
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        Serial.println("M3 Key B");
        LED1_2_Countrl(2, 1);
        delay(KeyB);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(2, 0);
        S_last(7, 2);
        KeyB = 30;
      }else{
        KeyB = 485;
        Keys_Press_Count(2, 1);
      }

      if(digitalRead(10) == 1 && Lock == 0){       //按下C
        bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
        Serial.println("M3 Key C");
        LED1_2_Countrl(2, 1);
        delay(KeyC);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(3, 0);
        S_last(5, 3);
        KeyC = Nn_KSp;
      }else{
        KeyC = 485;
        Keys_Press_Count(3, 1);
      }

      if(digitalRead(6) == 1 && Lock == 0){       //按下D
        bleKeyboard.press(KEY_LEFT_ALT);
        bleKeyboard.press(KEY_F4);
        bleKeyboard.releaseAll();
        Serial.println("M3 Key D");
        LED1_2_Countrl(2, 1);
        delay(KeyD);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(4, 0);
        S_last(4, 4);
        KeyD = Nn_KSp;
      }else{
        KeyD = 485;
        Keys_Press_Count(4, 1);
      }

      if(digitalRead(19) == 0){       //按下E     
        delay(Fast_Key4);
        if((digitalRead(19) == 0) && (!(digitalRead(18) == 0))){   
          bleKeyboard.press(KEY_RETURN);
          bleKeyboard.releaseAll();
          Serial.println("M3 Key E");
          LED1_2_Countrl(2, 1);
          delay(KeyE);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(5, 0);
          S_last(3, 5);
          KeyE = Nn_KSp;
          Fast_Key4 = 0;
        }
      }else{
        KeyE = 485;
        Fast_Key4 = 50;
        Keys_Press_Count(5, 1);
      }


      if((digitalRead(1) == 0) && Dsd == 0 && ExitS_CK == 0){       //按下F
        delay(Fast_Key1);
        if((digitalRead(1) == 0) && (!(digitalRead(0) == 0))){
          bleKeyboard.press(KEY_UP_ARROW);
          bleKeyboard.releaseAll();
          Serial.println("M3 Key F");
          LED1_2_Countrl(2, 1);
          delay(KeyF);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(6, 0);
          S_last(1, 6);
          KeyF = Nn_KSp;
          Fast_Key1 = 0;
        }
      }else{
        KeyF = 485;
        Fast_Key1 = 50;
        Keys_Press_Count(6, 1);
      }

      if((digitalRead(0) == 0) && Dsd == 0){       //按下G
        delay(Fast_Key2);
        if((digitalRead(0) == 0) && (!(digitalRead(1) == 0))){
          bleKeyboard.press(KEY_DOWN_ARROW);
          bleKeyboard.releaseAll();
          Serial.println("M3 Key G");
          LED1_2_Countrl(2, 1);
          delay(KeyG);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(7, 0);
          S_last(2, 7);
          KeyG = Nn_KSp;
          Fast_Key2 = 0;
        }
      }else{
        KeyG = 485;
        Fast_Key2 = 50;
        Keys_Press_Count(7, 1);
      }

    }else if(Mode == 4){          //*************************4*************************
      if(digitalRead(2) == 1 && Lock == 0){       //按下A
        bleKeyboard.press(KEY_UP_ARROW);
        bleKeyboard.releaseAll();
        Serial.println("M4 Key A");
        LED1_2_Countrl(2, 1);    //动作指示灯（2）亮/灭
        delay(KeyA);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(1, 0);
        S_last(1, 1);
        KeyA = Nn_KSp;
      }else{
        KeyA = 485;
        Keys_Press_Count(1, 1);
      }

      if(digitalRead(3) == 1 && Lock == 0){       //按下B
        bleKeyboard.press(KEY_DOWN_ARROW);
        bleKeyboard.releaseAll();
        Serial.println("M4 Key B");
        LED1_2_Countrl(2, 1);
        delay(KeyB);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(2, 0);
        S_last(2, 2);
        KeyB = Nn_KSp;
      }else{
        KeyB = 485;
        Keys_Press_Count(2, 1);
      }

      if(digitalRead(10) == 1 && Lock == 0){       //按下C
        bleKeyboard.press(KEY_RETURN);
        bleKeyboard.releaseAll();
        Serial.println("M4 Key C");
        LED1_2_Countrl(2, 1);
        delay(KeyC);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(3, 0);
        S_last(3, 3);
        KeyC = Nn_KSp;
      }else{
        KeyC = 485;
        Keys_Press_Count(3, 1);
      }

      if(digitalRead(6) == 1 && Lock == 0){       //按下D
        bleKeyboard.press(KEY_LEFT_ALT);
        bleKeyboard.press(KEY_F4);
        bleKeyboard.releaseAll();
        Serial.println("M4 Key D");
        LED1_2_Countrl(2, 1);
        delay(KeyD);
        LED1_2_Countrl(2, 0);
        Keys_Press_Count(4, 0);
        S_last(4, 4);
        KeyD = Nn_KSp;
      }else{
        KeyD = 485;
        Keys_Press_Count(4, 1);
      }

      if(digitalRead(19) == 0){       //按下E
        delay(Fast_Key4);
        if((digitalRead(19) == 0) && (!(digitalRead(18) == 0))){
          bleKeyboard.press(KEY_LEFT_ALT);
          bleKeyboard.press(KEY_F4);
          bleKeyboard.releaseAll();
          Serial.println("M4 Key E");
          LED1_2_Countrl(2, 1);
          delay(KeyE);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(5, 0);
          S_last(4, 5);
          KeyE = Nn_KSp;
          Fast_Key4 = 0;
        }
      }else{
        KeyE = 485;
        Fast_Key4 = 50;
        Keys_Press_Count(5, 1);
      }


      if((digitalRead(1) == 0) && Dsd == 0 && ExitS_CK == 0){       //按下F
        delay(Fast_Key1);
        if((digitalRead(1) == 0) && (!(digitalRead(0) == 0))){
          bleKeyboard.press(KEY_UP_ARROW);
          bleKeyboard.releaseAll();
          Serial.println("M4 Key F");
          LED1_2_Countrl(2, 1);
          delay(KeyF);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(6, 0);
          S_last(1, 6);
          KeyF = Nn_KSp;
          Fast_Key1 = 0;
        }
      }else{
        KeyF = 485;
        Fast_Key1 = 50;
        Keys_Press_Count(6, 1);
      }

      if((digitalRead(0) == 0) && Dsd == 0){       //按下G
        delay(Fast_Key2);
        if((digitalRead(0) == 0) && (!(digitalRead(1) == 0))){
          bleKeyboard.press(KEY_DOWN_ARROW);
          bleKeyboard.releaseAll();
          Serial.println("M4 Key G");
          LED1_2_Countrl(2, 1);
          delay(KeyG);
          LED1_2_Countrl(2, 0);
          Keys_Press_Count(7, 0);
          S_last(2, 7);
          KeyG = Nn_KSp;
          Fast_Key2 = 0;
        }
      }else{
        KeyG = 485;
        Fast_Key2 = 50;
        Keys_Press_Count(7, 1);
      }

    }else if(Mode == 5){          //*************************5*************************
      if(LockBottom_L5){    //切换的L5时锁定L5输入
        if((digitalRead(19) == 1) && (digitalRead(1) == 1)){    //按键E和F都是松开状态
          LockBottom_L5 = false;
        }
      }else{
        if(input_count_L5 <= 8){     //输入限制8位数
          // 检测按钮E的状态
          if (digitalRead(19) == 0) {
            // 按钮E被按下
            delay(50); // 延迟一小段时间以防止抖动
            if (digitalRead(19) == 0) {
              if(tag_clear){    //清除标记
                LCD_SetCursor(2,1);
                LCD_Print("        ");
                LCD_SetCursor(2,1);
                tag_clear = false;
              }
              if(FastIned_L5){
                input_count_L5 = 0;
                FastIned_L5 = false;
                // 清空输入缓冲区
                inputBuffer = "";
              }

              inputBuffer += "."; // 将“.”添加到输入缓冲区
              Serial.print(".");
              input_count_L5 += 1;    //输入计数
              if(input_count_L5 <= 8){
              LCD_Print(".");
              }
              delay(200); // 延迟一段时间以避免重复检测到按下
            }
          }

          // 检测按钮F的状态
          if (digitalRead(1) == 0) {
            // 按钮F被按下
            delay(50); // 延迟一小段时间以防止抖动
            if (digitalRead(1) == 0) {
              if(tag_clear){    //清除标记
                LCD_SetCursor(2,1);
                LCD_Print("        ");
                LCD_SetCursor(2,1);
                tag_clear = false;
              }
              if(FastIned_L5){
                input_count_L5 = 0;
                FastIned_L5 = false;
                // 清空输入缓冲区
                inputBuffer = "";
              }

              inputBuffer += "-"; // 将“-”添加到输入缓冲区
              Serial.print("-");
              input_count_L5 += 1;    //输入计数
              if(input_count_L5 <= 8){
              LCD_Print("-");
              }
              delay(200); // 延迟一段时间以避免重复检测到按下
            }
          }
        }else{
          inputBuffer = inputBuffer.substring(0, inputBuffer.length() - 1);      // 删除输入缓冲区的最后一个字符

          input_count_L5 = input_count_L5 - 1;
          FlashLED_3();   //闪灯报错
          Serial.println("");   //换行
          Serial.println("Code limit");
        }

        // 检测按钮G的状态
        if(digitalRead(0) == 0 && !buttonGPressed){
          buttonGPressStartTime = millis();
          buttonGPressed = true;
        }
        if(buttonGPressed){
          if(millis() - buttonGPressStartTime >= 700){   // 按钮G按下时间超过700毫秒
            buttonGPressed = false;
            Sb_Set = 0;
            input_count_L5 = 0;
            FastIned_L5 = false;
            // 清空输入缓冲区
            inputBuffer = "";
            Serial.println("");   //换行
            Serial.println("Input buffer cleared.");
          }else if(digitalRead(0) == 1 && inputBuffer != ""){     // 按钮G按下时间不超过700毫秒
            buttonGPressed = false;
            // 发送输入缓冲区内容
            sendInputBuffer();
            Sb_Set = 0;
          }
          if(digitalRead(0) == 1){
            buttonGPressed = false;
          }
        }
      }
    }
    Lat_Off = 0;
  }else{       //否则未配对
    if(bleKeyboard.isConnected() == false && Connect_Check != 1 && Connect_Check != 2){
      LED1_2_Countrl(1, 0);    //BLE通信指示灯（1）灭
      Serial.println("");   //换行
      Serial.println("BLE offline");
      FlashLED_3();
      inputBuffer = "";      // 清空输入缓冲区
      input_count_L5 = 0;
      FastIned_L5 = false;
      Sb_Set = 0;
      Lat_Off = 1;
      Key_Count = 0;      //键盘计数清除
      Kp_inde = 0;
      BleKeyboard bleKeyboard("Remote control", "ID_drives", 100);
      Serial.println("BLE pairing...");
      FlashLED_3();

      System_Message(1, "Offline");          //LCD提示掉线
      delay(4000);
      LCD_Clear();
      Custom_characters(0);      //自定义字符集0
      handleBLEDisconnect();     //掉线异常探针
      esp_task_wdt_reset();
      
      Connect_Check = 1;
    }
  }
  esp_task_wdt_reset();
}
