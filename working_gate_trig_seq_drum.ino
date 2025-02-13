int debug = 0; // 1 =on 0 =off

//INTERNAL CLOCK Setup
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 1000;  //the value is a number of milliseconds
const byte ledPin = 4;    //using the built in LED

int tempo = 120;
int lasttempo = 120;

int ledState = LOW;           
long previousMillis = 0;

long interval = 1000;

//EEPROM
#include <EEPROM.h>

//Encoder setting
#define  ENCODER_OPTIMIZE_INTERRUPTS //エンコーダノイズ対策
#include <Encoder.h>
#include <Keypad.h>
// encoder & direction

Encoder myEnc(3, 2);

const int buttonPin = A6; // Пин для подключения кнопки
const int threshold = 512; // Пороговое значение для определения нажатия
const unsigned long debounceDelay = 50; // Задержка для подавления дребезга кнопки
bool buttonPressed = false;
unsigned long lastDebounceTime = 0;

int oldPosition  = -999;
int newPosition = -999;
byte enc = 100; //選択中のエンコーダ。初回起動時はMANUAL表示
byte enc_max = 105; //マニュアルモードではmax=99(16*6ch+option3+mute6)。AUTOでは11(MANUAL,genre,fillin,repeat,sw+mute6)
unsigned int enc_bit = 0x00;//

//ボタン設定
byte button = 0;//0=OFF,1=ON
byte old_button = 0;//チャタリング対策
byte button_on = 0;//チャタリング判定後のボタン状態。0=OFF,1=ON

//OLED settings
#include<Wire.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>

#define OLED_ADDRESS 0x3C
#define SCREEN_WIDTH 128//128が推奨表示
#define SCREEN_HEIGHT 64//64が推奨表示 

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', '4'},
  {'5', '6', '7', '8'},
  {'9', 'A', 'B', 'C'},
  {'D', 'E', 'F', 'G'}
};

byte rowPins[ROWS] = {4, 11, 0, 1};
byte colPins[COLS] = {A0, A1, A2, A3};
Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//----------------変数の定義---------------------------
byte step_count = 0; //clock in毎に1繰り上がり。17になると1に戻る。増えた時にOUTPUTをONする。
byte clock_in = 0 ; //外部クロックの入力状態。H=1,L=0
byte reset_in = 0 ; //外部クロックの入力状態。H=1,L=0
byte old_clock_in = 0 ; //0→1を認識するための変数
byte old_reset_in = 0 ; //0→1を認識するための変数

unsigned int ch1_step = 0x8888; //テスト用
byte CH1_output = 0 ;
byte CH1_mute = 0 ;//0=ミュートしない、1=ミュートする

unsigned int ch2_step = 0x0808; //テスト用
byte CH2_output = 0 ;
byte CH2_mute = 0 ;//0=ミュートしない、1=ミュートする

unsigned int ch3_step = 0xCCCC; //テスト用
byte CH3_output = 0 ;
byte CH3_mute = 0 ;//0=ミュートしない、1=ミュートする

unsigned int ch4_step = 0x2222; //テスト用
byte CH4_output = 0 ;
byte CH4_mute = 0 ;//0=ミュートしない、1=ミュートする

unsigned int ch5_step = 0xFFFF; //テスト用
byte CH5_output = 0 ;
byte CH5_mute = 0 ;//0=ミュートしない、1=ミュートする

unsigned int ch6_step = 0x0000; //テスト用
byte CH6_output = 0 ;
byte CH6_mute = 0 ;//0=ミュートしない、1=ミュートする

// ディスプレイ変数の宣言
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//オプション
byte mode = 0;//0=MANUAL,1=AUTO
byte count_reset = 0;//1でcountが0になる
byte save = 0;//1でseve実行しすぐに0に戻る

byte genre = 0; //パターンジャンル0=techno,1=dub,2=house,3=half
byte repeat = 2; //0=4times,1=8times,2=16times,3=32times,4=eternal
byte fillin = 1; //0=OFF,1=ON
byte sw = 0;//0=2,1=4,2=8,3=16,4=eternal

//AUTOモード
int repeat_max = 4;//repeat_doneこの値に達したらフィルイン
int repeat_done = 0; //stepが16になったら繰り上がる。
byte test = 0; //開発用
byte change_bnk1 = 1;//プリセット1
byte change_bnk2 = 1;//プリセット2
byte change_bnk3 = 1;//プリセット3（任意に増やしてOK）
byte change_bnk4 = 1;
byte sw_max = 1;//se_doneがこの値に達したらパターン変更（ランダム）
byte sw_done = 0;//repeat_maxになったら繰り上がる


//preset step pattern
//[パターンの数][CHの数*2。頭6個が通常時、後ろ6個がfillin]
//16信数はドラムパターンを示す。

//bnk1 TECHNO
const static word bnk1_ptn[8][12] PROGMEM = {
 { 0x8888,  0x0808, 0xDDDD, 0x2222, 0x1000, 0x0022,
   0x8888, 0x88AF, 0xDDDD, 0x2222, 0x1000, 0x0022
 },
 { 0x8888,  0x0808, 0xFFFF, 0x2222, 0x1000, 0x0022,
   0x8888, 0x88AF, 0xFFFF, 0x2222, 0x1000, 0x0022
 },
 { 0x8888,  0x0808, 0xCCCC, 0x2222, 0x1000, 0x0022,
   0x8888, 0x88AF, 0xCCCC, 0x2222, 0x1000, 0x0022
 },
 { 0x8888,  0x0808, 0x4545, 0x2222, 0x1000, 0x0022,
   0x88AF, 0x0808, 0x4545, 0x2222, 0x1000, 0x0022
 },
 { 0x8888,  0x0808, 0xFFFF, 0x2222, 0x1000, 0x0022,
   0x88AF, 0x0808, 0xFFFF, 0x2222, 0x1000, 0x0022
 },
 { 0x8888,  0x0809, 0xDDDD, 0x2222, 0x1000, 0x0022,
   0x0000, 0x0809, 0xDDDD, 0x2222, 0x1000, 0x0022
 },
 { 0x8888,  0x0849, 0xDDDD, 0x2222, 0x1000, 0x0022,
   0x0000, 0x0849, 0xDDDD, 0x2222, 0x1000, 0x0022
 },
 { 0x8888,  0x0802, 0xDDDD, 0x2222, 0x1000, 0x0022,
   0x8896, 0x0869, 0xDDDD, 0x2222, 0x1000, 0x0022
 }
};

