
/*
 * ================================================================================*
 *                            BANLINHKIEN.COM
 * ================================================================================*                           
 *                  Hướng dẫn làm bãi đỗ xe sử dụng RFID-RC522
 * Đấu nối: 
 + RFID:
  -D13: nối chân SCK của RFID      
  -D12: nối chân MISO của RFID
  -D11: nối chân MOSI của RFID      
  -D10: nối chân SS(DSA)của RFID  
  -D9: nối chân RST của RFID 
 + LCD:
  -D3: nối chân RS của LCD  
  -D4: nối chân ENABLE của LCD
  -D5: nối chân D4 của LCD     
  -D6: nối chân D5 của LCD 
  -D7: nối chân D6 của LCD 
  -D8: nối chân D7 của LCD   
 + Loa BUZZER:
  -A0: nối chân Anot của loa   
 + Động cơ Servo:
  -D2: nối chân tín hiệu của động cơ Servo
 */
 
//Khai báo thư viện

#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <LiquidCrystal.h>
//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>

//const char* ssid = "Your-SSID";
//const char* password = "Your-Password";


//Khai báo các chân kết nối của Arduino với các linh kiện

#define SS_PIN 10
#define RST_PIN 9
#define SERVO_PIN 2
#define BUZZER_PIN A0
#define LCD_RS 3
#define LCD_ENABLE 4
#define LCD_D4 5
#define LCD_D5 6
#define LCD_D6 7
#define LCD_D7 8


// code cho thêm vào ( chưa rõ)
const byte RX = 2;
const byte TX = 3;
SoftwareSerial mySerial = SoftwareSerial(RX, TX);
String statusFromESP = "";
bool stringComplete = false;
//kết thúc code cho thêm 

//Khởi tạo các đối tượng (instances) cho các module

MFRC522 mfrc522(SS_PIN, RST_PIN);                                       // Tạo đối tượng cho đọc thẻ RFID
Servo servo;                                                            // Tạo đối tượng cho Servo
LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);  // Tạo đối tượng cho màn hình LCD

int totalSlot=5;
int availableSlots = 5;                                                 // Số lượng chỗ đỗ trống ban đầu
String parkedRFIDs[5];                                                  // Mảng để lưu trữ thông tin về các thẻ RFID đã đỗ

int fireAlarmPin = 11; // Số chân GPIO của cảm biến báo cháy( fireAlarmPin 
bool fireAlarmTriggered = false; // Biến để theo dõi trạng thái báo cháy

// Thêm biến để theo dõi thời gian còi kêu

unsigned long buzzerStartTime = 0;
const unsigned long buzzerDuration = 5000;                              // Thời gian kêu còi: 5 giây

//// Khởi tạo web server 
//
//ESP8266WebServer server(80);                                      // gán cổng số 80 
String UIDSend = "";

void setup() 
{
  Serial.begin(9600);
  SPI.begin();                                                          // Khởi tạo bus SPI
  mfrc522.PCD_Init();                                                   // Khởi tạo đọc thẻ RFID

  servo.attach(SERVO_PIN);
  servo.write(0);                                                       // Đóng Servo ban đầu
  
  pinMode(BUZZER_PIN, OUTPUT);
  
  lcd.begin(16, 2);                                                     // Khởi tạo màn hình LCD 16x2
  lcd.print("Bãi đỗ xe thông minh");
  lcd.setCursor(0, 1);
  lcd.print("Chỗ còn trống: " + String(availableSlots));
  pinMode(fireAlarmPin, INPUT);                                       //  thiết lập chân cảm biến báo cháy là đầu vào


//     // Kết nối Wi-Fi
//    WiFi.begin(ssid, password);
//    while (WiFi.status() != WL_CONNECTED) {
//        delay(1000);
//    }
//    
//    // Định tuyến các đường dẫn và xử lý yêu cầu HTTP
//    server.on("/", HTTP_GET, handleRoot);
//   
//    
//    server.begin();

    
    
}


