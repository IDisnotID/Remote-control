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

char version[] = "2.3.37";          //*************************版本信息*************************
const char *ssid = "Remote control"; //*************************热点名称*************************
const char *BLE_Address = "ecda3bd25a4a"; //*************************BLE地址*************************

int LED = 1, Connect_Check = 2, KeyA = 485, KeyB = 485, KeyC = 485, KeyD = 485, KeyMode = 485, KeyE = 485, KeyF = 485, KeyG = 485, Key_wifi = 100,
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


BleKeyboard bleKeyboard("Remote control", "ID_Inc.", 100);        //实例化BleKeyboard对象，“Remote control”为键盘名称；"ID_Inc."为制造商
void(* resetFunc) (void) = 0;     //重启函数声明

//****************************************************************Wifi Web Page定义****************************************************************
AsyncWebServer server(80);        // 创建WebServer对象, 端口号80
// 使用端口号80可以直接输入IP访问，使用其它端口需要输入IP:端口号访问
// 一个储存网页的数组
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" type="image/x-icon" href="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADIAAAAyCAYAAAAeP4ixAAAACXBIWXMAAC4jAAAuIwF4pT92AAAAsklEQVRoge2asRGCQBQF7xgDS7ELCEntQy1G7cOUELuwFLOzAS5w0GPnz27IJey8e/8TkEspKQLd1i/wKxShoQiNMCK72sHp+UDO5Xt/zEvPwySiCI0wItWyp5QWS9WQr4ZNmEQUoVHtyPk9N12It/2wqpNhElGEhiI0ttjsf5mGYRJRhEa1I4fxuvouv6ZLsy/oMIkoQkMRGorQUISGIjQUoaEIDUVoZP9FgaEIDUVofABsTRRnSgUS2QAAAABJRU5ErkJggg==">
  <title>Remote control Wifi Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      display: flex;
      flex-direction: column;
      align-items: center;
      text-align: center;
      background-color: #f0f0f0;
    }
    h1, h4, h5, h6 {
      margin: 10px 0;
      color: #333;
    }
    h1 {
      margin-top: 30px;
      white-space: nowrap; /* 不换行 */
    }
    .container {
      width: 90%;
      max-width: 800px;
      margin: 20px auto;
      background: #fff;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 0 10px rgba(255, 255, 255, 0);
      position: relative; /* 让下拉框定位相对于.container */
    }
    .grid-container {
      display: grid;
      gap: 10px;
      margin: 10px 0;
    }
    .grid-container.single {
      max-width: calc(100% / 3.085); /* 设置按钮最大宽度为父容器宽度的1/3 */
      margin: auto;
      grid-template-columns: 1fr;
    }
    .grid-container.triple {
      grid-template-columns: repeat(3, 1fr);
    }
    .grid-container.quad {
      grid-template-columns: repeat(4, 1fr);
    }
    .grid-container.send {
      grid-template-columns: repeat(3, 1fr);
    }
    button {
      width: 100%;
      height: 50px;
      font-size: 16px;
      color: #fff;
      background-color: #d8b86e;
      border: none;
      border-radius: 10px;
      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
      transition: background-color 0.3s, transform 0.3s;
    }
    button:hover {
      background-color: #407e70;
    }
    button:active {
      background-color: #6bc0ae;
      transform: translateY(2px);
    }
    .br-container {
      display: block;
    }
    #textInput {
      width: 95%;
      height: 150px;
      resize: none;
      border-radius: 8px;
      padding: 10px;
      font-family: Arial, sans-serif;
      font-size: 20px;
    }
    .right-align {
        text-align: right;
        color: #888;
        font-size: 15px;
        padding-right: 10px;
    }
    body.no-scroll {
      overflow: hidden;
    }
    .overlay {
      display: none;
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background-color: rgba(0, 0, 0, 0.8);
      color: #fff;
      justify-content: center;
      align-items: center;
      z-index: 1000;
    }
    .overlay-content {
      display: flex;
      flex-direction: column;
      align-items: center;
      text-align: center;
    }
    .spinner-container {
      width: 50vw;
      max-width: 500px;
      border-radius: 5px;
      overflow: hidden;
    }
    .spinner {
      width: 100%;
      height: 8px;
      position: relative;
      overflow: hidden;
    }
    .spinner::before {
      content: "";
      display: block;
      position: absolute;
      border-radius: 5px;
      width: 100%;
      height: 100%;
      background-color: #6bc0ae;
      animation: slide 2s cubic-bezier(0.7, 1, 0.3, 0.6) infinite;
    }
    @keyframes slide {
      0% {
        transform: translateX(-100%);
      }
      50% {
        transform: translateX(100%);
      }
      100% {
        transform: translateX(100%);
      }
    }
    body.dark-mode {
      background-color: #121212;
      color: #e0e0e0;
    }
    body.dark-mode h1, body.dark-mode h4, body.dark-mode h5, body.dark-mode h6 {
      color: #ffffff;
    }
    body.dark-mode .container {
      background: #333;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.5);
    }
    body.dark-mode button {
      background-color: #d8b86e;
    }
    body.dark-mode button:hover {
      background-color: #407e70;
    }
    body.dark-mode button:active {
      background-color: #6bc0ae;
    }
    body.dark-mode .language-selector-container {
      background-color: #444;
      border: 1px solid #555;
    }
    body.dark-mode .language-selector {
      background-color: #444;
      color: #e0e0e0;
      border: none;
      box-shadow: none;
    }
    body.dark-mode .language-selector option {
      background-color: #444;
      color: #e0e0e0;
    }
    body.dark-mode input[type="text"], body.dark-mode input[type="password"], body.dark-mode textarea {
      background-color: #444;
      color: #e0e0e0;
      border: 1px solid #555;
    }
    .dark-mode-toggle {
      display: flex;
      align-items: center;
      position: relative;
      font-size: 12px;
      top: -5px;
      left: 0;
    }
    .switch {
      position: relative;
      display: inline-block;
      width: 34px;
      height: 20px;
      margin-right: 10px;
    }
    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }
    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      transition: .4s;
      border-radius: 34px;
    }
    .slider:before {
      position: absolute;
      content: "";
      height: 12px;
      width: 12px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: .4s;
      border-radius: 50%;
    }
    input:checked + .slider {
      background-color: #6bc0ae;
    }
    input:checked + .slider:before {
      transform: translateX(14px);
    }
    .language-selector-container {
      position: absolute;
      top: 10px;
      right: 10px;
      padding: 5px 10px;
      font-size: 14px;
      border: 1px solid #ccc;
      border-radius: 5px;
      background-color: #fff;
      appearance: none;
      width: 150px; /* 固定宽度 */
    }
    .language-selector {
      border: none;
      background: none;
      cursor: pointer;
      outline: none;
      font-size: 14px;
      width: 100%; /* 撑满容器宽度 */
    }
    .language-selector:focus {
      outline: 2px solid #6bc0ae;
    }
    .footer {
      margin-top: 30px; /* 调整底部距离上方内容的间距 */
      font-size: 12px;
      color: #666;
      font-family: Arial, sans-serif;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .copyright {
      flex: 1; /* 让版权声明占据剩余的空间 */
      text-align: center;
    }
    :focus {
      outline: 3px solid #6bc0ae; /* 示例的外边框颜色 */
      /* 添加其他样式以增强焦点可见性 */
    }
    @media (max-width: 600px) {   /* 小屏设备 */
      body {
        background-color: #fff;
      }
      body.dark-mode {
        background-color: #333;
      }
      h1 {
        font-size: calc(1.2rem + 1.2vw); /* 小屏幕下的字号调整 */
      }
      .container {
        box-shadow: none;
      }
      body.dark-mode .container {
        box-shadow: none;
      }
      .grid-container {
        margin-bottom: 30px; /* 为每类按钮之间增加间距 */
      }
      .grid-container.single {
        margin: auto;
        width: 100%;
        max-width: 100%;
      }
      .grid-container.triple,
      .grid-container.quad {
        grid-template-columns: 1fr;
      }
      .br-container {
        display: none;
      }
    }
    @media (hover: none) {   /* 触摸设备的样式 */
      button:hover {
        background-color: #d8b86e;
      }
      button:active {
        background-color: #6bc0ae;
        transform: none;
      }
      body.dark-mode button:hover {
        background-color: #d8b86e;
      }
      body.dark-mode button:active {
        background-color: #6bc0ae;
      }
      .language-selector:focus {
        outline: none;
      }
      :focus {
        outline: none;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="dark-mode-toggle">
      <label class="switch">
        <input type="checkbox" id="darkmodeswitch" title="Toggle Dark Mode" aria-label="Toggle Dark Mode">
        <span class="slider round"></span>
      </label>
      <span id="darkModeLabel">Dark Mode</span>
    </div>
    <div class="language-selector-container">
      <!-- Language selector -->
      <select id="languageSelect" class="language-selector" onchange="switchLanguage(this.value)" title="Language">
        <option value="en">English</option>
        <option value="es">Español</option>
        <option value="fr">Français</option>
        <option value="de">Deutsch</option>
        <option value="pt">Português</option>
        <option value="ja">日本語</option>
        <option value="ru">Pусский</option>
        <option value="it">Italiano</option>
        <option value="nl">Nederlands</option>
        <option value="ko">한국어</option>
        <option value="ar">العربية</option>
        <option value="zh-CN">简体中文（中国大陆）</option>
        <option value="zh-TW-MO-HK">繁體中文（香港特別行政區、澳門特別行政區、台灣）</option>
        <!-- 多语言选项 -->
      </select>
    </div>
    <h1 id="title">Remote control Wifi Control</h1>
    <h5 id="subtitle">You have enabled wifi control for Remote control, and you can operate it remotely through this page.</h5>
    <hr />

    <h4 id="function-keys">Available Function Keys</h4>

    <div class="grid-container single">
      <button type="button" onclick="sendCommand(1)">Up</button>
    </div>

    <div class="grid-container triple">
      <button type="button" onclick="sendCommand(2)">Left</button>
      <button type="button" onclick="sendCommand(3)">Down</button>
      <button type="button" onclick="sendCommand(4)">Right</button>
    </div>

    <div class="grid-container triple">
      <button type="button" onclick="sendCommand(5)">V-</button>
      <button type="button" onclick="sendCommand(6)">V+</button>
      <button type="button" onclick="sendCommand(7)">Pause/Play</button>  
    </div>

    <div class="grid-container quad">
      <button type="button" onclick="sendCommand(8)">Win</button>
      <button type="button" onclick="sendCommand(9)">Alt+Tab</button>
      <button type="button" onclick="sendCommand(10)">Alt+F4</button>
      <button type="button" onclick="sendCommand(11)">Enter</button>
    </div>
    
    <div class="br-container"><br></div>
    <h4 id="text-input">Text Input</h4>

    <textarea id="textInput" rows="10" oninput="restrictInput(this)" placeholder="Please enter the text you want to send"></textarea>
    <div id="charCount" class="right-align">0 / 200</div>

    <div class="grid-container send">
      <button id="send" type="button">Send</button>
      <button type="button" onclick="sendCommand(12)">Backspace</button>
      <button type="button" onclick="sendCommand(13)">Shift</button>
    </div>

    <h6 id="disclaimer">*The above function keys are available only when Bluetooth is connected and remote control is not locked.</h6>

    <div id="overlay" class="overlay">
      <div class="overlay-content">
        <h2 id="processing">Operation in progress...</h2>
        <div class="spinner-container">
          <div class="spinner"></div>
        </div>
      </div>
    </div>

    <div class="footer">
      <span id="version">v0.0.0</span>
      <span class="copyright">Copyright&copy; 2020-<span id="currentYear"></span> ID_Inc. All rights reserved.</span>
    </div>
  </div>

  <script>
    // 按钮处理
    function sendCommand(buttonNumber) {
      fetch('/sendCommand?button=' + buttonNumber, {
        method: 'GET',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded; charset=utf-8',
          'Cache-Control': 'no-cache' // 设置 Cache-Control 头部
        }
      })
      .then(response => response.text())
      .then(data => {
        if (data === 'ERROR') {
          alert(translations[currentLanguage]['alertMessage']);  // 弹窗提示设备状态错误
        }
        console.log(data);
      })
      .catch(error => {
        console.error('Error:', error);
      });
    }

    // 文本输入
    var sendButton = document.getElementById('send');
    document.getElementById('send').addEventListener('click', function() {
      showOverlay(); // Show the overlay when sending the command
      var text = document.getElementById('textInput').value;
      fetch('/sendText', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded; charset=utf-8',
          'Cache-Control': 'no-cache',
        },
        body: 'text=' + encodeURIComponent(text)
      })
      .then(response => response.text())
      .then(data => {
        hideOverlay(); // Hide the overlay when the response is received
        if (data === 'CLEAR') {
          document.getElementById('textInput').value = '';
          var counterElement = document.getElementById('charCount');
          counterElement.textContent = '0 / 200';
          counterElement.style.color = '#888';
          sendButton.disabled = true;
          sendButton.style.backgroundColor = '#ccc';
          sendButton.style.transform = 'none';
        } else if (data === 'ERROR') {
          alert(translations[currentLanguage]['alertMessage']);
        }
        console.log(data);
      })
      .catch((error) => {
        hideOverlay(); // Hide the overlay in case of an error
        console.error('Error:', error);
      });
    });

    // 叠加层控制
    function showOverlay() {
      document.getElementById('overlay').style.display = 'flex';
      document.body.classList.add('no-scroll');
    }
    function hideOverlay() {
      document.getElementById('overlay').style.display = 'none';
      document.body.classList.remove('no-scroll');
    }

    //字符输入限制
    function restrictInput(element) {
      // 限制输入为ASCII字符
      element.value = element.value.replace(/[^\x00-\x7F]+/g, '');
      // 限制最多输入200个字符
      var maxLength = 200;
      if (element.value.length > maxLength) {
        element.value = element.value.slice(0, maxLength);
      }
      // 更新字符计数器
      var currentLength = element.value.length;
      var counterElement = document.getElementById('charCount');
      counterElement.textContent = currentLength + ' / ' + maxLength;
      // 如果达到最大字符数限制，将计数器字体颜色设置为红色
      if (currentLength >= maxLength) {
          counterElement.style.color = 'red';
      } else {
          counterElement.style.color = '#888'; // 恢复为灰色
      }

      // 启用或禁用发送按钮
      var sendButton = document.getElementById('send');
      if (currentLength === 0) {
        sendButton.disabled = true;
        sendButton.style.backgroundColor = '#ccc'; // 设置禁用状态下的按钮背景颜色
        sendButton.style.transform = 'none'; // 禁用状态下设置transform为none
      } else {
        sendButton.disabled = false;
        sendButton.style.backgroundColor = ''; // 恢复按钮的默认背景颜色
        sendButton.style.transform = ''; // 恢复默认transform
      }
    }

    // 初始化页面时设置发送按钮状态
    document.addEventListener('DOMContentLoaded', function() {
        restrictInput(document.getElementById('textInput'));
    });

    // Function to enable dark mode
    function enableDarkMode() {
      document.body.classList.add('dark-mode');
    }

    // Function to disable dark mode
    function disableDarkMode() {
      document.body.classList.remove('dark-mode');
    }

    // Handle dark mode toggle
    const darkmodeswitch = document.getElementById('darkmodeswitch');
    darkmodeswitch.addEventListener('change', () => {
      if (darkmodeswitch.checked) {
        enableDarkMode();
      } else {
        disableDarkMode();
      }
    });

    // Follow system preference
    if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
      darkmodeswitch.checked = true;
      enableDarkMode();
    } else {
      darkmodeswitch.checked = false;
      disableDarkMode();
    }

    // Listen for changes in the system color scheme
    window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', event => {
      if (!localStorage.getItem('darkMode')) {
        if (event.matches) {
          enableDarkMode();
          darkmodeswitch.checked = true;
        } else {
          disableDarkMode();
          darkmodeswitch.checked = false;
        }
      }
    });

    // 默认语言为英语
    let currentLanguage = 'en';

    // 语言切换函数
    function switchLanguage(language) {
      currentLanguage = language;
      updateTextContent(); // 更新文本内容，包括标题
      updateTitle(); // 更新页面标题
      updateHtmlLangAttribute(); // 更新html标签的lang属性
    }

    // 更新页面标题函数
    function updateTitle() {
      const translations = {
        'zh-CN': 'Remote control Wifi控制',
        'zh-TW-MO-HK': 'Remote control Wifi控制',
        'en': 'Remote control Wifi Control',
        'es': 'Control remoto de Wifi',
        'fr': 'Contrôle à distance du Wifi',
        'de': 'Fernbedienung Wifi Steuerung',
        'pt': 'Controle remoto do Wifi',
        'ja': 'リモートコントロールWifi制御',
        'ru': 'Удаленное управление Wifi',
        'it': 'Controllo remoto Wifi',
        'nl': 'Wifi Afstandsbediening',
        'ko': '리모컨 Wifi 제어',
        'ar': 'التحكم عن بعد عبر Wifi'
        // 多语言标题翻译
      };

      document.title = translations[currentLanguage];
    }

    // 更新HTML标签的lang属性
    function updateHtmlLangAttribute() {
      document.documentElement.lang = currentLanguage;
    }

    // 更新文本内容函数
    const translations = {
      'zh-CN': {
        'title': 'Remote control Wifi控制',
        'subtitle': '您已启用Remote control的wifi控制功能，可以通过此页面进行遥控。',
        'function-keys': '可用功能键',
        'text-input': '文本输入',
        'send': '发送',
        'selector_title': '语言',
        'darkModeLabel': '深色模式',
        'switch_drakmode': '切换深色模式',
        'alertMessage': '设备状态错误，命令未成功执行。 (0x66)',
        'sendtip': '请输入你要发送的文本',
        'processing': '操作进行中...',
        'disclaimer': '*以上功能仅在蓝牙连接且远程控制未锁定时可用。'
      },
      'zh-TW-MO-HK': {
        'title': 'Remote control Wifi控制',
        'subtitle': '您已啟用Remote control的wifi控制功能，可以透過此頁面進行遙控。',
        'function-keys': '可用功能鍵',
        'text-input': '文本輸入',
        'send': '發送',
        'selector_title': '語言',
        'darkModeLabel': '深色模式',
        'switch_drakmode': '切換深色模式',
        'alertMessage': '設備狀態錯誤，命令未成功執行。 (0x66)',
        'sendtip': '請輸入您要發送的文本',
        'processing': '操作進行中...',
        'disclaimer': '*以上功能僅在藍牙連接且遠程控制未鎖定時可用。'
      },
      'en': {
        'title': 'Remote control Wifi Control',
        'subtitle': 'You have enabled wifi control for Remote control, and you can operate it remotely through this page.',
        'function-keys': 'Available Function Keys',
        'text-input': 'Text Input',
        'send': 'Send',
        'selector_title': 'Language',
        'darkModeLabel': 'Dark Mode',
        'switch_drakmode': 'Switch Dark Mode',
        'alertMessage': 'Device status error, command not executed successfully. (0x66)',
        'sendtip': 'Please enter the text you want to send',
        'processing': 'Operation in progress...',
        'disclaimer': '*The above functions are available only when Bluetooth is connected and remote control is not locked.'
      },
      'es': {
        'title': 'Control remoto de Wifi',
        'subtitle': 'Ha habilitado el control wifi para Remote control, y puede operarlo de forma remota a través de esta página.',
        'function-keys': 'Teclas de función disponibles',
        'text-input': 'Entrada de texto',
        'send': 'Enviar',
        'selector_title': 'Idioma',
        'darkModeLabel': 'Modo oscuro',
        'switch_drakmode': 'Cambiar modo oscuro',
        'alertMessage': 'Error de estado del dispositivo, comando no ejecutado correctamente. (0x66)',
        'sendtip': 'Por favor, ingrese el texto que desea enviar',
        'processing': 'Operación en progreso...',
        'disclaimer': '*Las teclas de función anteriores solo están disponibles cuando Bluetooth está conectado y el control remoto no está bloqueado.'
      },
      'fr': {
        'title': 'Contrôle à distance du Wifi',
        'subtitle': 'Vous avez activé le contrôle wifi pour Remote control, et vous pouvez l\'utiliser à distance via cette page.',
        'function-keys': 'Touches de fonction disponibles',
        'text-input': 'Zone de texte',
        'send': 'Envoyer',
        'selector_title': 'Langue',
        'darkModeLabel': 'Mode sombre',
        'switch_drakmode': 'Changer de mode sombre',
        'alertMessage': 'Erreur d\'état du périphérique, commande non exécutée avec succès. (0x66)',
        'sendtip': 'Veuillez saisir le texte que vous souhaitez envoyer',
        'processing': 'Opération en cours...',
        'disclaimer': '*Les touches de fonction ci-dessus ne sont disponibles que lorsque le Bluetooth est connecté et que la télécommande n\'est pas verrouillée.'
      },
      'de': {
        'title': 'Fernbedienung Wifi Steuerung',
        'subtitle': 'Sie haben die WLAN-Steuerung für Remote control aktiviert und können diese über diese Seite fernsteuern.',
        'function-keys': 'Verfügbare Funktionstasten',
        'text-input': 'Texteingabe',
        'send': 'Senden',
        'selector_title': 'Sprache',
        'darkModeLabel': 'Dunkler Modus',
        'switch_drakmode': 'Dunklen Modus umschalten',
        'alertMessage': 'Gerätestatusfehler, Befehl wurde nicht erfolgreich ausgeführt. (0x66)',
        'sendtip': 'Bitte geben Sie den Text ein, den Sie senden möchten',
        'processing': 'Operation läuft...',
        'disclaimer': '*Die oben genannten Funktionstasten sind nur verfügbar, wenn Bluetooth verbunden ist und die Fernbedienung nicht gesperrt ist.'
      },
      'pt': {
        'title': 'Controle remoto do Wifi',
        'subtitle': 'Você habilitou o controle wifi para Remote control e pode operá-lo remotamente através desta página.',
        'function-keys': 'Teclas de função disponíveis',
        'text-input': 'Entrada de texto',
        'send': 'Enviar',
        'selector_title': 'Idioma',
        'darkModeLabel': 'Modo escuro',
        'switch_drakmode': 'Alternar modo escuro',
        'alertMessage': 'Erro de status do dispositivo, comando não executado com sucesso. (0x66)',
        'sendtip': 'Por favor, digite o texto que deseja enviar',
        'processing': 'Operação em andamento...',
        'disclaimer': '*As teclas de função acima estão disponíveis apenas quando o Bluetooth está conectado e o controle remoto não está bloqueado.'
      },
      'ja': {
        'title': 'リモートコントロールWifi制御',
        'subtitle': 'あなたはRemote controlのwifi制御を有効にし、このページを通じてリモートで操作することができます。',
        'function-keys': '利用可能な機能キー',
        'text-input': 'テキスト入力',
        'send': '送信',
        'selector_title': '言語',
        'darkModeLabel': 'ダークモード',
        'switch_drakmode': 'ダークモードを切り替える',
        'alertMessage': 'デバイスの状態エラー、コマンドが正常に実行されませんでした。 (0x66)',
        'sendtip': '送信するテキストを入力してください',
        'processing': '処理中...',
        'disclaimer': '*上記の機能キーは、Bluetoothが接続されており、リモートコントロールがロックされていない場合にのみ使用できます。'
      },
      'ru': {
        'title': 'Удаленное управление Wifi',
        'subtitle': 'Вы активировали беспроводное управление для Remote control, и можете управлять им удаленно через эту страницу.',
        'function-keys': 'Доступные функциональные клавиши',
        'text-input': 'Текстовое поле',
        'send': 'Отправить',
        'selector_title': 'Язык',
        'darkModeLabel': 'Темный режим',
        'switch_drakmode': 'Переключить темный режим',
        'alertMessage': 'Ошибка состояния устройства, команда не выполнена успешно. (0x66)',
        'sendtip': 'Пожалуйста, введите текст, который вы хотите отправить',
        'processing': 'Операция выполняется...',
        'disclaimer': '*Вышеуказанные функциональные клавиши доступны только при подключенном Bluetooth и не заблокированном удаленном управлении.'
      },
      'it': {
        'title': 'Controllo remoto Wifi',
        'subtitle': 'Hai abilitato il controllo wifi per Remote control e puoi utilizzarlo in remoto tramite questa pagina.',
        'function-keys': 'Tasti funzione disponibili',
        'text-input': 'Inserimento di testo',
        'send': 'Invia',
        'selector_title': 'Lingua',
        'darkModeLabel': 'Modalità scura',
        'switch_drakmode': 'Attiva modalità scura',
        'alertMessage': 'Errore di stato del dispositivo, comando non eseguito con successo. (0x66)',
        'sendtip': 'Inserisci il testo che desideri inviare',
        'processing': 'Operazione in corso...',
        'disclaimer': '*I tasti funzione sopra indicati sono disponibili solo quando il Bluetooth è collegato e il controllo remoto non è bloccato.'
      },
      'nl': {
        'title': 'Wifi Afstandsbediening',
        'subtitle': 'U heeft de wifi-bediening voor Remote control ingeschakeld en u kunt deze op afstand bedienen via deze pagina.',
        'function-keys': 'Beschikbare functietoetsen',
        'text-input': 'Tekstinvoer',
        'send': 'Verzenden',
        'selector_title': 'Taal',
        'darkModeLabel': 'Donkere modus',
        'switch_drakmode': 'Donkere modus wisselen',
        'alertMessage': 'Apparaatstatusfout, opdracht niet succesvol uitgevoerd. (0x66)',
        'sendtip': 'Voer de tekst in die je wilt verzenden',
        'processing': 'Verwerking bezig...',
        'disclaimer': '*De bovenstaande functietoetsen zijn alleen beschikbaar wanneer Bluetooth is verbonden en de afstandsbediening niet is vergrendeld.'
      },
      'ko': {
        'title': '리모컨 Wifi 제어',
        'subtitle': '리모컨의 wifi 제어를 활성화했으며, 이 페이지를 통해 원격으로 조작할 수 있습니다.',
        'function-keys': '사용 가능한 기능 키',
        'text-input': '텍스트 입력',
        'send': '보내기',
        'selector_title': '언어',
        'darkModeLabel': '다크 모드',
        'switch_drakmode': '다크 모드 전환',
        'alertMessage': '장치 상태 오류, 명령이 성공적으로 실행되지 않았습니다. (0x66)',
        'sendtip': '전송하려는 텍스트를 입력하세요',
        'processing': '진행 중...',
        'disclaimer': '*위의 기능 키는 블루투스가 연결되고 원격 제어가 잠기지 않은 경우에만 사용할 수 있습니다.'
      },
      'ar': {
        'title': 'التحكم عن بعد عبر Wifi',
        'subtitle': 'لقد قمت بتمكين التحكم عبر wifi للتحكم عن بعد، ويمكنك تشغيله عن بُعد من خلال هذه الصفحة.',
        'function-keys': 'مفاتيح الوظائف المتاحة',
        'text-input': 'إدخال نص',
        'send': 'إرسال',
        'selector_title': 'اللغة',
        'darkModeLabel': 'الوضع الداكن',
        'switch_drakmode': 'تبديل الوضع الداكن',
        'alertMessage': 'خطأ في حالة الجهاز، الأمر لم يتم تنفيذه بنجاح. (0x66)',
        'sendtip': 'الرجاء إدخال النص الذي ترغب في إرساله',
        'processing': 'جاري التشغيل...',
        'disclaimer': '*المفاتيح الوظيفية المذكورة أعلاه متاحة فقط عند اتصال البلوتوث وعدم قفل التحكم عن بُعد.'
      }
      // 多语言翻译
    };

    function updateTextContent() {
      // 更新页面上的文本内容
      Object.keys(translations[currentLanguage]).forEach(key => {
        const element = document.getElementById(key);
        if (element) {
          element.textContent = translations[currentLanguage][key];
        }
      });
      // 更新 textarea 的 placeholder
      const textInput = document.getElementById('textInput');
      if (textInput) {
        textInput.placeholder = translations[currentLanguage]['sendtip'];
      }
      // 更新 select 的 title
      const languageSelect = document.getElementById('languageSelect');
      if (languageSelect) {
        languageSelect.title = translations[currentLanguage]['selector_title'];
      }
      // 更新 switch 的 title
      const darkmodeswitch = document.getElementById('darkmodeswitch');
      if (darkmodeswitch) {
        darkmodeswitch.title = translations[currentLanguage]['switch_darkmode'];
      }
    }

    // 更新版权年份
    document.getElementById('currentYear').textContent = new Date().getFullYear();

    // 请求版本信息
    document.addEventListener('DOMContentLoaded', function () {
      fetch('/version')
        .then(response => response.text())
        .then(data => {
          document.getElementById('version').textContent = data;
        })
        .catch(error => console.error('Error fetching version:', error));
    });

    // 页面加载时初始化文本内容和标题
    updateTextContent();
    updateTitle();

    // 自动检测用户系统语言并设置网页语言
    const userLanguage = navigator.language.toLowerCase(); // 获取用户完整的语言标签并转为小写
    let languageCode = '';

    // 根据用户语言标签截取前两个字符来确定语言
    switch (userLanguage.substring(0, 2)) {
      case 'zh':
        // 检查是否为中文，进一步区分简体和繁体
        if (userLanguage === 'zh-cn' || userLanguage === 'zh-hans' || userLanguage === 'zh-sg') {
          languageCode = 'zh-CN'; // 简体中文
        } else if (userLanguage === 'zh-tw' || userLanguage === 'zh-hant' || userLanguage === 'zh-hk') {
          languageCode = 'zh-TW-MO-HK'; // 繁体中文
        } else {
          // 默认使用简体中文
          languageCode = 'zh-CN';
        }
        break;
      case 'en':
        languageCode = 'en'; // 英语
        break;
      case 'es':
        languageCode = 'es'; // 西班牙语
        break;
      case 'fr':
        languageCode = 'fr'; // 法语
        break;
      case 'de':
        languageCode = 'de'; // 德语
        break;
      case 'pt':
        languageCode = 'pt'; // 葡萄牙语
        break;
      case 'ja':
        languageCode = 'ja'; // 日语
        break;
      case 'ru':
        languageCode = 'ru'; // 俄语
        break;
      case 'it':
        languageCode = 'it'; // 意大利语
        break;
      case 'nl':
        languageCode = 'nl'; // 荷兰语
        break;
      case 'ko':
        languageCode = 'ko'; // 韩语
        break;
      case 'ar':
        languageCode = 'ar'; // 阿拉伯语
        break;
      default:
        languageCode = 'en'; // 默认为英语
        break;
    }

    // 根据识别的语言代码切换网页语言
    switchLanguage(languageCode);
    document.querySelector('.language-selector').value = languageCode; // 更新下拉框选择
  </script>