//bunk2 DUBTECHNO
const static word bnk2_ptn[8][12] PROGMEM = {
 { 0x8888,  0x0808, 0xDDDD, 0x2222, 0x1240, 0x0022,
   0x8888, 0x0809, 0xDDDD, 0x2222, 0x1240, 0x0022
 },
 { 0x8888,  0x0808, 0xFFFF, 0x2222, 0x1240, 0x0022,
   0x000A, 0x0849, 0xDDDD, 0x2222, 0x1000, 0x0022
 },
 { 0x8889,  0x0808, 0xCCCC, 0x2222, 0x1240, 0x0022,
   0x8888, 0x0000, 0xCCCC, 0x2222, 0x1240, 0x0022
 },
 { 0x8889,  0x0808, 0x4545, 0x2222, 0x1240, 0x0022,
   0x8888, 0x0809, 0x4545, 0x2222, 0x1240, 0x0022
 },
 { 0x888C,  0x0808, 0xFFFF, 0x2222, 0x1240, 0x0022,
   0x8888, 0x8888, 0xFFFF, 0x2222, 0x1240, 0x0022
 },
 { 0x888C,  0x0809, 0xDDDD, 0x2222, 0x1240, 0x0022,
   0x999F, 0x0000, 0xDDDD, 0x2222, 0x1240, 0x0022
 },
 { 0x0000,  0x0849, 0xDDDD, 0x2222, 0x1240, 0x0022,
   0x000A, 0x0849, 0xDDDD, 0x2222, 0x1000, 0x0022
 },
 { 0x0000,  0x0802, 0xDDDD, 0x2222, 0x1240, 0x0022,
   0x000A, 0x0802, 0xDDDD, 0x2222, 0x1000, 0x0022
 }
};

//bunk3 HOUSE
const static word bnk3_ptn[8][12]PROGMEM = {
 { 0x8888, 0x0808, 0x2222, 0x0000, 0x0040, 0x0101,
   0x8888, 0x88AF, 0x2222, 0x0000, 0x0040, 0x0101
 },
 { 0x888A, 0x0808, 0x2323, 0x0000, 0x0040, 0x0101,
   0x8888, 0x88AF, 0x2323, 0x0000, 0x0040, 0x0101
 },
 { 0x8888, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x0101,
   0x8888, 0x88AF, 0xCCCC, 0x2222, 0x0040, 0x0101
 },
 { 0x888A, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x0101,
   0x8888, 0x88AF, 0xCCCC, 0x2222, 0x0040, 0x0101
 },
 { 0x8888, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101,
   0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101
 },
 { 0x888A, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101,
   0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101
 },
 { 0x888A, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x4112,
   0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x4112
 },
 { 0x8888, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x4112,
   0x88AF, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x4112
 }
};

//bank4 HALFSTEP
const static word bnk4_ptn[8][12]PROGMEM = {
 { 0x8888, 0x0808, 0x0000, 0x2222, 0x0040, 0x0101,
   0x8888, 0x88AF, 0x2222, 0x0040, 0x0000, 0x0101
 },
 { 0x888A, 0x0808, 0x2323, 0x0000, 0x0040, 0x0101,
   0x8888, 0x88AF, 0x2323, 0x0000, 0x0040, 0x0101
 },
 { 0x8888, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x0101,
   0x8888, 0x88AF, 0xCCCC, 0x2222, 0x0040, 0x0101
 },
 { 0x888A, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x0101,
   0x8888, 0x88AF, 0xCCCC, 0x2222, 0x0040, 0x0101
 },
 { 0x8888, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101,
   0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101
 },
 { 0x888A, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101,
   0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x0101
 },
 { 0x888A, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x4112,
   0x0000, 0x0808, 0xFFFF, 0x2222, 0x0040, 0x4112
 },
 { 0x8888, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x4112,
   0x88AF, 0x0808, 0xCCCC, 0x2222, 0x0040, 0x4112
 }
};


void setup() {
Serial.end();

 // ディスプレイの初期化
 display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
 #ifdef PANEL_USD
 display.setRotation(2);  // 180 degree rotation for upside-down use
 #else
 display.setRotation(0);  // Normal orientation
 #endif
 pinMode(11, INPUT); //RST
 pinMode(12, INPUT_PULLUP); //BUTTON
 pinMode(5, OUTPUT); //CH1
 pinMode(6, OUTPUT); //CH2
 pinMode(7, OUTPUT); //CH3
 pinMode(8, OUTPUT); //CH4
 pinMode(9, OUTPUT); //CH5
 pinMode(10, OUTPUT); //CH6
 
 pinMode(13, INPUT); //Trigger

 //保存データの読み出し
 ch1_step = EEPROM.read(1) * 256 + EEPROM.read(2);
 ch2_step = EEPROM.read(3) * 256 + EEPROM.read(4);
 ch3_step = EEPROM.read(5) * 256 + EEPROM.read(6);
 ch4_step = EEPROM.read(7) * 256 + EEPROM.read(8);
 ch5_step = EEPROM.read(9) * 256 + EEPROM.read(10);
 ch6_step = EEPROM.read(11) * 256 + EEPROM.read(12);

 //開発用通信設定
 //  Serial.begin(9600);

 OLED_display();
}


