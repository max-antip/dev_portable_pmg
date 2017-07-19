#include <SoftwareSerial.h>


const int START_MESS = 0;
const int END_MESS = 1;
const char START_CH = '~';
const char STOP_CH = '$';
const int buttonNextCard = 8;
const int buttonClearQueue = 9;
const int led         = 4;
const String MESS_RESIEVE_ANSWER = "MR";

int state = -1;

const boolean debug = true;
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
      Serial.print(cardNum);
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
  q_pointer = 0;
}

void clearQueue() {
  for (int idx = 0; idx < q_size; idx++) {
    queue[idx] = "";
  }
  q_pointer = 0;
  q_size = 0;
}

void printCardQueue() {
  for (int idx = 0; idx < q_size; idx++) {
    String str = queue[idx];
    Serial.println(str);
  }
}

//-----------------------------------------------------------------
void setup() {
  pinMode(buttonNextCard, INPUT);
  pinMode(buttonClearQueue, INPUT);
  pinMode(led, OUTPUT);
  Serial.begin(9600);
  bluetooth.begin(9600);

  while (!Serial) {}


  bluetooth.write("AT+ADDR?");
  delay(500);
  while (bluetooth.available())
  {
    Serial.write(bluetooth.read());
  }

  delay(100);


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

  //const int buttonNextCard = 8;
  //const int buttonClearQueue = 9;
  int nextCardPressed = digitalRead(buttonNextCard);

  int clearQPressed = digitalRead(buttonClearQueue);

  if (clearQPressed) {
    resetQueuePointer();
    delay(500);
  }


  if (nextCardPressed) {
    boolean connected = hasBLEConnected();
    if (connected) {
      String cardStr = getNextCard();
      if (debug) {
        Serial.println("Sending card number :" + cardStr);
      }

      bluetooth.write(START_CH);
      for (int i = 0; i < cardStr.length(); i++)
      {
        bluetooth.write(cardStr[i]);
      }
      bluetooth.write(STOP_CH);
    }
    delay(500);
  }
}