void loop() 
{
    // Kiểm tra trạng thái cảm biến báo cháy
    if (digitalRead(FIRE_ALARM_PIN) == HIGH) {
    // Báo cháy đã được kích hoạt
    if (digitalRead(fireAlarmPin) == HIGH) {
    // Báo cháy đã được kích hoạt
    if (!fireAlarmTriggered) {
      // Nếu trạng thái trước đó không phải là báo cháy, thực hiện mở rào chắn
      openBarrier();
      fireAlarmTriggered = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Thông báo cháy");
      lcd.setCursor(0, 1);
      lcd.print("Đã mở rào chắn");
    }
    fireAlarmTriggeredctive = true; // Đặt trạng thái báo cháy thành true
  } else {
    // Báo cháy đã tắt
    fireAlarmTriggeredtive = false; // Đặt trạng thái báo cháy thành false
    closeBarrier();
    lcd.clear();
    lcd.print("Đóng rào chắn");
  }
  

    
    // Kiểm tra bãi đỗ xe còn chỗ trống hay không
    if(availableSlots==0){
     lcd.setCursor(0, 0);
     lcd.print("Xin thông cảm");
     lcd.setCursor(0, 1);
     lcd.print("Bãi đỗ xe đã đầy");
     delay(2000);
    }

  
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) 
  {
    String rfid = getRFID();                                            // Đọc mã RFID từ thẻ
    read_ESP();

     /*
      * Tạm thời comment
      * 
      *  if (rfid != "") 
    {
      lcd.clear();
      lcd.print("RFID: " + rfid);

      bool isParkedCar = false;
      for (int i = 0; i < totalSlot; i++) 
      {
        // Xử lý thẻ đã đọc
        if (rfid == parkedRFIDs[i]) 
        {
          isParkedCar = true;
          break;
        }
      }

      if (isParkedCar)                        // xe đã đỗ trong bãi => đang đi ra 
      {
        openBarrier();              // mở servo
        lcd.setCursor(1, 1);
        lcd.print("Hẹn gặp lại!");
        buzzBuzzer(1000);  
        delay(2000);
        lcd.clear();
        lcd.print("Chỗ còn trống: " + String(++availableSlots));
        delay(1000);

        closeBarrier(); // Close the servo
        removeParkedRFID(rfid);                    // xóa thông tin thẻ ra khỏi mảng 
      } 
      else if (availableSlots > 0) 
      {
        openBarrier();                      // Mở 
        lcd.setCursor(4, 1);
        lcd.print("Xin chào!");
        buzzBuzzer(1000);  
        delay(2000);
        lcd.clear();
        lcd.print("Chỗ còn trống: " + String(--availableSlots));
        delay(1000);

        servo.write(0); 
        addParkedRFID(rfid);              // thêm thông tin thẻ vào mảng.
      } 
      else 
      {
        lcd.setCursor(3, 1);
        lcd.print("Bãi đỗ xe đã đầy");
        buzzBuzzer(1000);  
        delay(2000);
      }

      lcd.clear();
      lcd.print("Bãi đỗ xe thông minh");
      lcd.setCursor(0, 1);
      lcd.print("Chỗ còn trống: " + String(availableSlots));
    }

      */
   
      
    // Kiểm tra và điều khiển còi kêu
    
    if (isBuzzerOn()) 
    {
      if (millis() - buzzerStartTime >= buzzerDuration) 
      {
        stopBuzzer();
      }
    }
    mfrc522.PICC_HaltA();                                             // Dừng truyền thẻ RFID
  }
  
  
}

