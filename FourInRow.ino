
#include <Adafruit_NeoPixel.h>
   #define MATRIX_ROWS 8
   #define MATRIX_COLS 8
   #define MATRIX_TOTAL 64
   
    #define PIN 6         // Parameter 1 = number of pixels in strip
                          // Parameter 2 = pin number (most are valid)
                          // Parameter 3 = pixel type flags, add together as needed:
                          // NEO_KHZ800 800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
                          // NEO_KHZ400 400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
                          // NEO_GRB Pixels are wired for GRB bitstream (most NeoPixel products)
                          // NEO_RGB Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(MATRIX_TOTAL, PIN, NEO_GRB + NEO_KHZ800);

 #define jXpin A1
 #define btnPin 8//11
 #define BOARD_PIX_IN_ROW 7
 #define BOARD_PIX_IN_COL 6
 byte board[BOARD_PIX_IN_COL][BOARD_PIX_IN_ROW];
 int dropOnMsec=250;
uint32_t clrPlayer[3]={0,strip.Color(0, 30, 30),strip.Color(30, 0, 30)};
byte player=1;
bool isGameOn=false;
long turnStartTime;
int turnNullTimeMsec=400; //msec from turn start with no btn press
//------marker-----------
int markerOnMsec=350;
int markerPos=0;
bool isMarkerOn=false;
long markerToggleTime;
int jStickChangeMsec=50;
long jStickChangeTime;
void markerOn(){
  lightPixel(7,markerPos,clrPlayer[player]);
  isMarkerOn=true;
}
void markerOff(){
  lightPixel(7,markerPos,strip.Color(0, 0, 0));
  isMarkerOn=false;
}
void markerBlink(){
  if(millis()-markerToggleTime>markerOnMsec){
    if(!isMarkerOn){
      markerOn();
    } else {
      markerOff();
    }
    markerToggleTime=millis();
  }
}
void jStickMoveCheck(){
  int jPos=analogRead(jXpin);
  if(millis()-jStickChangeTime>jStickChangeMsec){
    if(jPos < 200){
      markerOff();
      markerPos++;
      if(markerPos>=BOARD_PIX_IN_ROW){
        markerPos=BOARD_PIX_IN_ROW-1;
      }
      markerOn();
      markerToggleTime=millis();
      jStickChangeTime=millis();
    }
    else if(jPos > 800){
      markerOff();
      markerPos--;
      if(markerPos<=0){
        markerPos=0;
      }
      markerOn();
      markerToggleTime=millis();
      jStickChangeTime=millis();
    }
  }
}
//------------ button--------------
bool isBtnPressed=false;
long btnPressTime;
int debounceMsec=70;
bool btnPressed(){
  bool ret=false;
  if((millis()-btnPressTime>debounceMsec)&&(millis()-turnStartTime>turnNullTimeMsec)){
    if(digitalRead(btnPin)==LOW){
      if(isBtnPressed==false){
        ret=true;
        btnPressTime=millis();
      }
      isBtnPressed=true;
    } else {
      isBtnPressed=false;
    }
  }
  return ret;
}
void shortBlinkPix(int r,int c){
  lightPixel(r,c,clrPlayer[player]);
  delay(dropOnMsec);
  lightPixel(r,c,strip.Color(0, 0, 0));
}
void dropCoin(){
  for(int k=BOARD_PIX_IN_COL-1;k>=0;k--){
    if(board[k][markerPos]==0){
      if((k==0)||(board[k-1][markerPos]!=0)){
        lightPixel(k,markerPos,clrPlayer[player]);
        board[k][markerPos]=player;
        if(gameWinner(player)){
          finishGame();
        }else{
          togglePlayer();
        }
      } else {
        shortBlinkPix(k,markerPos);
      }
    } else {
      break;
    }
  }
}
//----------matrix------------
void slowLightAll(uint32_t clr){
  for(int pixelNum=0;pixelNum<MATRIX_TOTAL;pixelNum++){
    strip.setPixelColor(pixelNum, clr);     
    strip.show();
    delay(20);
  }
}
void slowOffAll(){
  for(int pixelNum=0;pixelNum<MATRIX_TOTAL;pixelNum++){
    strip.setPixelColor(pixelNum, strip.Color(0, 0, 0));     
    strip.show();
    delay(40);
  }
  delay(200);
}
void lightPixel(int row,int col,uint32_t clr){
  int pixelNum=row*MATRIX_COLS+col;
  strip.setPixelColor(pixelNum, clr);     
  strip.show();
}
//-------- game manager---------
void newTurn(){
  markerPos=0;
  markerToggleTime=millis();
  btnPressTime=millis();
  turnStartTime=millis();
  jStickChangeTime=millis();
}
void newGame(){
  for(byte r=0;r<BOARD_PIX_IN_COL;r++){
    for(byte c=0;c<BOARD_PIX_IN_ROW;c++){
      board[r][c]=0;
    }
  }
  btnPressed();
  newTurn();
  player=1;
  isGameOn=true;
}
void togglePlayer(){
  player=(player==1)?2:1;
  newTurn();
}
bool gameWinner(int plyr){
  bool ret=false;
  byte goodInRow=4-1;
  bool foundFlag;
  for(byte r=0;r<BOARD_PIX_IN_COL;r++){
    for(byte c=0;c<BOARD_PIX_IN_ROW;c++){
      // check row
      if(c+goodInRow < BOARD_PIX_IN_ROW){
        foundFlag=true;
        for(byte k=0;k<=goodInRow;k++){
          if(board[r][c+k]!=plyr){
            foundFlag=false;
            break;
          }
        }
        if(foundFlag){
          return true;
        }
      }
      // check col
      if(r+goodInRow < BOARD_PIX_IN_COL){
        foundFlag=true;
        for(byte k=0;k<=goodInRow;k++){
          if(board[r+k][c]!=plyr){
            foundFlag=false;
            break;
          }
        }
        if(foundFlag){
          return true;
        }
      }
      // check main diag
      if((r+goodInRow < BOARD_PIX_IN_COL)&&(c+goodInRow < BOARD_PIX_IN_ROW)){
        foundFlag=true;
        for(byte k=0;k<=goodInRow;k++){
          if(board[r+k][c+k]!=plyr){
            foundFlag=false;
            break;
          }
        }
        if(foundFlag){
          return true;
        }
      }
      // check sec diag
      if((r+goodInRow < BOARD_PIX_IN_COL)&&(c-goodInRow >=0)){
        foundFlag=true;
        for(byte k=0;k<=goodInRow;k++){
          if(board[r+k][c-k]!=plyr){
            foundFlag=false;
            break;
          }
        }
        if(foundFlag){
          return true;
        }
      }
    }
  }
  return false;  
}
void finishGame(){
  slowLightAll(clrPlayer[player]);
  slowOffAll();
  isGameOn=false;
}
//---------------------------------
void setup() {
  //Serial.begin(9600);
  strip.begin();
  pinMode(btnPin,INPUT_PULLUP);
  newGame();
  strip.show();   // initialize all pixels to 'off'    
}

void loop() {
  if(isGameOn){
    markerBlink();
    jStickMoveCheck();
    if(btnPressed()){
      markerOff();
      dropCoin();
    }
  } else {
    if(btnPressed()){
      newGame();
    }
  }
  delay(50);
}