</body>
</html>
)rawliteral";
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
    LCD_Makechar(0, CG_dat1);     //输出0x0-0x6
    LCD_Makechar(1, CG_dat2);
    LCD_Makechar(2, CG_dat3);
    LCD_Makechar(3, CG_dat4);
    LCD_Makechar(4, CG_dat5);
    LCD_Makechar(5, CG_dat6);
    LCD_Makechar(6, CG_dat18);
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
      break;
    case 12:
      LCD_Write(0x6, 1);      //Backspace
      break;
    case 13:
      LCD_Print("(Text)");    // 文本内容
      break;
    case 14:
      LCD_Write(0x2, 1);      //Shift
    }
  }
}

//**********************************************Wifi遥控函数开始**********************************************
volatile bool isBusy = false; // 定义一个忙标志位
String receivedText = "";     // 用于存储接收到的文本

void Config_Callback(AsyncWebServerRequest *request){  // 下发处理回调函数
  if (isBusy) {
    request->send(200, "text/plain", "BUSY"); // 如果忙，返回忙碌状态
    return;
  }
  if (request->hasParam("value")) { // 如果有值下发
    String HTTP_Payload = request->getParam("value")->value(); // 获取下发的数据
    Serial.printf("[%lu]%s\r\n", millis(), HTTP_Payload.c_str()); // 打印调试信息
  }
  request->send(200, "text/plain", "OK"); // 发送接收成功标志符
}

