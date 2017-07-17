const int fusePin = 0;
const int sig = 7;
const int pwm = 5;
unsigned long watchdog = millis();
int timeout = 10000; //watcchdog timeout ms
unsigned long dt = 1; //time step
unsigned long prevTime = millis(); //previous time
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
// sends as output:
//  -1 = no watchdog signal recieved in last "timeout" interval
//  -2 = no 24v power, fuse may be dead
//  x y z= measured value as "x", current duty as "y" and current sp as "z"
//  -3 = incorrect setpoint received: negative or exeeds maxValue

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(5);
  pinMode(pwm, OUTPUT);
  analogWrite(pwm, duty);
  analogReference(EXTERNAL);
  pinMode(A0, INPUT);
  pinMode(A7, INPUT);
  Serial.println("connected");
}

void loop() {
  if(millis() - watchdog >= timeout){
    if(duty > 0){
      duty = 0;
      sp = 0;
    }
    Serial.println("-1");
  }else{
    if(analogRead(fusePin) <= 512){
      if(duty > 0){
        duty = 0;
        sp = 0;
      }
      Serial.println(-2);
    }else{
      dt = millis();
      mv = (analogRead(sig)/1024)*(maxValue/maxVoltage);
      dt -= prevTime;
      prevTime += dt;
      e = sp - mv;
      i += e*dt;
      duty = k*(e + i/Ti + Td*(mv-prev/dt));
      prev = mv;
    }
  }

  // !!!DEBUG!!! //cuts-off power
  if(duty > limit){
    duty = limit;
  }
  
  analogWrite(pwm, duty);
  String out;
  out += mv;
  out += ' ';
  out += duty;
  out += ' ';
  out += sp;
  Serial.println(out);
}

void serialEvent(){
  String in = Serial.readString();
  watchdog = millis();
  Serial.println(in);
  float tmp = in.toFloat();
  if(tmp <= maxValue && tmp >= 0){
    sp = tmp;
  }else{
    sp = 0;
    Serial.println(-3);
  }
}
