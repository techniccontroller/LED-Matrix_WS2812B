// LEDmatrix with WS2812B RGB LEDs: complete Program
// You need Libraries from Adafruit to control the WS2812B RGB LEDs
// Video on Youtube: https://youtu.be/ykgVRxlKJrg
//
// modified 04 July 2017
// by techniccontroller

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define PIN 2
#define LEFT 1
#define RIGHT 2
#define EEPROM_MIN_ADDR 0
#define EEPROM_MAX_ADDR 100
#define LEN 100
#define LINE 10
#define RECT 5

// own datatype for matrix movement (snake and spiral)
enum direction {right, left, up, down};



// six predefined colors (red, yellow, purple, orange, green, blue) 
const uint16_t colors[] = {
  matrix.Color(255, 0, 0),
  matrix.Color(200, 200, 0),
  matrix.Color(255, 0, 200),
  matrix.Color(255, 128, 0), 
  matrix.Color(0, 128, 0), 
  matrix.Color(0, 0, 255) };

// width of the led matrix
const int width = 6;
// height of the led matrix
const int height = 10;
// some variables relevant for the snake program
int dx = 0, dy = 1;   
// default tex, when no text is in the eeprom from previous sessions
const String defaultText = "Hello world";
// temp variable for storing the displayed text
String in = defaultText;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(width, height, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);

SoftwareSerial BTserial(3, 4); // RX | TX

void setup() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(colors[0]);
  randomSeed(analogRead(0));
  BTserial.begin(115200);
  Serial.begin(9600);
  
  char chararray[LEN];
  if(eeprom_read_string(10, chararray, LEN)) {
    Serial.println(chararray);
    in = chararray;
  }  
}

void loop() {
  if(BTserial.available() > 0){
    in = BTserial.readString();
    char temparray[in.length()+1];
    in.toCharArray(temparray, in.length()+1);
    if(strstr(temparray, "new") != NULL){
      in = strstr(temparray, "new")+3;
      char temp[in.length()+1];
      in.toCharArray(temp, in.length()+1);
      eeprom_write_string(10, temp);
    }
    else{
      in = defaultText;
      char temp[in.length()+1];
      in.toCharArray(temp, in.length()+1);
      eeprom_write_string(10, temp);
    }
  }
  colorgradient(LINE);
  text(random(6));
  colorgradient(RECT);
  tetris();
  spiral(false);
  spiral(true);
  snake(colors[random(6)]);
  lines(random(6));
}

void lines(int colorID){
  matrix.fillScreen(0);
  for(int i = 0; i < 16; i++){
    matrix.drawPixel(1, i, colors[colorID]);
    matrix.drawPixel(3, i-3, colors[colorID]);
    matrix.drawPixel(5, i-6, colors[colorID]);
    matrix.show();
    delay(250);
  }
  for(int i = -1; i < 30; i++){
    if(i%4 != 0){
      matrix.drawPixel(i+1, 1, colors[(colorID+2)%6]);
      matrix.drawPixel(i-5, 3, colors[(colorID+2)%6]);
      matrix.drawPixel(i-11, 5, colors[(colorID+2)%6]); 
      matrix.drawPixel(i-17, 7, colors[(colorID+2)%6]);
      matrix.drawPixel(i-23, 9, colors[(colorID+2)%6]);
      
    }

    matrix.show();
    delay(250);
  }
}

void spiral(bool empty){
  dx = 0, dy = 1;               // Veränderung
  direction dir1 = down;        // aktuelle Richtung
  int x = 2;
  int y = 4;
  if(!empty)matrix.fillScreen(0);
  int counter1 = 0;
  int countEdge = 1;
  int countCorner = 0;
  bool breiter = true;
  int randNum = random(255);
  for(int i = 0; i < 42; i++){
    if(empty){
      matrix.drawPixel(x, y, 0);
    }
    else{
      matrix.drawPixel(x, y, Wheel((randNum +i*6)%255));
    }
    Serial.print(counter1);
    Serial.print(countEdge);
    Serial.println(countCorner);
    if(countCorner == 2 && breiter){
      countEdge +=1;
      breiter = false;
    }
    if(counter1 >= countEdge){
      dir1 = nextDir(dir1, LEFT);
      counter1 = 0;
      countCorner++;
    }
    if(countCorner >= 4){
      countCorner = 0;
      countEdge += 1;
      breiter = true;
    }
    
    x += dx;
    y += dy;
    
    matrix.show();
    counter1++;
    delay(200);
  }
}

