
/* PINOUT SETUP */
#define RED_OUT 9
#define BLU_OUT 10
#define GRN_OUT 11
#define PIN_ENC_A 4
#define PIN_ENC_B 2
#define PIN_BUT 3

#define BRIGHT_STEP 3

 int WHITE[3] = {255, 255, 255};
 int OFF[3] = {0, 0, 0};
 int RED[3] = {255, 0, 0};
 int BLUE[3] = {0, 255, 0};
 int GREEN[3] = {0, 0, 255};
int rgb[3] = {RED_OUT, BLU_OUT, GRN_OUT};
int color[3] = {0,0,0};
int max_bright = 50;
int func_mode = 1;
int editing = 0;  // edit mode active
int enc_dir = 0;  // encoder direction - for use outside encoder functions, mostly by color edit mode
static int but_pressed = 0;

void setup(){
  Serial.begin(9600);
  pinMode(RED_OUT,OUTPUT);
  pinMode(BLU_OUT,OUTPUT);
  pinMode(GRN_OUT,OUTPUT);
  pinMode(PIN_BUT, INPUT);
  digitalWrite(PIN_BUT, HIGH);
  pinMode(PIN_ENC_A, INPUT);
  pinMode(PIN_ENC_B, INPUT);
  digitalWrite(PIN_ENC_A, HIGH);
  digitalWrite(PIN_ENC_B, HIGH);
  
  static uint8_t enc_prev_pos   = 0;
  static uint8_t enc_flags      = 0;
  randomSeed(analogRead(0));
  attachInterrupt(0, doEncoder, CHANGE);  // encoder pin on interrupt 0 - pin 2
  attachInterrupt(1, doButton, CHANGE);   // encoder button on interrupt 0 - pin 3
}

//--------------------------------------------------------------------
void light(int col[]){
  for (int i=0; i<3; i++){
    analogWrite(rgb[i], (col[i]*max_bright/100));
  }
}

//--------------------------------------------------------------------
void flash(int flashes[3], int pause=0){
    if (pause){
      light(OFF);
      delay(300);
    }
    light(flashes);
    delay(300);
    light(OFF);
    delay(300);
}  

//--------------------------------------------------------------------
void fade_to(int target[3]){
  int step_val[3] = {0,0,0};
  int num_steps = 200;
  int wait_time = 50;
    for(int j=0; j<num_steps;  j++){
      for (int k=0; k<3; k++){
          color[k] = color[k] + ( (target[k] - color[k])*(j*100/num_steps)/100 );
      }
      light(color);
      delay(wait_time);
      if (but_pressed == 1){    //allows button press to exit current fade
        return;                 //press will be handled by other functions
      }
    }  
}

//--------------------------------------------------------------------
void rand_mode(int num_new_colors){
  switch ( but_pressed ) {
    case 1:
      but_pressed = 0;
      func_mode = 2;
      break;
    case 2:
      but_pressed = 0;
      func_mode = 2; //change to edit once function created
      break;
    default:
      int target1[3] = {0,0,0};    //dummy target
      if (num_new_colors == 3){    //
        for (int i=0; i<3; i++){   //for all three colors
          target1[i] = random(255);  //set each color to a random value
        }    
      } else {                      //only change one random color
        int i = random(3);          //randomly pick a color
        target1[i] = random(255);   //and randomly set that color
      }
      fade_to(target1);             //fade from curren to new
  }
}

//--------------------------------------------------------------------
void single_mode() {
  switch (but_pressed) {
    case 1:
      but_pressed = 0;
      func_mode = 1;
    case 2:
      but_pressed == 0;
      editing = 1;
    return;
  }
  fade_to(color);
}

//--------------------------------------------------------------------
void edit_single() {
  int* flashes[3] = {RED, BLUE, GREEN};
  int add_value = 0;
  for (int i; i<3; i++) {
    flash(flashes[i]);
    while (add_value > -2) {
      enc_dir = 0;
      color[i] += add_value;
      add_value = get_rotary_value(); // get next value
    }
    if (add_value == -2){ //timout occured whiel waiting
      i=3;                //break loop
      break;
    }
  }
  flash(WHITE);
  flash(WHITE);
  editing = 0;
}

//--------------------------------------------------------------------
int get_rotary_value(){
  int count = 0;
  while (enc_dir != 0) {
    count++;
    delay(5);
    if (but_pressed==1){
      but_pressed = 0;
      return -3;
    }
    if (count >40) { //time-out
      flash(WHITE);
      flash(WHITE);
      return -2;
    }
  }
  return enc_dir;
}
//--------------------------------------------------------------------
void doEncoder() {
  if (digitalRead(PIN_ENC_A) == digitalRead(PIN_ENC_B)) { //encoder 'down'/counterclockwise
    //Serial.println("Encoder DOWN");
    if (editing == 1){
      enc_dir = -1; 
      return;
    } 
    switch (func_mode) {
      case 1:  //random mode
        if (max_bright >= 7){
          max_bright -= BRIGHT_STEP;
        } else {
          max_bright = 4;
        }
        break;
      case 2:
        for (int i; i<3; i++){
          color[i] = random(255);
          light(color);
        }
        break;
      default:
       enc_dir = -1; 
       return;
    }
  } else { //encoder 'up'/clockwise
     //Serial.println("Encoder UP");
     if (editing == 1){
      enc_dir = 1; 
      return;
    } 
    switch (func_mode) {
      case 1:  //random mode
        if (max_bright <= 97){          
          max_bright += BRIGHT_STEP;
        } else {
          max_bright = 100;
        }
        break;
      case 2:
        for (int i; i<3; i++){
          color[i] = random(255);
          light(color);
        }
      default:
       enc_dir = 1; 
       return;
    }
  }
  //Serial.println(max_bright);
}

//--------------------------------------------------------------------
void doButton(){
  Serial.println("button pressed");
  but_pressed = 0;
  delay(5);
  if (digitalRead(PIN_BUT)==1){
    int count = 0;    
    but_pressed = 1;
    while (digitalRead(PIN_BUT)==1){
      count++;
      delay(5);      
    }
    if (count >= 20){
        but_pressed = 2;
    }
  } 
}

//--------------------------------------------------------------------
void loop(){
  switch (func_mode) {
    case 1:
      rand_mode(3);
 //     flash(WHITE);
      break;
    case 2:
      if (editing) {
        edit_single();
      } else {
        single_mode();
      }
      break;
    default:
      rand_mode(1);
  }
}