void ch1() {

char key = customKeypad.getKey();

//ch1---------------------------------------------------------------------------------------------
if (enc == 100 && key == '1') {
  if (bitRead(ch1_step, 15) == 0) {
    bitWrite(ch1_step, 15, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 15) == 1) {
    bitWrite(ch1_step, 15, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == '2') {
  if (bitRead(ch1_step, 14) == 0) {
    bitWrite(ch1_step, 14, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 14) == 1) {
    bitWrite(ch1_step, 14, 0); // Устанавливаем бит в 0
  }
}

if (enc == 100 && key == '3') {
  if (bitRead(ch1_step, 13) == 0) {
    bitWrite(ch1_step, 13, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 13) == 1) {
    bitWrite(ch1_step, 13, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == '4') {
  if (bitRead(ch1_step, 12) == 0) {
    bitWrite(ch1_step, 12, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 12) == 1) {
    bitWrite(ch1_step, 12, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == '5') {
  if (bitRead(ch1_step, 11) == 0) {
    bitWrite(ch1_step, 11, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 11) == 1) {
    bitWrite(ch1_step, 11, 0); // Устанавливаем бит в 0
  }
}

if (enc == 100 && key == '6') {
  if (bitRead(ch1_step, 10) == 0) {
    bitWrite(ch1_step, 10, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 10) == 1) {
    bitWrite(ch1_step, 10, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == '7') {
  if (bitRead(ch1_step, 9) == 0) {
    bitWrite(ch1_step, 9, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 9) == 1) {
    bitWrite(ch1_step, 9, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == '8') {
  if (bitRead(ch1_step, 8) == 0) {
    bitWrite(ch1_step, 8, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 8) == 1) {
    bitWrite(ch1_step, 8, 0); // Устанавливаем бит в 0
  }
}

if (enc == 100 && key == '9') {
  if (bitRead(ch1_step, 7) == 0) {
    bitWrite(ch1_step, 7, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 7) == 1) {
    bitWrite(ch1_step, 7, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == 'A') {
  if (bitRead(ch1_step, 6) == 0) {
    bitWrite(ch1_step, 6, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 6) == 1) {
    bitWrite(ch1_step, 6, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == 'B') {
  if (bitRead(ch1_step, 5) == 0) {
    bitWrite(ch1_step, 5, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 5) == 1) {
    bitWrite(ch1_step, 5, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == 'C') {
  if (bitRead(ch1_step, 4) == 0) {
    bitWrite(ch1_step, 4, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 4) == 1) {
    bitWrite(ch1_step, 4, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == 'D') {
  if (bitRead(ch1_step, 3) == 0) {
    bitWrite(ch1_step, 3, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 3) == 1) {
    bitWrite(ch1_step, 3, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == 'E') {
  if (bitRead(ch1_step, 2) == 0) {
    bitWrite(ch1_step, 2, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 2) == 1) {
    bitWrite(ch1_step, 2, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == 'F') {
  if (bitRead(ch1_step, 1) == 0) {
    bitWrite(ch1_step, 1, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 1) == 1) {
    bitWrite(ch1_step, 1, 0); // Устанавливаем бит в 0
  }
}
if (enc == 100 && key == 'G') {
  if (bitRead(ch1_step, 0) == 0) {
    bitWrite(ch1_step, 0, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch1_step, 0) == 1) {
    bitWrite(ch1_step, 0, 0); // Устанавливаем бит в 0
  }
}

}

void ch2(){

char key = customKeypad.getKey();
//ch2---------------------------------------------------------------------------------------------
if (enc == 101 && key == '1') {
  if (bitRead(ch2_step, 15) == 0) {
    bitWrite(ch2_step, 15, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 15) == 1) {
    bitWrite(ch2_step, 15, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == '2') {
  if (bitRead(ch2_step, 14) == 0) {
    bitWrite(ch2_step, 14, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 14) == 1) {
    bitWrite(ch2_step, 14, 0); // Устанавливаем бит в 0
  }
}

if (enc == 101 && key == '3') {
  if (bitRead(ch2_step, 13) == 0) {
    bitWrite(ch2_step, 13, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 13) == 1) {
    bitWrite(ch2_step, 13, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == '4') {
  if (bitRead(ch2_step, 12) == 0) {
    bitWrite(ch2_step, 12, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 12) == 1) {
    bitWrite(ch2_step, 12, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == '5') {
  if (bitRead(ch2_step, 11) == 0) {
    bitWrite(ch2_step, 11, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 11) == 1) {
    bitWrite(ch2_step, 11, 0); // Устанавливаем бит в 0
  }
}

if (enc == 101 && key == '6') {
  if (bitRead(ch2_step, 10) == 0) {
    bitWrite(ch2_step, 10, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 10) == 1) {
    bitWrite(ch2_step, 10, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == '7') {
  if (bitRead(ch2_step, 9) == 0) {
    bitWrite(ch2_step, 9, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 9) == 1) {
    bitWrite(ch2_step, 9, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == '8') {
  if (bitRead(ch2_step, 8) == 0) {
    bitWrite(ch2_step, 8, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 8) == 1) {
    bitWrite(ch2_step, 8, 0); // Устанавливаем бит в 0
  }
}

if (enc == 101 && key == '9') {
  if (bitRead(ch2_step, 7) == 0) {
    bitWrite(ch2_step, 7, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 7) == 1) {
    bitWrite(ch2_step, 7, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == 'A') {
  if (bitRead(ch2_step, 6) == 0) {
    bitWrite(ch2_step, 6, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 6) == 1) {
    bitWrite(ch2_step, 6, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == 'B') {
  if (bitRead(ch2_step, 5) == 0) {
    bitWrite(ch2_step, 5, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 5) == 1) {
    bitWrite(ch2_step, 5, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == 'C') {
  if (bitRead(ch2_step, 4) == 0) {
    bitWrite(ch2_step, 4, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 4) == 1) {
    bitWrite(ch2_step, 4, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == 'D') {
  if (bitRead(ch2_step, 3) == 0) {
    bitWrite(ch2_step, 3, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 3) == 1) {
    bitWrite(ch2_step, 3, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == 'E') {
  if (bitRead(ch2_step, 2) == 0) {
    bitWrite(ch2_step, 2, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 2) == 1) {
    bitWrite(ch2_step, 2, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == 'F') {
  if (bitRead(ch2_step, 1) == 0) {
    bitWrite(ch2_step, 1, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 1) == 1) {
    bitWrite(ch2_step, 1, 0); // Устанавливаем бит в 0
  }
}
if (enc == 101 && key == 'G') {
  if (bitRead(ch2_step, 0) == 0) {
    bitWrite(ch2_step, 0, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch2_step, 0) == 1) {
    bitWrite(ch2_step, 0, 0); // Устанавливаем бит в 0
  }
}

}

void ch3(){
  char key = customKeypad.getKey();
//ch3---------------------------------------------------------------------------------------------
if (enc == 102 && key == '1') {
  if (bitRead(ch3_step, 15) == 0) {
    bitWrite(ch3_step, 15, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 15) == 1) {
    bitWrite(ch3_step, 15, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == '2') {
  if (bitRead(ch3_step, 14) == 0) {
    bitWrite(ch3_step, 14, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 14) == 1) {
    bitWrite(ch3_step, 14, 0); // Устанавливаем бит в 0
  }
}

if (enc == 102 && key == '3') {
  if (bitRead(ch3_step, 13) == 0) {
    bitWrite(ch3_step, 13, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 13) == 1) {
    bitWrite(ch3_step, 13, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == '4') {
  if (bitRead(ch3_step, 12) == 0) {
    bitWrite(ch3_step, 12, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 12) == 1) {
    bitWrite(ch3_step, 12, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == '5') {
  if (bitRead(ch3_step, 11) == 0) {
    bitWrite(ch3_step, 11, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 11) == 1) {
    bitWrite(ch3_step, 11, 0); // Устанавливаем бит в 0
  }
}

if (enc == 102 && key == '6') {
  if (bitRead(ch3_step, 10) == 0) {
    bitWrite(ch3_step, 10, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 10) == 1) {
    bitWrite(ch3_step, 10, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == '7') {
  if (bitRead(ch3_step, 9) == 0) {
    bitWrite(ch3_step, 9, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 9) == 1) {
    bitWrite(ch3_step, 9, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == '8') {
  if (bitRead(ch3_step, 8) == 0) {
    bitWrite(ch3_step, 8, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 8) == 1) {
    bitWrite(ch3_step, 8, 0); // Устанавливаем бит в 0
  }
}

if (enc == 102 && key == '9') {
  if (bitRead(ch3_step, 7) == 0) {
    bitWrite(ch3_step, 7, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 7) == 1) {
    bitWrite(ch3_step, 7, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == 'A') {
  if (bitRead(ch3_step, 6) == 0) {
    bitWrite(ch3_step, 6, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 6) == 1) {
    bitWrite(ch3_step, 6, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == 'B') {
  if (bitRead(ch3_step, 5) == 0) {
    bitWrite(ch3_step, 5, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 5) == 1) {
    bitWrite(ch3_step, 5, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == 'C') {
  if (bitRead(ch3_step, 4) == 0) {
    bitWrite(ch3_step, 4, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 4) == 1) {
    bitWrite(ch3_step, 4, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == 'D') {
  if (bitRead(ch3_step, 3) == 0) {
    bitWrite(ch3_step, 3, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 3) == 1) {
    bitWrite(ch3_step, 3, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == 'E') {
  if (bitRead(ch3_step, 2) == 0) {
    bitWrite(ch3_step, 2, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 2) == 1) {
    bitWrite(ch3_step, 2, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == 'F') {
  if (bitRead(ch3_step, 1) == 0) {
    bitWrite(ch3_step, 1, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 1) == 1) {
    bitWrite(ch3_step, 1, 0); // Устанавливаем бит в 0
  }
}
if (enc == 102 && key == 'G') {
  if (bitRead(ch3_step, 0) == 0) {
    bitWrite(ch3_step, 0, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch3_step, 0) == 1) {
    bitWrite(ch3_step, 0, 0); // Устанавливаем бит в 0
  }
}

}


void ch4(){
  char key = customKeypad.getKey();
//ch4---------------------------------------------------------------------------------------------
if (enc == 103 && key == '1') {
  if (bitRead(ch4_step, 15) == 0) {
    bitWrite(ch4_step, 15, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 15) == 1) {
    bitWrite(ch4_step, 15, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == '2') {
  if (bitRead(ch4_step, 14) == 0) {
    bitWrite(ch4_step, 14, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 14) == 1) {
    bitWrite(ch4_step, 14, 0); // Устанавливаем бит в 0
  }
}

if (enc == 103 && key == '3') {
  if (bitRead(ch4_step, 13) == 0) {
    bitWrite(ch4_step, 13, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 13) == 1) {
    bitWrite(ch4_step, 13, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == '4') {
  if (bitRead(ch4_step, 12) == 0) {
    bitWrite(ch4_step, 12, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 12) == 1) {
    bitWrite(ch4_step, 12, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == '5') {
  if (bitRead(ch4_step, 11) == 0) {
    bitWrite(ch4_step, 11, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 11) == 1) {
    bitWrite(ch4_step, 11, 0); // Устанавливаем бит в 0
  }
}

if (enc == 103 && key == '6') {
  if (bitRead(ch4_step, 10) == 0) {
    bitWrite(ch4_step, 10, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 10) == 1) {
    bitWrite(ch4_step, 10, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == '7') {
  if (bitRead(ch4_step, 9) == 0) {
    bitWrite(ch4_step, 9, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 9) == 1) {
    bitWrite(ch4_step, 9, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == '8') {
  if (bitRead(ch4_step, 8) == 0) {
    bitWrite(ch4_step, 8, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 8) == 1) {
    bitWrite(ch4_step, 8, 0); // Устанавливаем бит в 0
  }
}

if (enc == 103 && key == '9') {
  if (bitRead(ch4_step, 7) == 0) {
    bitWrite(ch4_step, 7, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 7) == 1) {
    bitWrite(ch4_step, 7, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == 'A') {
  if (bitRead(ch4_step, 6) == 0) {
    bitWrite(ch4_step, 6, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 6) == 1) {
    bitWrite(ch4_step, 6, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == 'B') {
  if (bitRead(ch4_step, 5) == 0) {
    bitWrite(ch4_step, 5, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 5) == 1) {
    bitWrite(ch4_step, 5, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == 'C') {
  if (bitRead(ch4_step, 4) == 0) {
    bitWrite(ch4_step, 4, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 4) == 1) {
    bitWrite(ch4_step, 4, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == 'D') {
  if (bitRead(ch4_step, 3) == 0) {
    bitWrite(ch4_step, 3, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 3) == 1) {
    bitWrite(ch4_step, 3, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == 'E') {
  if (bitRead(ch4_step, 2) == 0) {
    bitWrite(ch4_step, 2, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 2) == 1) {
    bitWrite(ch4_step, 2, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == 'F') {
  if (bitRead(ch4_step, 1) == 0) {
    bitWrite(ch4_step, 1, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 1) == 1) {
    bitWrite(ch4_step, 1, 0); // Устанавливаем бит в 0
  }
}
if (enc == 103 && key == 'G') {
  if (bitRead(ch4_step, 0) == 0) {
    bitWrite(ch4_step, 0, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch4_step, 0) == 1) {
    bitWrite(ch4_step, 0, 0); // Устанавливаем бит в 0
  }
}


}

void ch5() {
  char key = customKeypad.getKey();
//ch5---------------------------------------------------------------------------------------------
if (enc == 104 && key == '1') {
  if (bitRead(ch5_step, 15) == 0) {
    bitWrite(ch5_step, 15, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 15) == 1) {
    bitWrite(ch5_step, 15, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == '2') {
  if (bitRead(ch5_step, 14) == 0) {
    bitWrite(ch5_step, 14, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 14) == 1) {
    bitWrite(ch5_step, 14, 0); // Устанавливаем бит в 0
  }
}

if (enc == 104 && key == '3') {
  if (bitRead(ch5_step, 13) == 0) {
    bitWrite(ch5_step, 13, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 13) == 1) {
    bitWrite(ch5_step, 13, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == '4') {
  if (bitRead(ch5_step, 12) == 0) {
    bitWrite(ch5_step, 12, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 12) == 1) {
    bitWrite(ch5_step, 12, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == '5') {
  if (bitRead(ch5_step, 11) == 0) {
    bitWrite(ch5_step, 11, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 11) == 1) {
    bitWrite(ch5_step, 11, 0); // Устанавливаем бит в 0
  }
}

if (enc == 104 && key == '6') {
  if (bitRead(ch5_step, 10) == 0) {
    bitWrite(ch5_step, 10, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 10) == 1) {
    bitWrite(ch5_step, 10, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == '7') {
  if (bitRead(ch5_step, 9) == 0) {
    bitWrite(ch5_step, 9, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 9) == 1) {
    bitWrite(ch5_step, 9, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == '8') {
  if (bitRead(ch5_step, 8) == 0) {
    bitWrite(ch5_step, 8, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 8) == 1) {
    bitWrite(ch5_step, 8, 0); // Устанавливаем бит в 0
  }
}

if (enc == 104 && key == '9') {
  if (bitRead(ch5_step, 7) == 0) {
    bitWrite(ch5_step, 7, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 7) == 1) {
    bitWrite(ch5_step, 7, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == 'A') {
  if (bitRead(ch5_step, 6) == 0) {
    bitWrite(ch5_step, 6, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 6) == 1) {
    bitWrite(ch5_step, 6, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == 'B') {
  if (bitRead(ch5_step, 5) == 0) {
    bitWrite(ch5_step, 5, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 5) == 1) {
    bitWrite(ch5_step, 5, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == 'C') {
  if (bitRead(ch5_step, 4) == 0) {
    bitWrite(ch5_step, 4, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 4) == 1) {
    bitWrite(ch5_step, 4, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == 'D') {
  if (bitRead(ch5_step, 3) == 0) {
    bitWrite(ch5_step, 3, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 3) == 1) {
    bitWrite(ch5_step, 3, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == 'E') {
  if (bitRead(ch5_step, 2) == 0) {
    bitWrite(ch5_step, 2, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 2) == 1) {
    bitWrite(ch5_step, 2, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == 'F') {
  if (bitRead(ch5_step, 1) == 0) {
    bitWrite(ch5_step, 1, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 1) == 1) {
    bitWrite(ch5_step, 1, 0); // Устанавливаем бит в 0
  }
}
if (enc == 104 && key == 'G') {
  if (bitRead(ch5_step, 0) == 0) {
    bitWrite(ch5_step, 0, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch5_step, 0) == 1) {
    bitWrite(ch5_step, 0, 0); // Устанавливаем бит в 0
  }
}

}

void ch6() {
  char key = customKeypad.getKey();
//ch6---------------------------------------------------------------------------------------------
if (enc == 105 && key == '1') {
  if (bitRead(ch6_step, 15) == 0) {
    bitWrite(ch6_step, 15, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 15) == 1) {
    bitWrite(ch6_step, 15, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == '2') {
  if (bitRead(ch6_step, 14) == 0) {
    bitWrite(ch6_step, 14, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 14) == 1) {
    bitWrite(ch6_step, 14, 0); // Устанавливаем бит в 0
  }
}

if (enc == 105 && key == '3') {
  if (bitRead(ch6_step, 13) == 0) {
    bitWrite(ch6_step, 13, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 13) == 1) {
    bitWrite(ch6_step, 13, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == '4') {
  if (bitRead(ch6_step, 12) == 0) {
    bitWrite(ch6_step, 12, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 12) == 1) {
    bitWrite(ch6_step, 12, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == '5') {
  if (bitRead(ch6_step, 11) == 0) {
    bitWrite(ch6_step, 11, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 11) == 1) {
    bitWrite(ch6_step, 11, 0); // Устанавливаем бит в 0
  }
}

if (enc == 105 && key == '6') {
  if (bitRead(ch6_step, 10) == 0) {
    bitWrite(ch6_step, 10, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 10) == 1) {
    bitWrite(ch6_step, 10, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == '7') {
  if (bitRead(ch6_step, 9) == 0) {
    bitWrite(ch6_step, 9, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 9) == 1) {
    bitWrite(ch6_step, 9, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == '8') {
  if (bitRead(ch6_step, 8) == 0) {
    bitWrite(ch6_step, 8, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 8) == 1) {
    bitWrite(ch6_step, 8, 0); // Устанавливаем бит в 0
  }
}

if (enc == 105 && key == '9') {
  if (bitRead(ch6_step, 7) == 0) {
    bitWrite(ch6_step, 7, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 7) == 1) {
    bitWrite(ch6_step, 7, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == 'A') {
  if (bitRead(ch6_step, 6) == 0) {
    bitWrite(ch6_step, 6, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 6) == 1) {
    bitWrite(ch6_step, 6, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == 'B') {
  if (bitRead(ch6_step, 5) == 0) {
    bitWrite(ch6_step, 5, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 5) == 1) {
    bitWrite(ch6_step, 5, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == 'C') {
  if (bitRead(ch6_step, 4) == 0) {
    bitWrite(ch6_step, 4, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 4) == 1) {
    bitWrite(ch6_step, 4, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == 'D') {
  if (bitRead(ch6_step, 3) == 0) {
    bitWrite(ch6_step, 3, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 3) == 1) {
    bitWrite(ch6_step, 3, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == 'E') {
  if (bitRead(ch6_step, 2) == 0) {
    bitWrite(ch6_step, 2, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 2) == 1) {
    bitWrite(ch6_step, 2, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == 'F') {
  if (bitRead(ch6_step, 1) == 0) {
    bitWrite(ch6_step, 1, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 1) == 1) {
    bitWrite(ch6_step, 1, 0); // Устанавливаем бит в 0
  }
}
if (enc == 105 && key == 'G') {
  if (bitRead(ch6_step, 0) == 0) {
    bitWrite(ch6_step, 0, 1); // Устанавливаем бит в 1
  } else if (bitRead(ch6_step, 0) == 1) {
    bitWrite(ch6_step, 0, 0); // Устанавливаем бит в 0
  }
}

}

void loop() {

 old_clock_in = clock_in;
 old_reset_in = reset_in;
 old_button = button;

 //-----------モード判定----------------

if (enc == 100){
ch1();
}
else if (enc == 101){
  ch2();
}
else if (enc == 102){
  ch3();
}
else if (enc == 103){
  ch4();
}
else if (enc == 104){
  ch5();
}
else if (enc == 105){
  ch6();
}
 //MANUALモード
 if (mode == 0) {
   enc_max = 106;
 }

 //AUTOモード
 else if (mode == 1) {
   enc_max = 11;
 }

 //-----------ロータリーエンコーダ読み取り----------------
 newPosition = myEnc.read();

 if ( newPosition + 2  < oldPosition ) {//左回し
   oldPosition = newPosition;
   enc = enc - 1;
 }

 else if ( newPosition - 2   > oldPosition ) {//右回し
   oldPosition = newPosition;
   enc = enc + 1;
 }

 if (enc <= 0) {
   enc = enc_max;//選択が各モードの最小値に達したら、最大値に戻す
 }
 else if ( enc > enc_max) {
   enc = 1;//選択が各モードの最大値に達したら、最小値に戻す
 }

 if (mode == 0) {//選択中のパターンのビット管理
   enc_bit = 0;
   bitSet(enc_bit, abs(enc % 16 - 16));
   if (abs(enc % 16 - 16) == 16) {
     bitSet(enc_bit, 0);
   }
 }

 //----------BUTTON読み取り----------------

 button = 1 - digitalRead(12); //input_pullupのため"1-"

 if (old_button == 0 && button == 1) {//0→1の時に1回だけpush判定
   button_on = 1;

 }
 else {
   button_on = 0;
 }

 //-------------------MANUALモード-------------------------
if (mode == 0) {
  if (enc == 1){
enc = 99;
  }
  else if (enc == 96){
  enc = 105;
    }
    else if ( enc == enc_max) {
   enc = 97;
    }
  }

 if (mode == 0) {
   if (button_on == 1) {//Нажатие кнопки при выборе любого шага включает или выключает gate.
     /*if ( enc <= 16) {
       ch1_step = ch1_step ^ enc_bit;
     }
     else if ( enc <= 32 && enc > 16) {
       ch2_step = ch2_step ^ enc_bit;
     }
     else if ( enc <= 48 && enc > 32) {
       ch3_step = ch3_step ^ enc_bit;
     }
     else if ( enc <= 64 && enc > 48) {
       ch4_step = ch4_step ^ enc_bit;
     }
     else if ( enc <= 80 && enc > 64) {
       ch5_step = ch5_step ^ enc_bit;
     }
     else if ( enc <= 96 && enc > 80) {
       ch6_step = ch6_step ^ enc_bit;
     }*/




     //AUTO選択
     if ( enc == 97 ) {
       mode = 1;
       change_step();//AUTOモードのパターンに切り替え
     }

     //RESET選択
     else if ( enc == 98 ) {
       step_count = 1;
     }

     //SAVE選択
     else if ( enc == 99 ) {
       save_data();//EEPROMにセーブ保存
       step_count = 1;
     }

     //MUTE設定
     else if ( enc == 100) {
       CH1_mute = !CH1_mute;
     }
     else if ( enc == 101) {
       CH2_mute = !CH2_mute;
     }
     else if ( enc == 102) {
       CH3_mute = !CH3_mute;
     }
     else if ( enc == 103) {
       CH4_mute = !CH4_mute;
     }
     else if ( enc == 104) {
       CH5_mute = !CH5_mute;
     }
     else if ( enc == 105) {
       CH6_mute = !CH6_mute;
     }

   }
 }

 //-------------------AUTOモード-------------------------
 else if (mode == 1) {
   if (button_on == 1) {
     //MANUAL選択
     if ( enc == 1 ) {
       mode = 0;//MANUAL呼び出し
       enc = 97;//AUTOからMANUALに戻ったときに、MANUALを選択状態にする。自由に変えてOK
       //保存データの読み出し
       ch1_step = EEPROM.read(1) * 256 + EEPROM.read(2);
       ch2_step = EEPROM.read(3) * 256 + EEPROM.read(4);
       ch3_step = EEPROM.read(5) * 256 + EEPROM.read(6);
       ch4_step = EEPROM.read(7) * 256 + EEPROM.read(8);
       ch5_step = EEPROM.read(9) * 256 + EEPROM.read(10);
       ch6_step = EEPROM.read(11) * 256 + EEPROM.read(12);
     }


     //genre選択
     else if ( enc == 2 ) {
       genre++;
       if (genre >= 4) {
         genre = 0;
       }
     }

     //fillin選択 0=OFF,1=ON
     else if ( enc == 3 ) {
       fillin = !fillin;
     }

     //repeat選択
     else if ( enc == 4 ) {
       repeat++;
       if (repeat >= 5) {//0=4times,1=8times,2=16times,3=32times,4=eternal
         repeat = 0;
       }
     }

     //switch選択
     else if ( enc == 5) {
       sw++;
       if (sw >= 5) {//0=2,1=4,2=8,3=16,4=eternal
         sw = 0;
       }
     }

     //MUTE設定
     else if ( enc == 6) {
       CH1_mute = !CH1_mute;
     }
     else if ( enc == 7) {
       CH2_mute = !CH2_mute;
     }
     else if ( enc == 8) {
       CH3_mute = !CH3_mute;
     }
     else if ( enc == 9) {
       CH4_mute = !CH4_mute;
     }
     else if ( enc == 10) {
       CH5_mute = !CH5_mute;
     }
     else if ( enc == 11) {
       CH6_mute = !CH6_mute;
     }

   }
 }
 //-------------AUTOモード処理---------------

 switch (repeat) {
   case 0:
     repeat_max = 4;
     break;

   case 1:
     repeat_max = 8;
     break;

   case 2:
     repeat_max = 16;
     break;

   case 3:
     repeat_max = 32;
     break;

   case 4:
     repeat_max = 10000;//ETERNAL
     break;
 }

 switch (sw) {
   case 0:
     sw_max = 2;
     break;

   case 1:
     sw_max = 4;
     break;

   case 2:
     sw_max = 8;
     break;

   case 3:
     sw_max = 16;
     break;

   case 4:
     sw_max = 255;//ETERNAL
     break;
 }
 //--------------外部クロック入力検出,カウント----------------

 clock_in = digitalRead(13);
  if (old_clock_in == 0 && clock_in == 1) {
   step_count++;
 }



 // Reset Input not working for Gate Sequencer yet!
int buttonValue = analogRead(buttonPin);
unsigned long currentTime = millis();

if (buttonValue < threshold) {
  if (!buttonPressed && (currentTime - lastDebounceTime > debounceDelay)) {
    buttonPressed = true;
    lastDebounceTime = currentTime;
    step_count = 1;
    OLED_display();
  }
  } else {
    buttonPressed = false;
  }
 //reset_in = digitalRead(11);
 //if (old_reset_in == 0 && reset_in == 1) {
  // step_count = 1;
  // OLED_display();
 //}

 

 if (step_count >= 17) {
   step_count = 1;
   

   if (mode == 1) {
     repeat_done++;

     if ( fillin == 1 && repeat_done == repeat_max - 1) {
       fillin_step();
     }

     else if (repeat_done >= repeat_max) {
       sw_done++;
       repeat_done = 0;
       change_step();
     }
   }
 }

 if (sw_done >= sw_max) {
   sw_done = 0;
 }


 //--------------シーケンス------------------------------

 CH1_output = bitRead(ch1_step, 16 - step_count );
 CH2_output = bitRead(ch2_step, 16 - step_count );
 CH3_output = bitRead(ch3_step, 16 - step_count );
 CH4_output = bitRead(ch4_step, 16 - step_count );
 CH5_output = bitRead(ch5_step, 16 - step_count );
 CH6_output = bitRead(ch6_step, 16 - step_count );

 //--------------出力----------------------------------
 if (CH1_output == 1 && CH1_mute == 0) {
   digitalWrite(5, clock_in);
   digitalWrite(14, clock_in);
 }
 if (CH2_output == 1 && CH2_mute == 0) {
   digitalWrite(6, clock_in);
   digitalWrite(15, clock_in);
 }
 if (CH3_output == 1 && CH3_mute == 0) {
   digitalWrite(7, clock_in);
   digitalWrite(16, clock_in);
 }
 if (CH4_output == 1 && CH4_mute == 0) {
   digitalWrite(8, clock_in);
   digitalWrite(0, clock_in);
 }
 if (CH5_output == 1 && CH5_mute == 0) {
   digitalWrite(9, clock_in);
   digitalWrite(1, clock_in);
 }
 if (CH6_output == 1 && CH6_mute == 0) {
   digitalWrite(10, clock_in);
   digitalWrite(17, clock_in);
 }



 //--------------OLED出力----------------------------------
 //注意：OLEDの更新はクロック入ったタイミングのみ。Arduinoのビジー状態を避けるため。
 if (old_clock_in == 0 && clock_in == 1){
   OLED_display();
 }

 //  //開発用
 //    Serial.print(repeat_done);
 //    Serial.print(",");
 //    Serial.print(sw_max);
 //    Serial.println("");
}
//--------------OLED出力----------------------------------
void OLED_display() {
 display.clearDisplay();  // ディスプレイをクリア
 display.setTextSize(1);  // 出力する文字の大きさ
 display.setTextColor(WHITE);  // 出力する文字の色

 //CH1表示
 display.setCursor(0, 0);  // 文字の一番端の位置
 display.print("CH1");

 display.setCursor(30, 0);
 if (CH1_mute == 0) {
   if (bitRead(ch1_step, 15 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 14 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 13 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 12 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 11 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 10 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 9 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 8 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 7 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 6 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 5 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 4 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 3 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 2 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 1 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch1_step, 0 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }
 }

 //CH2表示
 display.setCursor(0, 9);  // 文字の一番端の位置
 display.print("CH2");

 display.setCursor(30, 9);
 if (CH2_mute == 0) {
   if (bitRead(ch2_step, 15 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 14 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 13 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 12 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 11 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 10 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 9 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 8 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 7 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 6 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 5 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 4 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 3 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 2 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch2_step, 1 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }
   if (bitRead(ch2_step, 0 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }
 }
 //CH3表示
 display.setCursor(0, 18);  // 文字の一番端の位置
 display.print("CH3");

 display.setCursor(30, 18);
 if (CH3_mute == 0) {
   if (bitRead(ch3_step, 15 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 14 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 13 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 12 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 11 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 10 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 9 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 8 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 7 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 6 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 5 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 4 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 3 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 2 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch3_step, 1 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }
   if (bitRead(ch3_step, 0 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }
 }

 //CH4表示
 display.setCursor(0, 27);  // 文字の一番端の位置
 display.print("CH4");

 display.setCursor(30, 27);
 if (CH4_mute == 0) {
   if (bitRead(ch4_step, 15 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 14 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 13 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 12 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 11 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 10 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 9 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 8 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 7 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 6 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 5 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 4 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 3 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 2 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch4_step, 1 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }
   if (bitRead(ch4_step, 0 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }
 }

 //CH5表示
 display.setCursor(0, 36);  // 文字の一番端の位置
 display.print("CH5");

 display.setCursor(30, 36);
 if (CH5_mute == 0) {
   if (bitRead(ch5_step, 15 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 14 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 13 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 12 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 11 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 10 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 9 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 8 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 7 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 6 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 5 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 4 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 3 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 2 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch5_step, 1 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }
   if (bitRead(ch5_step, 0 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }
 }

 //CH6表示
 display.setCursor(0, 45);  // 文字の一番端の位置
 display.print("CH6");

 display.setCursor(30, 45);
 if (CH6_mute == 0) {
   if (bitRead(ch6_step, 15 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 14 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 13 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 12 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 11 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 10 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 9 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 8 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 7 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 6 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 5 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 4 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 3 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 2 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }

   if (bitRead(ch6_step, 1 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }
   if (bitRead(ch6_step, 0 ) == 1) {
     display.print("*");
   }
   else {
     display.print("_");
   }
 }

 //-------------エンコード表示---------------------------------


 //MANUALモード
 if (mode == 0) {

   //選択中のステップ
   if ( enc <= 16) {
     display.drawRect(enc * 6 + 24, 0 , 6, 8, WHITE);
   }
   else if ( enc <= 32 && enc > 16) {
     display.drawRect((enc - 16) * 6 + 24, 9 , 6, 8, WHITE);
   }
   else if ( enc <= 48 && enc > 32) {
     display.drawRect((enc - 32) * 6 + 24, 18 , 6, 8, WHITE);
   }
   else if ( enc <= 64 && enc > 48) {
     display.drawRect((enc - 48) * 6 + 24, 27 , 6, 8, WHITE);
   }
   else if ( enc <= 80 && enc > 64) {
     display.drawRect((enc - 64) * 6 + 24, 36 , 6, 8, WHITE);
   }
   else if ( enc <= 96 && enc > 80) {
     display.drawRect((enc - 80) * 6 + 24, 45 , 6, 8, WHITE);
   }

   //オプション項目
   display.setCursor(0, 54);  // 文字の一番端の位置
   if (enc == 97) {
     display.setTextColor(BLACK, WHITE); // (BLACK, WHITE)は出力する文字の色を反転
     display.print("MNAL");
   }
   else {
     display.setTextColor(WHITE);
     display.print("MNAL");
   }

   display.setCursor(48, 54);

   if (enc == 98) {
     display.setTextColor(BLACK, WHITE);
     display.print("RESET");
   }
   else {
     display.setTextColor(WHITE);
     display.print("RESET");
   }

   display.setCursor(102, 54);

   if (enc == 99) {
     display.setTextColor(BLACK, WHITE);
     display.print("SAVE");
   }
   else {
     display.setTextColor(WHITE);
     display.print("SAVE");
   }

   if (enc == 100) {
     display.setTextColor(BLACK, WHITE);
     display.setCursor(0, 0);
     display.print("CH1");
   }
   else {
     display.setTextColor(WHITE);
     display.setCursor(0, 0);
     display.print("CH1");
   }

   if (enc == 101) {
     display.setTextColor(BLACK, WHITE);
     display.setCursor(0, 9);
     display.print("CH2");
   }
   else {
     display.setTextColor(WHITE);
     display.setCursor(0, 9);
     display.print("CH2");
   }

   if (enc == 102) {
     display.setTextColor(BLACK, WHITE);
     display.setCursor(0, 18);
     display.print("CH3");
   }
   else {
     display.setTextColor(WHITE);
     display.setCursor(0, 18);
     display.print("CH3");
   }

   if (enc == 103) {
     display.setTextColor(BLACK, WHITE);
     display.setCursor(0, 27);
     display.print("CH4");
   }
   else {
     display.setTextColor(WHITE);
     display.setCursor(0, 27);
     display.print("CH4");
   }

   if (enc == 104) {
     display.setTextColor(BLACK, WHITE);
     display.setCursor(0, 36);
     display.print("CH5");
   }
   else {
     display.setTextColor(WHITE);
     display.setCursor(0, 36);
     display.print("CH5");
   }

   if (enc == 105) {
     display.setTextColor(BLACK, WHITE);
     display.setCursor(0, 45);
     display.print("CH6");
   }
   else {
     display.setTextColor(WHITE);
     display.setCursor(0, 45);
     display.print("CH6");
   }

 }

 //AUTOモード
 display.setCursor(0, 54);  // 文字の一番端の位置
 if (mode == 1) {
   if (enc <= 3 ) {
     if (enc == 1) {
       display.setTextColor(BLACK, WHITE);
       display.print("AUTO");
     }
     else {
       display.setTextColor(WHITE);
       display.print("AUTO");
     }

     display.setTextColor(WHITE);
     display.print("  ");

     if (enc == 2) {
       display.setTextColor(BLACK, WHITE);

       switch (genre) {//プリセットの選択
         case 0:
           display.print("TECHNO");//表示領域の都合上、6文字とする
           break;

         case 1:
           display.print("DUBTCN");
           break;

         case 2:
           display.print("HOUSE ");
           break;

         case 3:
           display.print("HALF ");
           break;
       }

     }
     else {
       display.setTextColor(WHITE);

       switch (genre) {
         case 0:
           display.print("TECHNO");
           break;

         case 1:
           display.print("DUBTCN");
           break;

         case 2:
           display.print("HOUSE ");
           break;
       }
     }

     display.setTextColor(WHITE);
     display.print("  ");

     if (enc == 3) {//Fillinの選択
       display.setTextColor(BLACK, WHITE);
       switch (fillin) {
         case 0:
           display.print("FilIN:N");
           break;

         case 1:
           display.print("FilIN:Y");
           break;
       }
     }
     else {
       display.setTextColor(WHITE);
       switch (fillin) {
         case 0:
           display.print("FilIN:N");
           break;

         case 1:
           display.print("FilIN:Y");
           break;
       }
     }

   }

   else if (enc >= 4 ) {

     if (enc == 4) {//リピートの選択
       display.setCursor(0, 54);  // 文字の一番端の位置
       display.setTextColor(BLACK, WHITE);
       display.print("REP:");
       display.print(repeat_done + 1);
       display.print("/");
       if (repeat <= 3) {
         display.print(repeat_max);
       }
       else if (repeat >= 4) {
         display.print("ET");
       }
     }

     else {
       display.setCursor(0, 54);
       display.setTextColor(WHITE);
       display.print("REP:");
       display.print(repeat_done + 1);
       display.print("/");
       if (repeat <= 3) {
         display.print(repeat_max);
       }
       else if (repeat >= 4) {
         display.print("ET");
       }
     }

     if (enc == 5) {//SWの選択
       display.setTextColor(BLACK, WHITE);
       display.setCursor(70, 54);
       display.print("SW:");
       display.print(sw_done + 1);
       display.print("/");
       if (sw <= 3) {
         display.print(sw_max);
       }
       else if (sw >= 4) {
         display.print("ET");
       }
     }
     else {
       display.setTextColor(WHITE);
       display.setCursor(70, 54);
       display.print("SW:");
       display.print(sw_done + 1);
       display.print("/");
       if (sw <= 3) {
         display.print(sw_max);
       }
       else if (sw >= 4) {
         display.print("ET");
       }
     }

     if (enc == 6) {
       display.setTextColor(BLACK, WHITE);
       display.setCursor(0, 0);
       display.print("CH1");
     }
     else {
       display.setTextColor(WHITE);
       display.setCursor(0, 0);
       display.print("CH1");
     }

     if (enc == 7) {
       display.setTextColor(BLACK, WHITE);
       display.setCursor(0, 9);
       display.print("CH2");
     }
     else {
       display.setTextColor(WHITE);
       display.setCursor(0, 9);
       display.print("CH2");
     }

     if (enc == 8) {
       display.setTextColor(BLACK, WHITE);
       display.setCursor(0, 18);
       display.print("CH3");
     }
     else {
       display.setTextColor(WHITE);
       display.setCursor(0, 18);
       display.print("CH3");
     }

     if (enc == 9) {
       display.setTextColor(BLACK, WHITE);
       display.setCursor(0, 27);
       display.print("CH4");
     }
     else {
       display.setTextColor(WHITE);
       display.setCursor(0, 27);
       display.print("CH4");
     }

     if (enc == 10) {
       display.setTextColor(BLACK, WHITE);
       display.setCursor(0, 36);
       display.print("CH5");
     }
     else {
       display.setTextColor(WHITE);
       display.setCursor(0, 36);
       display.print("CH5");
     }

     if (enc == 11) {
       display.setTextColor(BLACK, WHITE);
       display.setCursor(0, 45);
       display.print("CH6");
     }
     else {
       display.setTextColor(WHITE);
       display.setCursor(0, 45);
       display.print("CH6");
     }

   }
 }


 //出力中のstep
 display.drawRect(step_count * 6 + 24, 0 , 6, 53, WHITE);
 display.setCursor(0, 54);  // 文字の一番端の位置

 display.display();  // ディスプレイへの表示
}

void save_data() {
 EEPROM.update(1, highByte(ch1_step)); //変更データをEEPROMに書き込み
 EEPROM.update(2, lowByte(ch1_step)); //変更データをEEPROMに書き込み
 EEPROM.update(3, highByte(ch2_step)); //変更データをEEPROMに書き込み
 EEPROM.update(4, lowByte(ch2_step)); //変更データをEEPROMに書き込み
 EEPROM.update(5, highByte(ch3_step)); //変更データをEEPROMに書き込み
 EEPROM.update(6, lowByte(ch3_step)); //変更データをEEPROMに書き込み
 EEPROM.update(7, highByte(ch4_step)); //変更データをEEPROMに書き込み
 EEPROM.update(8, lowByte(ch4_step)); //変更データをEEPROMに書き込み
 EEPROM.update(9, highByte(ch5_step)); //変更データをEEPROMに書き込み
 EEPROM.update(10, lowByte(ch5_step)); //変更データをEEPROMに書き込み
 EEPROM.update(11, highByte(ch6_step)); //変更データをEEPROMに書き込み
 EEPROM.update(12, lowByte(ch6_step)); //変更データをEEPROMに書き込み

}

void change_step() {
 //AUTOモードでSTEPを自動変更

 //bank1
 if (genre == 0) {
   if (sw_done >= sw_max) {//SWが指定値になったときのみ、プリセットからランダムにパターンを選択
     change_bnk1 = random(0, 7);//random maxはパターン上限と一致
   }
   ch1_step = pgm_read_word(&(bnk1_ptn[change_bnk1][0]));
   ch2_step = pgm_read_word(&(bnk1_ptn[change_bnk1][1]));
   ch3_step = pgm_read_word(&(bnk1_ptn[change_bnk1][2]));
   ch4_step = pgm_read_word(&(bnk1_ptn[change_bnk1][3]));
   ch5_step = pgm_read_word(&(bnk1_ptn[change_bnk1][4]));
   ch6_step = pgm_read_word(&(bnk1_ptn[change_bnk1][5]));
 }

 //bank2
 if (genre == 1) {
   if (sw_done >= sw_max) {//SWが指定値になったときのみ、プリセットからランダムにパターンを選択
     change_bnk2 = random(0, 4);//random maxはパターン上限と一致
   }
   ch1_step = pgm_read_word(&(bnk2_ptn[change_bnk2][0]));
   ch2_step = pgm_read_word(&(bnk2_ptn[change_bnk2][1]));
   ch3_step = pgm_read_word(&(bnk2_ptn[change_bnk2][2]));
   ch4_step = pgm_read_word(&(bnk2_ptn[change_bnk2][3]));
   ch5_step = pgm_read_word(&(bnk2_ptn[change_bnk2][4]));
   ch6_step = pgm_read_word(&(bnk2_ptn[change_bnk2][5]));
 }

 //bank3
 if (genre == 2) {
   if (sw_done >= sw_max) {//SWが指定値になったときのみ、プリセットからランダムにパターンを選択
     change_bnk3 = random(0, 4);//random maxはパターン上限と一致
   }
   ch1_step = pgm_read_word(&(bnk3_ptn[change_bnk3][0]));
   ch2_step = pgm_read_word(&(bnk3_ptn[change_bnk3][1]));
   ch3_step = pgm_read_word(&(bnk3_ptn[change_bnk3][2]));
   ch4_step = pgm_read_word(&(bnk3_ptn[change_bnk3][3]));
   ch5_step = pgm_read_word(&(bnk3_ptn[change_bnk3][4]));
   ch6_step = pgm_read_word(&(bnk3_ptn[change_bnk3][5]));
 }

 //bank4
 if (genre == 3) {
   if (sw_done >= sw_max) {//SWが指定値になったときのみ、プリセットからランダムにパターンを選択
     change_bnk4 = random(0, 4);//random maxはパターン上限と一致
   }
   ch1_step = pgm_read_word(&(bnk4_ptn[change_bnk4][0]));
   ch2_step = pgm_read_word(&(bnk4_ptn[change_bnk4][1]));
   ch3_step = pgm_read_word(&(bnk4_ptn[change_bnk4][2]));
   ch4_step = pgm_read_word(&(bnk4_ptn[change_bnk4][3]));
   ch5_step = pgm_read_word(&(bnk4_ptn[change_bnk4][4]));
   ch6_step = pgm_read_word(&(bnk4_ptn[change_bnk4][5]));
 } 

}

void fillin_step() {
  const uint16_t* current_bnk_ptn;

  // Determine which bnk_ptn array to use based on the genre
  switch (genre) {
    case 0:
      current_bnk_ptn = bnk1_ptn[change_bnk1];
      break;
    case 1:
      current_bnk_ptn = bnk2_ptn[change_bnk2];
      break;
    case 2:
      current_bnk_ptn = bnk3_ptn[change_bnk3];
      break;
    case 3:
      current_bnk_ptn = bnk4_ptn[change_bnk4];
      break;
    default:
      // Handle invalid genre
      return;
  }

  // Assign values using the determined bnk_ptn array
  ch1_step = pgm_read_word(&(current_bnk_ptn[6]));
  ch2_step = pgm_read_word(&(current_bnk_ptn[7]));
  ch3_step = pgm_read_word(&(current_bnk_ptn[8]));
  ch4_step = pgm_read_word(&(current_bnk_ptn[9]));
  ch5_step = pgm_read_word(&(current_bnk_ptn[10]));
  ch6_step = pgm_read_word(&(current_bnk_ptn[11]));
}