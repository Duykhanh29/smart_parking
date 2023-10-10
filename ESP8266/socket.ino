#include <ESP8266WiFi.h>                // Thư viện dùng để kết nối WiFi của ESP8266
#include <WebSocketsClient.h>           // Thư viện WebSocketsClient
//UART
#include <SoftwareSerial.h>
const byte RX = D6;
const byte TX = D5;
SoftwareSerial mySerial = SoftwareSerial(RX, TX);

String UIDReceived = "";
String UID = "";
String statusFromSV = "";
bool stringComplete = false;

const char* ssid = "CHINH CHINH HY";         // Tên của mạng WiFi mà bạn muốn kết nối đến
const char* password = "chinhchinhhy";   // Mật khẩu của mạng WiFi

String result = "";

WebSocketsClient webSocket;

void setup() {
  Serial.begin(115200);                 // Khởi tạo kết nối Serial để truyền dữ liệu đến máy tính
  while (!Serial);
  mySerial.begin(115200);
  while (!mySerial);
  startWiFi();

  connectWebSocket();

}

void loop() {
  webSocket.loop();


  //UID
  read_UID();
}

void read_UID() {
  while (mySerial.available()) {
    char inChar = (char)mySerial.read();
    UIDReceived += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
  if (stringComplete) {
    Serial.print("UID nhận từ Uno: ");
    Serial.println(UIDReceived);

    //Mở đóng gói UID
    unpackUID(UIDReceived);
    webSocket.sendTXT(UID);
    UIDReceived = "";
    stringComplete = false;
  }
}

void unpackUID(String UIDReceived) {
  //Tìm vị trí kí tự
  int findStarChar, findOpenBracketChar = -1;
  findStarChar = UIDReceived.indexOf("*");
  findOpenBracketChar = UIDReceived.indexOf("(");
  //Cắt chuỗi
  if (findStarChar >= 0 && findOpenBracketChar >= 0) {
    UID =  UIDReceived.substring(findStarChar + 1, findOpenBracketChar);
    Serial.print("UID: ");
    Serial.println(UID);
  }
}

void startWiFi() {
  WiFi.begin(ssid, password);           // Kết nối vào mạng WiFi
  Serial.print("Đang kết nối tới");
  Serial.print(ssid);
  // Chờ kết nối WiFi được thiết lập
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\n"); 
  Serial.println("Kết nối đã được thiết lập!");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());       // Gởi địa chỉ IP đến máy tinh
}

void connectWebSocket() {
  webSocket.begin("192.168.1.9", 3000, "/");          // Địa chỉ websocket server, port và URL  ( 3000 là port)
  //  chú ý IP address và port có thể là ip được lấy từ file html. Cụ thể là từ  (var url = window.location.host;)   


  webSocket.onEvent(webSocketEvent);

  // webSocket.setAuthorization("user", "password");        // Sử dụng thông tin chứng thực nếu cần

  webSocket.setReconnectInterval(5000);                     // Thử lại sau 5s nếu kết nối không thành công
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:                         // Sự kiện khi client ngắt kết nối
      Serial.printf("[WSc] Ngắt kết nối!\n");
      break;
    case WStype_CONNECTED:                            // Sự kiện khi client kết nối
      Serial.printf("[WSc] Đã kết nối tới url: %s\n", payload);

      webSocket.sendTXT("Đã kết nối");          // Thông báo kết nối thành công
      break;
    case WStype_TEXT:                                 // Sự kiện khi nhận được thông điệp dạng TEXT
      Serial.printf("[WSc] Lấy đoạn text: %s\n", payload);

      webSocket.sendTXT("Xin chào Server!");               // Gởi thông điệp đến server
      break;
    case WStype_BIN:                                  // Sự kiện khi nhận được thông điệp dạng BINARY
      Serial.printf("\n[WSc] lấy độ dài nhị phân: %u\n", length);
      hexdump(payload, length);
      if (length == 5) {
        statusFromSV = "1111111111";
      }
      else if (length == 6) {
        statusFromSV = "0000000000";
      }
      sendStatus(statusFromSV);


      // webSocket.sendBIN(payload, length);
      break;
  }
}

void sendStatus(String statusFromSV) {
  String statusSend = "";
  //Đóng gói UID
  statusSend = "*" + statusFromSV + "(";
  Serial.print("Gửi tới Uno: ");
  Serial.println(statusSend);
  Serial.flush();
  mySerial.println(statusSend);        //Gửi dữ liệu trong chuỗi statusSend đi qua kết nối Serial (UART), kèm theo ký tự xuống dòng (\n)   
  mySerial.flush();
}
