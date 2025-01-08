#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <time.h>
#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>  // GxEPD2 依赖 Adafruit_GFX 库

// 创建 GxEPD2 实例，指定屏幕型号和引脚 2.13 黑白
GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT> display(GxEPD2_213_B72(/*CS=*/5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));

const int ledPin = 2; // 定义控制LED的引脚 (GPIO 2)

const char* ssid = "Arno2joy_home";
const char* password = "13757130316";
// NTP服务器相关信息
#define TZ 8      // 时区偏移值，以 UTC+8 为例
#define DST_MN 0  // 夏令时持续时间（分钟）

#define TZ_SEC ((TZ)*3600)     // 时区偏移值（秒）
#define DST_SEC ((DST_MN)*60)  // 夏令时持续时间（秒）

void setup() {
  Serial.begin(115200);

  // SPI.end(); // 先停止SPI，释放资源
  // SPI.begin(13, 14, 14, 4); // 初始化SPI，并指定自定义引脚

  delay(1000);          // 等待串口稳定
  Serial.print("hello world!");
  pinMode(ledPin, OUTPUT); // 设置引脚为输出模式
  digitalWrite(ledPin, LOW); // 初始设置为低电平
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

  // 清屏并设置初始内容
  display.setRotation(1);  // 屏幕方向
  display.setFullWindow();  // 使用全屏模式

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);  // 白色背景
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(10, 20);
    display.setTextSize(2);
    display.println("Hello, ePaper!");
  } while (display.nextPage());  // 刷新屏幕
}
void loop() {
  static int prevHour = -1;  // 用于存储上一次获取的小时
  static int prevMin = -1;   // 用于存储上一次获取的分钟

  time_t now = time(nullptr);        // 获取当前时间
  struct tm* ltm = localtime(&now);  // 将时间转换为本地时间结构体

  // 如果获取时间失败，打印错误信息并返回
  if (now == (time_t)-1) {
    Serial.println("Failed to obtain time");
    return;
  }
  printLocalTime();
// 读取当前引脚状态
  int currentState = digitalRead(ledPin);

  // 切换电平状态：高变低，低变高
  if (currentState == HIGH) {
    digitalWrite(ledPin, LOW); // 当前高电平，设置为低电平
  } else {
    digitalWrite(ledPin, HIGH); // 当前低电平，设置为高电平
  }
  // 等待一段时间再继续循环
  delay(1000);
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
}