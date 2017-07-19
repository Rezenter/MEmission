const int fusePin = 0;
const int sig = 7;
const int pwm = 5;
unsigned long watchdog = millis();
int timeout = 1000; //watcchdog timeout ms
unsigned long dt = 1000; //time step
unsigned long prevTime = micros(); //previous time
unsigned long timer; //transmit timer
//change 5 -> (real max voltage after divider)
float maxVoltage = 5; //voltage@10V
float maxValue = 65; //maxCurrent, mA // 130w@2kV
float sp; //setpoint, mA
float mv; //measured value, mA
float e; //error, mA
float i = 0; //integral contribution
//change coeffs
float k = 5; // proportional coeff
float Ti = 100000000; //integral time
float Td = 0; //derivative time
int duty = 0; //current duty
float prev; //previous measured value, mA
int limit = 100; //debug duty limit
bool spf = false; //wrong sp flag
bool fuse = false; // no power flag
bool wd = false; // watchdog flag
// sends as output:
//  x y z e = measured value as "x", current duty as "y" and current sp as "z", err as "e"
// err codes:
// error summs
//   0 = OK
//  -1 = no watchdog signal recieved in last "timeout" interval
//  -2 = no 24v power, fuse may be dead
//  -4 = incorrect setpoint received: negative or exeeds maxValue

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(5);
  pinMode(pwm, OUTPUT);
  analogWrite(pwm, duty);
  analogReference(EXTERNAL);
  pinMode(A0, INPUT);
  pinMode(A7, INPUT);
  Serial.println("connected");
  timer = millis();
}

void loop() {
  if(millis() - watchdog >= timeout){
    sp = 0;
    wd = true;
  }
  if(analogRead(fusePin) <= 512 && false){//debug
    sp = 0;
    duty = 0;
    fuse = true;
  }else{
    fuse = false;
  }
  if(!fuse){
    dt = micros();
    mv = (analogRead(sig)/1024)*(maxValue/maxVoltage);
    dt -= prevTime;
    prevTime += dt;
    e = sp - mv;
    i += e*dt;
    duty = k*(e + i/Ti + Td*((mv-prev)/dt));
    prev = mv;
  }
  

  // !!!DEBUG!!! //cuts-off power
  if(duty > limit){
    duty = limit;
  }
  
  analogWrite(pwm, duty);
  if(millis() - timer >= 100){
    String out;
    out += mv;
    out += ' ';
    out += duty/10.24;
    out += ' ';
    out += sp;
    out += ' ';
    int err = 0;
    if(wd){
      err -=1;
    }
    if(fuse){
      err -=2;
    }
    if(spf){
      err -=4;
    }
    out += err;
    Serial.println(out);
    timer = millis();
  }
}

void serialEvent(){
  String in = Serial.readString();
  watchdog = millis();
  wd = false;
  float tmp = in.toFloat();
  if(tmp <= maxValue && tmp >= 0){
    sp = tmp;
    spf = false;
  }else{
    sp = 0;
    spf = true;
  }
}
