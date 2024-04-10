#include <LiquidCrystal.h>

#include <Keypad.h>

#include <math.h>

// RW  подключаем к земле

const byte ROWS = 4; //число строк у нашей клавиатуры
const byte COLS = 4; //число столбцов у нашей клавиатуры
char hexaKeys[ROWS][COLS] = {//имена кнопок
{'d','f','1','1'}, 
{'u','b','1','1'},
{'c','1','1','1'},
{'1','1','1','1'}
};

byte rowPins[ROWS] = {13, 12, 11, 10};
byte colPins[COLS] = {3, 2, 1, 0}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //(RS, E, DB4, DB5, DB6, DB7)
long int timeout2 = 0;
long int timeout = 0;
int Room = 2; //текущая комната
int LastRoomCount = 4; //последняя выбранная комната
const int MaxRoom = 4 + 1; //кол-во комнат, индексация ведется с "1"
float term[MaxRoom]; //температура в комнатах
float LastTerm[MaxRoom]; //последняя отмеченная температура 
float PointTerm[MaxRoom]; //нужная температура
float LastPointTerm[MaxRoom]; //последняя нужная температура
bool RelePower[MaxRoom]; //состояние реле 
float TStep = 2; //шаг срабатывания реле на выключение(в градусах)
float TStep2 = 1.01; //шаг срабатывания реле на включение(в градусах)
bool start = 0; //запуск полного отображения

double Thermister(int RawADC) {
  double temp;
  temp = log(10240000 / RawADC - 10000);
  temp = 1 / (0.001129148 + (0.000234145 + (0.0000000876741 * temp * temp)) * temp);
  temp = temp - 273.15;
  return temp;
}
void HiHome() { //старт программы
  lcd.begin(16, 2); //размерность экрана
  lcd.setCursor(0, 0);
  lcd.print("Hello");
  lcd.setCursor(0, 1);
  lcd.print("    Your Home");
  delay(3000);
  lcd.clear();
}

void setup() {
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  for (int i = 1; i < MaxRoom; i++) {
    PointTerm[i] = 25;
  }
  Serial.begin(9600);
  HiHome();
}
void InfoRoom() { // вывод на экран информации по комнате
  if (timeout + 5000 > millis()) {
    infoPanels();
  } else {
    lcd.setCursor(0, 0);
    lcd.print("panel - ");
    lcd.setCursor(8, 0);
    lcd.print(Room);
    lcd.setCursor(9, 0);
    lcd.print("       ");
    lcd.setCursor(0, 1);
    lcd.print("t=");
    lcd.setCursor(2, 1);
    lcd.print(int(term[Room]));
    lcd.setCursor(4, 1);
    lcd.print("    ");
    lcd.setCursor(8, 1);
    lcd.print("Yt=");
    lcd.setCursor(11, 1);
    lcd.print(int(PointTerm[Room]));
    lcd.setCursor(13, 1);
    lcd.print("  ");
  }
}

void infoPanels() {
  lcd.clear();

  lcd.setCursor(0, 1);
  lcd.print(" ");
  lcd.setCursor(1, 0);
  lcd.print(int(term[1]));
  lcd.print("  ");
  lcd.setCursor(5, 0);
  lcd.print(int(term[2]));
  lcd.print("  ");
  lcd.setCursor(9, 0);
  lcd.print(int(term[3]));
  lcd.print("  ");
  lcd.setCursor(13, 0);
  lcd.print(int(term[4]));
  lcd.print(" ");
}

void Logic() {
  //ограничители селектора комнат
  if (Room < 1) {
    Room = MaxRoom - 1;
  }
  if (Room > MaxRoom - 1) {
    Room = 1;
  }

  //флажок для отрисовки
  if (LastRoomCount != Room or((LastTerm[Room] - 0.2) < term[Room] < (LastTerm[Room] + 0.2)) or LastPointTerm[Room] != PointTerm[Room]) {
    InfoRoom();
    LastRoomCount = Room;
    LastTerm[Room] = term[Room];
    LastPointTerm[Room] = PointTerm[Room];
  }
  if (LastRoomCount != Room) {
    timeout -= 5000;
    LastRoomCount = Room;
  }
}

void CheckButt() {
  char customKey = customKeypad.getKey();

  if (customKey) { //если что-то нажато
    switch (customKey) {
    case 'u':
      PointTerm[Room]--;
      break;
    case 'd':
      PointTerm[Room]++;
      break;
    case 'f':
      Room++;
      break;
    case 'b':
      Room--;
      break;
    case 'c':
      timeout = millis();
      infoPanels();
      break;
    }
  }
}

void CheckTerm() {
  term[1] = Thermister(950 - analogRead(A8));
  term[2] = Thermister(950 - analogRead(A9));
  term[3] = Thermister(950 - analogRead(A10));
  term[4] = Thermister(950 - analogRead(A11));
}

void TermoControl() {

  if (term[1] < PointTerm[1] - TStep2) {
    digitalWrite(14, HIGH);

  } else if (term[1] > PointTerm[1] + TStep) {
    digitalWrite(14, LOW);
  }

  if (term[2] < PointTerm[2] - TStep2) {
    digitalWrite(15, HIGH);
  } else if (term[2] > PointTerm[2] + TStep) {
    digitalWrite(15, LOW);
  }

  if (term[3] < PointTerm[3] - TStep2) {
    digitalWrite(16, HIGH);
  } else if (term[3] > PointTerm[3] + TStep) {
    digitalWrite(16, LOW);
  }

  if (term[4] < PointTerm[4] - TStep2) {
    digitalWrite(17, HIGH);
  } else if (term[4] > PointTerm[4] + TStep) {
    digitalWrite(17, LOW);
  }

}

void loop() {

  Logic();

  CheckButt();

  CheckTerm();

  TermoControl();

  if (!start) {
    Room = 1;
    start = !start;
  }
}
