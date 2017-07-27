#include <SoftwareSerial.h>


const int START_MESS = 0;
const int END_MESS = 1;
const char START_CH = '~';
const char STOP_CH = '$';
const int btn2 = 8;
const int btn1 = 7;
const int btn4 = 6;
const int btn3 = 5;
const int led         = 4;
const String MESS_RESIEVE_ANSWER = "MR";

int state = -1;


const boolean debug = false;

int bluetoothTx = 2;
int bluetoothRx = 3;
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);
int i = 1;


//------------Message--------
const int m_max_size = 50;
char message[m_max_size];
int m_pointer = 0;

//------------Queue----------
const int q_max_size = 20;
String queue[q_max_size];
int q_pointer = 0;
int q_size = 0;
//-------------------------------------------------------------------------


boolean addToMessage(char c) {
  if (m_pointer >= m_max_size) {
    return false;
  } else {
    message[m_pointer] = c;
    m_pointer++;
  }
  return true;
}

void clearMessage() {
  for (int idx = 0; idx < m_max_size; idx++) {
    message[idx] = '\0';
  }
  m_pointer = 0;
}


void printMessage() {
  for (int idx = 0; idx < m_pointer + 1; idx++) {
    if (message[idx] == '\0') {
      Serial.println("");
      return;
    }
    Serial.print(message[idx]);
  }
  Serial.println("");
}

//int messToCardNum() {
//  char trimArr [m_pointer+1];
//   memcpy(trimArr, message, m_pointer+1);
//  return atoi(trimArr);
//}

boolean hasCardNum(String cardNum) {
  if (q_size == 0) {
    return false;
  }
  for (int idx = 0; idx < q_max_size; idx++) {
    String str = queue[idx];
    if (str.equals(cardNum)) {
      return true;
    }
  }
  return false;
}

boolean addCard(String cardNum) {
  if (q_size > q_max_size) {
    return false;
  }
  if (hasCardNum(cardNum)) {
    if (debug) {
      Serial.print(cardNum);
      Serial.println(" Has already!");
    }
    return false;
  } else {
    queue[q_size] = cardNum;
    if (debug) {
      Serial.println(" Add to queue!");
    }
    q_size++;
    return true;
  }
}

String getNextCard() {
  if (q_size == 0) {
    return "N/A";
  } else if (q_pointer > q_size) {
    return "N/A";
  }
  String answer = queue[q_pointer];
  q_pointer++;
  return answer;
}

void resetQueuePointer() {
  if (debug) {
    Serial.println("Pointer reset");
  }
  q_pointer = 0;
}

void clearQueue() {
  for (int idx = 0; idx < q_size; idx++) {
    queue[idx] = "";
  }
  q_pointer = 0;
  q_size = 0;

  if (debug) {
    Serial.println("Queue cleared");
  }
  delay(500);
}

void printCardQueue() {
  for (int idx = 0; idx < q_size; idx++) {
    String str = queue[idx];
    Serial.println(str);
  }
}

//-----------------------------------------------------------------
void setup() {

  Serial.begin(9600);
  bluetooth.begin(9600);

  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(led, OUTPUT);

  while (!Serial) {}


  bluetooth.write("AT+ADDR?");
  delay(500);
  while (bluetooth.available())
  {
    Serial.write(bluetooth.read());
  }


  delay(200);
  Serial.println("");
  bluetooth.write("AT+IMME0");
  delay(500);
  while (bluetooth.available()) {
    Serial.write(bluetooth.read());
  }
  Serial.println("");

  delay(200);
  Serial.println("");
  bluetooth.write("AT+ROLE0");
  delay(500);
  while (bluetooth.available()) {
    Serial.write(bluetooth.read());
  }

  delay(200);
  bluetooth.write("AT+POWE3");
  delay(500);
  while (bluetooth.available())
  {
    Serial.write(bluetooth.read());
  }
  delay(100);
  Serial.println("");


  delay(500);
  bluetooth.write("AT+NAMEPortable");
  delay(500);
  while (bluetooth.available()) {
    Serial.write((char)bluetooth.read());

  }

  Serial.println("");
  bluetooth.write("AT+UUID0x1811"); //add charicteristic
  delay(500);
  while (bluetooth.available()) {
    Serial.write(bluetooth.read());

  }
  Serial.println("");

  Serial.println("");
  bluetooth.write("AT+CHAR0x2A46"); //add charicteristic
  delay(500);
  while (bluetooth.available()) {
    Serial.write(bluetooth.read());

  }
  Serial.println("");
  bluetooth.write("AT+RELI0");
  delay(500);
  while (bluetooth.available()) {
    Serial.write(bluetooth.read());
  }
  Serial.println("");
  bluetooth.write("AT+SHOW1");
  delay(100);
  while (bluetooth.available()) {
    Serial.write(bluetooth.read());

  }
  Serial.println("");

}


void blink() {
  digitalWrite(led, HIGH);
  delay(600);
  digitalWrite(led, LOW);
}

boolean hasBLEConnected() {
  if (debug) {
    Serial.println("CheckingBLE");
  }
  bluetooth.write("AT+PIO1?");
  delay(500);
  while (bluetooth.available())
  {
    String str = bluetooth.readString();

    if (str.equals("OK+Get:0")) {
      if (debug) {
        Serial.println("No connection");
      }
      return false;
    }
  }
  if (debug) {
    Serial.println("Device Connected");
  }
  return true;

}

void sendMessRecieved() {
  for (int idx = 0; idx < MESS_RESIEVE_ANSWER.length(); idx++)
    bluetooth.write(MESS_RESIEVE_ANSWER[idx]);
}

void disconnect() {
  bluetooth.write("AT");

  delay(500);
  while (bluetooth.available())
  {
    String answer = bluetooth.readString();
    if (answer.indexOf("OK") > 0) {
      if (debug) {
        Serial.println("disconnected");
      }

    }
    if (debug) {
      Serial.println("coud not disconnected");
    }
  }


}

void loop()
{


  while (bluetooth.available())
  {

    char toSend = (char)bluetooth.read();

    if (toSend == STOP_CH) {
      state = END_MESS;
    }

    if (state == START_MESS) {
      addToMessage(toSend);
    } else if (state == END_MESS) {
      String str = String(message);
      addCard(str);
      printCardQueue();
      clearMessage();
      state = -1;
      blink();
    }


    if (toSend == START_CH) {
      state = START_MESS;
    }


  }


  int nextCardPressed = digitalRead(btn1);

  int resetPoinetr = digitalRead(btn2);

  int clearQPressed = digitalRead(btn3);

  int disconnectPressed = digitalRead(btn4);


  if (!disconnectPressed) {
    disconnect();
  }

  if (!clearQPressed) {
    clearQueue();
  }

  if (!resetPoinetr) {
    resetQueuePointer();
    delay(500);
  }


  if (!nextCardPressed) {
//    boolean connected = hasBLEConnected();
//    if (connected) {
      String cardStr = getNextCard();
      Serial.println(cardStr);
//      bluetooth.write(START_CH);
      for (int i = 0; i < cardStr.length(); i++)
      {
        bluetooth.write(cardStr[i]);
      }
//      bluetooth.write(STOP_CH);
//    }
    delay(500);
  }
}