void snake(const uint16_t color){
  dx = 0, dy = 1;               // Change
  direction dir1 = down;        // current direction
  
  int snake1[2][3] = {{ 1,1,1}, // snake coordinates (x and y)
                      { 0,1,2}};
  
  int randomy = random(1,8);    // Random variable for y-direction
  int randomx = random(1,4);    // Random variable for x-direction
  int e = LEFT;                 // next turn
  for(int i = 0; i < 200; i++){ 
      matrix.fillScreen(0);

      // move one step forward
      snake1[0][0] = snake1[0][1];
      snake1[0][1] = snake1[0][2];
      snake1[0][2] = snake1[0][2]+dx;
      snake1[1][0] = snake1[1][1];
      snake1[1][1] = snake1[1][2];
      snake1[1][2] = snake1[1][2]+dy;
      // collision with wall?
      if( (dir1 == down && snake1[1][2] == height-1) || 
          (dir1 == up && snake1[1][2] == 0) ||
          (dir1 == right && snake1[0][2] == width-1) ||
          (dir1 == left && snake1[0][2] == 0)){
          dir1 = nextDir(dir1, e);  
      }
      // Random branching at the side edges
      else if((dir1 == up && snake1[1][2] == randomy && snake1[0][2] == width-1) || (dir1 == down && snake1[1][2] == randomy && snake1[0][2] == 0)){
        dir1 = nextDir(dir1, LEFT);
        e = (e+2)%2+1;
      }
      else if((dir1 == down && snake1[1][2] == randomy && snake1[0][2] == width-1) || (dir1 == up && snake1[1][2] == randomy && snake1[0][2] == 0)){
        dir1 = nextDir(dir1, RIGHT);
        e = (e+2)%2+1;
      }
      else if((dir1 == left && snake1[0][2] == randomx && snake1[1][2] == 0) || (dir1 == right && snake1[0][2] == randomx && snake1[1][2] == height-1)){
        dir1 = nextDir(dir1, LEFT);
        e = (e+2)%2+1;
      }
      else if((dir1 == right && snake1[0][2] == randomx && snake1[1][2] == 0) || (dir1 == left && snake1[0][2] == randomx && snake1[1][2] == height-1)){
        dir1 = nextDir(dir1, RIGHT);
        e = (e+2)%2+1;
      }

      
      for(int i = 0; i < 3; i++){
        // draw the snake
        matrix.drawPixel(snake1[0][i], snake1[1][i], color);
      }
      matrix.show();
      
      if(i%20== 0){
        randomy = random(1,8);
        randomx = random(1,4);
      }
      delay(200);
  }
}

direction nextDir(direction dir, int d){
  // d = 0 -> continue straight on
  // d = 1 -> turn LEFT
  // d = 2 -> turn RIGHT
  direction selection[3];
  switch(dir){
    case right: 
      selection[0] = right;
      selection[1] = up;
      selection[2] = down;
      break;
    case left:
      selection[0] = left;
      selection[1] = down;
      selection[2] = up;
      break;
    case up:
      selection[0] = up;
      selection[1] = left;
      selection[2] = right;
      break;
    case down:
      selection[0] = down;
      selection[1] = right;
      selection[2] = left;
      break; 
  }
  direction next = selection[d];
  
  switch(next){
    case right: 
      dx = 1;
      dy = 0;
      break;
    case left:
      dx = -1;
      dy = 0;
      break;
    case up:
      dx = 0;
      dy = -1;
      break;
    case down:
      dx = 0;
      dy = 1;
      break; 
  }
  return next;
}

void colorgradient(int shape){
  int wert = 0;
  for(int k = 0; k < 500; k++){
    matrix.fillScreen(0); // clear screen
    for(int i = 0; i < shape; i++){
      if(shape == LINE) matrix.drawLine(0,i,5,i, Wheel(((i * 256 / 20) + wert) & 255));
      else if(shape == RECT) matrix.drawRect(2-i,4-i,2+i*2,2+i*2, Wheel(((i * 256 / 20) + wert) & 255));
    }
    matrix.show();        // show the screen
    delay(50);
    wert--;
    if(wert <= 0) wert = 255;
  }
}