void sendCommand_Callback(AsyncWebServerRequest *request){       // 处理按钮按下的回调函数
  if (isBusy) {
    request->send(200, "text/plain", "BUSY"); // 如果忙，返回忙碌状态
    return;
  }
  if (request->hasParam("button")) {
    int buttonNumber = request->getParam("button")->value().toInt();
    if (Lock == 0 && bleKeyboard.isConnected()) { // 判断是否应控制
      isBusy = true; // 设置忙标志位
      switch (buttonNumber) {
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
          break;
      case 12:
          bleKeyboard.press(KEY_BACKSPACE);
          bleKeyboard.releaseAll();
          Serial.println("Wifi_Backspace");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(12, 8);
          Keys_Press_Count(19, 0);
          Keys_Press_Count(19, 1);
          break;
      case 13:
          bleKeyboard.press(KEY_LEFT_SHIFT);
          bleKeyboard.releaseAll();
          Serial.println("Wifi_Shift");
          LED1_2_Countrl(2, 1);
          delay(Key_wifi);
          LED1_2_Countrl(2, 0);     
          S_last(14, 8);
          Keys_Press_Count(20, 0);
          Keys_Press_Count(20, 1);
      }
    }else{
      FlashLED_3();
      request->send(200, "text/plain", "ERROR"); // 返回设备状态错误信号
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
    isBusy = false; // 清除忙标志位
    request->send(200, "text/plain", "OK"); // 发送接收成功标志符
    return;
  }
  request->send(400, "text/plain", "Bad Request"); // 发送错误响应
}

// 发送字符串
void sendTextOverBLE(String text) {
  for (int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);
    if (c == '\n') {
      bleKeyboard.press(KEY_RETURN); // 发送换行符
    } else {
      bleKeyboard.press(c); // 发送字符
    }
    delay(15); // 等待一小段时间，确保字符发送完整
    bleKeyboard.releaseAll(); // 释放按键状态，确保下一个字符不会受到影响
    delay(15); // 等待一小段时间，确保释放操作完成
  }
}

