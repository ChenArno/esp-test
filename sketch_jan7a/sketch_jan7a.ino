#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <time.h>
// #include <GxEPD2_BW.h>  // 包含 GxEPD2 黑白屏库
#include <GxEPD2_3C.h>
#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>  // 包含 Adafruit GFX 图形库
// 创建屏幕对象，指定型号和引脚 266黑白红
// GxEPD2_3C<GxEPD2_266c, GxEPD2_266c::HEIGHT> display(GxEPD2_266c(/*CS=*/5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));
// 双色 420
GxEPD2_3C<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT> display(GxEPD2_420_GYE042A87(/*CS=*/5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));
const int SWITCH_PIN = 14;// 按键

const int ledPin = 22; // 定义控制LED的引脚 (GPIO 2)
int counter = 0;  // 定义一个计数器变量
// const char* ssid = "Arno2joy_home";
// const char* password = "13757130316";
const char* ssid = "TP-LINK_308";
const char* password = "afd168888";
// NTP服务器相关信息
#define TZ 8      // 时区偏移值，以 UTC+8 为例
#define DST_MN 0  // 夏令时持续时间（分钟）

#define TZ_SEC ((TZ)*3600)     // 时区偏移值（秒）
#define DST_SEC ((DST_MN)*60)  // 夏令时持续时间（秒）

#define DEBOUNCE_DELAY 50  // 防抖延时（毫秒）
unsigned long lastDebounceTime = 0;  // 上次状态变化时间
volatile bool switchTriggered = false;  // 用于标记中断触发
// 中断处理函数
void IRAM_ATTR handleSwitchInterrupt() {
  switchTriggered = true;  // 设置标记
}

// 定义函数返回当前时间字符串
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time Sync Error";  // 如果时间获取失败，返回错误信息
  }

  // 格式化时间为字符串
  char timeString[30] = {0};  // 确保缓冲区初始化为零
  strftime(timeString, sizeof(timeString), "%Y/%m/%d %H:%M:%S", &timeinfo);

  // 返回格式化时间
  return String(timeString);
}

void setup() {
  Serial.begin(115200);

  // SPI.end(); // 先停止SPI，释放资源
  // SPI.begin(13, 14, 14, 4); // 初始化SPI，并指定自定义引脚

  delay(1000);          // 等待串口稳定
  pinMode(ledPin, OUTPUT); // 设置引脚为输出模式
  digitalWrite(ledPin, LOW); // 初始设置为低电平//点亮
    // 设置 GPIO14 为输入模式，并启用上拉电阻
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  // 绑定中断到 GPIO14，在下降沿触发
  attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), handleSwitchInterrupt, FALLING);
   // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  if (time(nullptr) < 1000000000) {
    // 如果没有获取过时间，重新获取时间
    uint8_t i = 0;
    configTime(TZ_SEC, DST_SEC, "ntp1.aliyun.com", "ntp2.aliyun.com");
    while ((time(nullptr) < 1000000000) & (i < 20)) {
      i++;
      Serial.print(".");
      delay(500);
    }
    Serial.println("时间同步成功");
  }
   // 获取时间并打印到屏幕
  // String currentTime = getFormattedTime();
  display.init(115200, true, 2, false);  // 初始化屏幕
  // 启用或禁用调试信息输出。
  // 控制屏幕刷新时的延迟模式，用于优化屏幕的刷新效果。
  // 是否启用快速刷新模式。
  // 设置屏幕内容
  display.setRotation(0);  // 旋转屏幕方向
  display.setFullWindow();  // 使用全屏模式
  display.firstPage();  // 开始绘制
  do {
    display.fillScreen(GxEPD_WHITE);  // 清屏为白色

    // 绘制红色矩形
    // display.fillRect(0, 100, 100, 50, GxEPD_RED);

    display.setTextColor(GxEPD_BLACK);  // 设置文字颜色为黑色
    display.setCursor(10, 20); // 设置文本的起始位置
    display.setTextSize(1); // 设置文字的缩放比例。
    display.println("Hello shaomai");

  } while (display.nextPage());  // 刷新屏幕
}



void loop() {
  int currentState = digitalRead(SWITCH_PIN);
  if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
    if (currentState == LOW) {
         // 获取时间并打印到屏幕
      String currentTime = getFormattedTime();
      Serial.println(currentTime);
        // 显示时间
      display.setPartialWindow(0, 80, 150, 50);
      display.firstPage();
      do {
        display.fillScreen(GxEPD_WHITE);  // 填充背景
        display.setTextColor(GxEPD_BLACK);  // 设置文字颜色为黑色
        display.setCursor(10, 80); // 设置文本的起始位置
        display.setTextSize(1); // 设置文字的缩放比例。
        display.println(currentTime);
      } while (display.nextPage());
    }
    lastDebounceTime = millis();
  }

  // 等待一段时间再继续循环
  // delay(1000);
}