void tetris(){
  boolean figures[9][3][3]={{ {0,0,0},
                              {0,0,0},
                              {0,0,0}},
                            { {1,0,0},
                              {1,0,0},
                              {1,0,0}},
                            { {0,0,0},
                              {1,0,0},
                              {1,0,0}},
                            { {0,0,0},
                              {1,1,0},
                              {1,0,0}},
                            { {0,0,0},
                              {0,0,0},
                              {1,1,0}},
                            { {0,0,0},
                              {1,1,0},
                              {1,1,0}},
                            { {0,0,0},
                              {0,0,0},
                              {1,1,1}},
                            { {0,0,0},
                              {1,1,1},
                              {1,0,0}},
                            { {0,0,0},
                              {0,0,1},
                              {1,1,1}}};
  int screen[height+3][width] = {{0,0,0,0,0,0},
                              {0,0,0,0,0,0},
                              {0,0,0,0,0,0},
                              {0,0,0,0,0,0},
                              {0,0,0,0,0,0},
                              {0,0,0,0,0,0},
                              {0,0,0,0,0,0},
                              {0,0,0,0,0,0},
                              {0,0,0,0,0,0},
                              {0,0,0,0,0,0},
                              {0,0,0,0,0,0},
                              {0,0,0,0,0,0},
                              {0,0,0,0,0,0}};
  boolean tomove[20];
  int counterTime = 0;
  int counterID = 0;
  int randx = 0;
  int randTile = 0;
  boolean finish = true;
  boolean lose = false;

  while(!lose){
    matrix.fillScreen(0); // clear screen
    counterTime++;
    if(finish){
      lose = false;
      for(int s = 0; s < 6; s++){
        if(screen[3][s] != 0) lose = true;
      }
      if(lose){
        counterID = 0;
        for(int i = 0; i < 6; i++){
          matrix.fillScreen(0);
          for(int s = 0; s < width; s++){
            for(int z = 0; z < height+3; z++){
              if(screen[z][s] != 0){
                matrix.drawPixel(s+i+1,z-3,colors[(screen[z][s]%6)]); 
              }
              else{
                matrix.drawPixel(s+i+1,z-3,0);
              }
            }  
          }
          matrix.show();
          delay(130);
        }
        for(int s = 0; s < width; s++){
          for(int z = 0; z < height+3; z++){
            screen[z][s] = 0;
          }
        }
      }
      counterID++;
      randTile = random(1,9);
      randx = random(0,6 - (randTile/3));
      for(int s1 = 0, s2 = randx; s1 <= randTile/3; s1++, s2++){
        for(int z = 0; z < 3; z++){
          if(figures[randTile][z][s1]) screen[z][s2] = counterID;
        }
      }
    }
   
    for(int i = 0; i < 20; i++) tomove[i]=true;
    for(int s = 0; s < width; s++){
      for(int z = 0; z < height+3; z++){
        if(screen[z][s] != 0){
          if(z == height+2 || (screen[z+1][s] != 0 && screen[z+1][s] != screen[z][s])){
            tomove[screen[z][s]] = false;
          }
          matrix.drawPixel(s,z-3,colors[(screen[z][s]%6)]); 
        }
      }  
    }
    finish = true;
    for(int s = width-1; s >= 0; s--){
      for(int z = height+1; z >= 0; z--){
        if(screen[z][s] != 0 && tomove[screen[z][s]]){
          screen[z+1][s] = screen[z][s];
          screen[z][s] = 0;
          finish = false;
        }
      }
    } 
    matrix.show();        // show the screen
    delay(300);
  }
}

void text(int colorbegin){
  int x    = matrix.width();
  int pass = 0;
  while( pass < 3){
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    int len = in.length();
    matrix.print(in);
    if(--x < -len*6) {
      x = matrix.width();
      pass++;
      matrix.setTextColor(colors[(colorbegin+pass)%6]);
    }
    matrix.show();
    delay(100);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return matrix.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return matrix.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return matrix.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


//Write a sequence of bytes starting at the specified address.
//Returns True if the entire array has been written,
//Returns False if start or end address is not between the minimum and maximum allowed range.
//If False was returned, nothing was written 
boolean eeprom_write_bytes(int startAddr, const byte* array, int numBytes) {
  int i;
 
  if (!eeprom_is_addr_ok(startAddr) || !eeprom_is_addr_ok(startAddr + numBytes)) return false;
 
  for (i = 0; i < numBytes; i++) {
    EEPROM.write(startAddr + i, array[i]);
  }  return true;
}
 
//Writes an int value to the specified address. 
boolean eeprom_write_int(int addr, int value) {
  byte *ptr;
 
  ptr = (byte*)&value;
  return eeprom_write_bytes(addr, ptr, sizeof(value));
}
 
//Reads an integer value at the specified address
boolean eeprom_read_int(int addr, int* value) {
  return eeprom_read_bytes(addr, (byte*)value, sizeof(int));
}
 


//Reads the specified number of bytes at the specified address
boolean eeprom_read_bytes(int startAddr, byte array[], int numBytes) {
  int i;
 
  if (!eeprom_is_addr_ok(startAddr) || !eeprom_is_addr_ok(startAddr + numBytes)) return false;
 
  for (i = 0; i < numBytes; i++) {
    array[i] = EEPROM.read(startAddr + i);
  } return true;
}
 
//Returns True if the specified address is between the minimum and the maximum allowed range.
//Invoked by other superordinate functions to avoid errors.
boolean eeprom_is_addr_ok(int addr) {
  return ((addr >= EEPROM_MIN_ADDR) && (addr <= EEPROM_MAX_ADDR));
}
 
//Write a string, starting at the specified address
boolean eeprom_write_string(int addr, const char* string) {
  int numBytes;
  numBytes = strlen(string) + 1;
 
  return eeprom_write_bytes(addr, (const byte*)string, numBytes);
}
 
//Reads a string from the specified address
boolean eeprom_read_string(int addr, char* buffer, int bufSize) {
  byte ch;
  int bytesRead;
 
  if (!eeprom_is_addr_ok(addr)) return false;
  if (bufSize == 0) return false;
 
  if (bufSize == 1) {
    buffer[0] = 0;
    return true;
  }
 
  bytesRead = 0;
  ch = EEPROM.read(addr + bytesRead);
  buffer[bytesRead] = ch;
  bytesRead++;
 
  while ((ch != 0x00) && (bytesRead < bufSize) && ((addr + bytesRead) <= EEPROM_MAX_ADDR)) {
    ch = EEPROM.read(addr + bytesRead);
    buffer[bytesRead] = ch;
    bytesRead++;
  }
 
  if ((ch != 0x00) && (bytesRead >= 1)) buffer[bytesRead - 1] = 0;
 
  return true;
}