// 文本发送功能部分
void sendtextCommand_callback(AsyncWebServerRequest *request){ 
  if (isBusy) {
    request->send(200, "text/plain", "BUSY"); // 如果忙，返回忙碌状态
    return;
  }
  

  if (request->hasParam("text", true)) {
    if (Lock == 0 && bleKeyboard.isConnected()) { // 判断是否应控制
      receivedText = request->getParam("text", true)->value();
      isBusy = true;
      LED1_2_Countrl(2, 1);
      sendTextOverBLE(receivedText);            // 发送文本到BLE
      Serial.println("Wifi_Text: " + receivedText);
      LED1_2_Countrl(2, 0); 
      S_last(13, 8);
      Keys_Press_Count(21, 0);
      Keys_Press_Count(21, 1);

      request->send(200, "text/plain", "CLEAR"); // 返回清空文本框信号
      isBusy = false;
    }else{      // 如果不应该控制
      FlashLED_3();
      request->send(200, "text/plain", "ERROR"); // 返回设备状态错误信号
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
    request->send(200, "text/plain", "OK");
  }else{
    request->send(400, "text/plain", "No text provided");
  }
};

void handleVersionRequest(AsyncWebServerRequest *request) {     // 发送版本信息
    String versionResponse = "v";
    versionResponse += version;
    request->send(200, "text/plain", versionResponse);
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
    server.on("/sendText", HTTP_POST, sendtextCommand_callback);
    server.on("/Up", HTTP_GET, Config_Callback);   // 绑定配置下发的处理函数
    server.on("/version", HTTP_GET, handleVersionRequest);  // 版本信息配置
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
    Change_LED3(1);
    break;
  case 1:
    LCD_Write(0x1, 1);
    LCD_Print("Message");
    FlashLED_3();
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
        LCD_Print("v");
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
          Keys_Press_Count(22, 0);
          Keys_Press_Count(22, 1);
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
  Serial.print("v");
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

  LCD_Print("v");   //Screen 2
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