// hàm đọc data từ module ESP8266 thông qua giao tiếp UART (Serial)
void read_ESP(){
  while (mySerial.available()) {           // mySerial.available():  ktra dữ liệu có sẵn trong bộ đệm của UART
    char inChar = (char)mySerial.read();
    statusFromESP += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
  if (stringComplete) {
    Serial.print("Data nhận: ");
    Serial.println(statusFromESP);
    //==============================
    //Xử lý dữ liệu
    handleData(statusFromESP);
    //==============================
    statusFromESP = "";
    stringComplete = false;
  }
}




// mở khi báo cháy
void openBarrier() {=
  servo.write(90); // Mở rào chắn
  
}
// đóng rào sau khi hết cháy
void closeBarrier() {
  servo.write(0); // Đóng rào chắn
}


String getRFID()                                                      // Hàm dùng để đọc mã RFID từ thẻ
{
  String rfid = "";                                                   // Xử lý và định dạng mã RFID
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    rfid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    rfid.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  mfrc522.PICC_HaltA();
  return rfid;
}





// nhiệm vụ: gửi dữ liệu thẻ RFID đến  module ESP8266
void sendUID(String content) {
  UIDSend = "";
  //Đóng gói UID
  UIDSend = "*" + content + "(";
  Serial.print("Gửi tới ESP: ");
  Serial.println(UIDSend);
  Serial.flush();
  mySerial.println(UIDSend);
  mySerial.flush();
}





void BarrierControl(String statusFromESP){
  if(statusFromESP.indexOf("1") >= 0){
     openBarrier(); 
      lcd.setCursor(4, 1);
      lcd.print("Xin chào!");
      buzzBuzzer(1000);  
      delay(2000);
      lcd.clear();
      lcd.print("Chỗ còn trống: " + String(--availableSlots));
      delay(1000);         
  }
  else if(statusFromESP.indexOf("0") >= 0){
    closeBarrier();
    lcd.print("Xin chào!");
    buzzBuzzer(1000);  
    delay(2000);
    lcd.clear();
    lcd.print("Chỗ còn trống: " + String(--availableSlots));
    delay(1000); 
  }
} 

//  xử lý dữ liệu nhận được từ module ESP8266 (được truyền vào qua tham số statusFromESP) và sau đó gọi hàm BarrierControl(statusFromESP)
void handleData(String statusFromESP){
  //Tìm vị trí kí tự
  int findStarChar, findOpenBracketChar = -1;
  findStarChar = statusFromESP.indexOf("*");                     // tìm vị trí của ký tự *
  findOpenBracketChar = statusFromESP.indexOf("(");               // tìm vị trí của ký tự (
  //Cắt chuỗi
  if (findStarChar >= 0 && findOpenBracketChar >= 0) {
    statusFromESP =  statusFromESP.substring(findStarChar + 1, findOpenBracketChar);
    Serial.print("Data đã xử lý: ");
    Serial.println(statusFromESP);
  }
  BarrierControl(statusFromESP);
}



// ý nghĩa:  quét thẻ RFID và xử lý dữ liệu từ thẻ được quét
void ScanCard() {
  if ( ! mfrc522.PICC_IsNewCardPresent())     // ktra có hẻ RFID mới được đặt trên đầu đọc thẻ RFID (mfrc522) hay không. Nếu ko thì return
  {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial())      // ko đọc được data từ thẻ( thẻ ko hợp lệ) => return
  {
    return;
  }
  String content = "";        // content lưu trữ data từ thẻ RFID
  byte letter;

   // vònd for có nhiệm vụ duyệt qua các byte của UID từ RFID và chuyển thành hệ 16 và lưu vào content
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    //    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    //    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  //  Serial.println();
  //  Serial.print("Message : ");
  content.toUpperCase();
  content = content.substring(1);
  Serial.println(content);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UID của bạn:  ");
  lcd.setCursor(0, 1);
  lcd.print(content);
  sendUID(content);
  buzzer = millis();
  analogWrite(6, 230);
}



// thêm xóa thông tin thẻ vào mảng

void addParkedRFID(String rfid)                                       // Hàm dùng để thêm thông tin thẻ đã đỗ vào mảng
{
  for (int i = 0; i < totalSlot; i++)                                         // Thêm thông tin thẻ vào mảng parkedRFIDs
  {
    if (parkedRFIDs[i] == "") 
    {
      parkedRFIDs[i] = rfid;
      break;
    }
  }
}

void removeParkedRFID(String rfid)                                      // Hàm dùng để xóa thông tin thẻ đã rời khỏi mảng
{
  for (int i = 0; i < totalSlot; i++)                                           // Xóa thông tin thẻ ra khỏi mảng parkedRFIDs
  {
    if (parkedRFIDs[i] == rfid) 
    {
      parkedRFIDs[i] = "";
      break;
    }
  }
}



// modulde còi

void buzzBuzzer(unsigned int duration)                                  // Hàm dùng để kêu còi với thời gian cho trước
{
  digitalWrite(BUZZER_PIN, HIGH);                                       // Kêu còi với thời gian đã định sẵn
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

void startBuzzer()                                                      // Hàm dùng để phát tín hiệu cho còi
{
  digitalWrite(BUZZER_PIN, HIGH);                                       // Còi bắt đầu kêu để báo hiệu
  buzzerStartTime = millis();                                           // Lưu thời gian còi bắt đầu kêu

}

void stopBuzzer()                                                       // Hàm dùng để tắt còi
{
  digitalWrite(BUZZER_PIN, LOW);                                        // Dừng phát tín hiệu cho còi
  buzzerStartTime = 0;                                                  // Đặt thời gian bắt đầu kêu còi về 0
}

bool isBuzzerOn()                                                       // Hàm kiểm tra trạng thái của còi có đang kêu hay không
{
  return buzzerStartTime > 0;                                           
}