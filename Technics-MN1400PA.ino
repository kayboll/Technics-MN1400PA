//Technics Direct Drive Automatic Turntable System SL-1600MK2 SL-1600MK2A SL-1610MK2 SL-1610MK2A
//Replacement of the MN1400PA IC401
//Arduino Nano
//https://github.com/kayboll/Technics-MN1400PA
//version 1.0

//###outputs###
#define LED_DRIVE0 11//DO0 (pin 23)
#define LED_DRIVE1 10//DO1 (pin 24)
#define LED_DRIVE2 9//DO2  (pin 25)
#define MOTOR_DRIVE0 3//E00 (pin 15)
#define MOTOR_DRIVE1 4//E01 (pin 16)
#define MOTOR_DRIVE2 5//E02 (pin 17)
#define REPEAT_LED  13 //CO9 (pin 2)
#define TURNTABLE_DRIVE 8 //CO8 (pin 3)
#define MATRIX_OUT_CUEINGKEY_REPEAT 6//CO6 (pin 5)
#define MATRIX_OUT_SS_CUEING 7//CO5 (pin 6)

//###inputs###
#define DISK_SIZE 14//SNS1 (pin 22)
#define MATRIX_IN_SS_REPEAT 15//A10 (pin 10)
#define MATRIX_IN_CUEING 16//AI1 (pin 9)
#define ARM_LIMIT0 17//AI2 (pin 8)
#define ARM_LIMIT1 18//AI3 (pin 7)
#define ARM_REST 19//BI0 (pin 14)
#define ARM_END1 2//BI1 (pin 13)
#define ARM_END2 12//BI2 (pin 12)
#define TURNTABLE_ROTATION A6 //BI3 (pin 11)

#other
//Reset: pin 20
//VDD +5V pin 27
//VSS GND pin 1

//###defines####
#define MATRIX_REPEAT 1
#define MATRIX_START_STOP 2
#define MATRIX_CUEING_KEY 3
#define MATRIX_CUEING_LEVER 4

#define STATE_REST 0
#define STATE_CUEING_UP 1
#define STATE_MOVE_START 2
#define STATE_CUEING_DOWN 3
#define STATE_PLAY 4
#define STATE_MOVE_REST 5
#define STATE_RETURN_TO_START 7
#define STATE_CUEING_DOWN_REVERSE 8
#define STATE_PAUSE 9

#define DISK_NONE 0
#define DISK_30 1
#define DISK_25 2
#define DISK_17 3


void setup() {
  Serial.begin(9600);
  
  pinMode(LED_DRIVE0, OUTPUT);
  pinMode(LED_DRIVE1, OUTPUT);
  pinMode(LED_DRIVE2, OUTPUT);
  pinMode(MOTOR_DRIVE0, OUTPUT);
  pinMode(MOTOR_DRIVE1, OUTPUT);
  pinMode(MOTOR_DRIVE2, OUTPUT);
  pinMode(REPEAT_LED, OUTPUT);
  pinMode(TURNTABLE_DRIVE, OUTPUT);
  pinMode(MATRIX_OUT_CUEINGKEY_REPEAT, OUTPUT);
  pinMode(MATRIX_OUT_SS_CUEING, OUTPUT);

  pinMode(DISK_SIZE, INPUT);
  pinMode(MATRIX_IN_SS_REPEAT, INPUT);
  pinMode(MATRIX_IN_CUEING, INPUT);
  pinMode(ARM_LIMIT0, INPUT);
  pinMode(ARM_LIMIT1, INPUT);
  pinMode(ARM_REST, INPUT);
  pinMode(ARM_END1, INPUT);
  pinMode(ARM_END2, INPUT);
  pinMode(TURNTABLE_ROTATION, INPUT);
  
  digitalWrite(LED_DRIVE0, LOW);
  digitalWrite(LED_DRIVE1, LOW);
  digitalWrite(LED_DRIVE2, LOW);
  digitalWrite(MOTOR_DRIVE0, LOW);
  digitalWrite(MOTOR_DRIVE1, LOW);
  digitalWrite(MOTOR_DRIVE2, LOW);
  digitalWrite(REPEAT_LED, LOW);
  digitalWrite(TURNTABLE_DRIVE, LOW);
  digitalWrite(MATRIX_OUT_CUEINGKEY_REPEAT, LOW);
  digitalWrite(MATRIX_OUT_SS_CUEING, LOW);

  digitalWrite(DISK_SIZE, HIGH);

  //interrupt for disksize messurement
  PCICR |= B00000010;
  PCMSK1 |= B00000001;
}

int tableTurning()
{
  int x=analogRead(TURNTABLE_ROTATION) > 512; //to few digital pins so i have to use an analog pin
  return digitalRead(TURNTABLE_DRIVE) && analogRead(TURNTABLE_ROTATION) > 512;
}

int lastkey= 0;
int matrix()
{
  digitalWrite(MATRIX_OUT_CUEINGKEY_REPEAT, HIGH);
  digitalWrite(MATRIX_OUT_SS_CUEING, LOW);
  delay(1);
  if(digitalRead(MATRIX_IN_SS_REPEAT))
  {
    Serial.println("button repeat");
    return MATRIX_REPEAT;
  }
  if(digitalRead(MATRIX_IN_CUEING))
  {
    Serial.println("button cueing key");
    return MATRIX_CUEING_KEY;
  }
  digitalWrite(MATRIX_OUT_CUEINGKEY_REPEAT, LOW);
  digitalWrite(MATRIX_OUT_SS_CUEING, HIGH);
  delay(1);
  if(digitalRead(MATRIX_IN_SS_REPEAT))
  {
    Serial.println("button start stop");
    return MATRIX_START_STOP;
  }
  if(digitalRead(MATRIX_IN_CUEING))
  {
    Serial.println("button cueing lever");
    return MATRIX_CUEING_LEVER;
  }
}

int getCueingLever()
{
  digitalWrite(MATRIX_OUT_CUEINGKEY_REPEAT, LOW);
  digitalWrite(MATRIX_OUT_SS_CUEING, HIGH);

  return digitalRead(MATRIX_IN_CUEING);
}

int cueingLeverDebounce=0;
int changedCueingLever()
{
  if(getCueingLever())
  {
    if(cueingLeverDebounce==0)
    {
      Serial.println("cueing Lever up");
      cueingLeverDebounce=1;
      return 1;
    }

  }else
  {
    cueingLeverDebounce=0;
  }
  return 0;
}


int cueingKeyDebounce=0;
int CueingKey()
{
  digitalWrite(MATRIX_OUT_CUEINGKEY_REPEAT, HIGH);
  digitalWrite(MATRIX_OUT_SS_CUEING, LOW);

  if(digitalRead(MATRIX_IN_CUEING))
  {
    if(cueingKeyDebounce==0)
    {
      Serial.println("button cueing key");
      cueingKeyDebounce=1;
      return 1;
    }

  }else
  {
    cueingKeyDebounce=0;
  }
  return 0;
}

int repeatDebounce=0;
int repeatKey()
{
  digitalWrite(MATRIX_OUT_CUEINGKEY_REPEAT, HIGH);
  digitalWrite(MATRIX_OUT_SS_CUEING, LOW);

  if(digitalRead(MATRIX_IN_SS_REPEAT))
  {
    if(repeatDebounce==0)
    {
      Serial.println("button repeat");
      repeatDebounce=1;
      return 1;
    }

  }else
  {
    repeatDebounce=0;
  }
  return 0;
}


int StartStopKeyDebounce=0;
int StartStopKey()
{
  digitalWrite(MATRIX_OUT_CUEINGKEY_REPEAT, LOW);
  digitalWrite(MATRIX_OUT_SS_CUEING, HIGH);

  if(digitalRead(MATRIX_IN_SS_REPEAT))
  {
    if(StartStopKeyDebounce==0)
    {
      Serial.println("button cueing key");
      StartStopKeyDebounce=1;
      return 1;
    }

  }else
  {
    StartStopKeyDebounce=0;
  }
  return 0;
}

int End1Debounce=0;
int end1()
{
  if(digitalRead(ARM_END1))
  {
    if(End1Debounce==0)
    {
      End1Debounce=1;
      return 1;
    }

  }else
  {
    End1Debounce=0;
  }
  return 0;
}

volatile int count = 0;
int getDiskSize()
{  
   int disksize = DISK_30;
   for(int i = 0;i<5000;i++)
   {
      count = 0;
      digitalWrite(LED_DRIVE0, HIGH);//outer
      delayMicroseconds(20);
      if(count > 0 && disksize == DISK_30)
      { 
        disksize=DISK_25;
      }
      digitalWrite(LED_DRIVE0, LOW); 
      delayMicroseconds(1);
      count = 0; 

      digitalWrite(LED_DRIVE1, HIGH);//mid
      delayMicroseconds(20);
      if(count > 0 && disksize == DISK_25)
      {
        disksize=DISK_17;
      }
      digitalWrite(LED_DRIVE1, LOW);
      delayMicroseconds(30);
      count = 0;

      digitalWrite(LED_DRIVE2, HIGH); //inner
      delayMicroseconds(20);
      if(count > 0 && disksize == DISK_17)
      {
        disksize=DISK_NONE;
      }
      digitalWrite(LED_DRIVE2, LOW);
      delayMicroseconds(1);
  
   
   }
   if(!tableTurning())
   {
      disksize=DISK_NONE;
   }
   return disksize;
}

void armMotorStop()
{
  digitalWrite(MOTOR_DRIVE0, LOW);
  digitalWrite(MOTOR_DRIVE1, LOW);
  digitalWrite(MOTOR_DRIVE2, LOW);
}

void armMotorForward()
{
  digitalWrite(MOTOR_DRIVE0, HIGH);
  digitalWrite(MOTOR_DRIVE1, LOW);
  digitalWrite(MOTOR_DRIVE2, LOW);
}

void armMotorReverse()
{
  digitalWrite(MOTOR_DRIVE0, LOW);
  digitalWrite(MOTOR_DRIVE1, HIGH);
  digitalWrite(MOTOR_DRIVE2, LOW);
}

void armMotorSlowForward()
{
  digitalWrite(MOTOR_DRIVE0, HIGH);
  digitalWrite(MOTOR_DRIVE1, LOW);
  digitalWrite(MOTOR_DRIVE2, HIGH);
}

void armMotorSlowReverse()
{
  digitalWrite(MOTOR_DRIVE0, LOW);
  digitalWrite(MOTOR_DRIVE1, HIGH);
  digitalWrite(MOTOR_DRIVE2, HIGH);
}


int armUp = 0;
int repeat = 0;
int state =STATE_REST;
int autoMove = 0;
int startPosition = 0;
int diskSize=DISK_NONE;
int parkDelay = 0;
int lastState = STATE_REST;
int startDelay = 0;
// the loop function runs over and over again forever
void loop() {
  
  switch(state)
  {
    case STATE_CUEING_UP:
      if(!digitalRead(ARM_LIMIT1))
        armMotorForward();
      else
      {
        armMotorStop();
        state = STATE_PAUSE;
      }
      break;
    case STATE_MOVE_START:
      if(!digitalRead(TURNTABLE_DRIVE))
      {
        digitalWrite(TURNTABLE_DRIVE, HIGH); //drive on
        startDelay = 0;
        delay(10);
      }
      if(!tableTurning() || startDelay < 100)
      {
        if(tableTurning()) startDelay++;
        else startDelay = 0;
        
        if(StartStopKey())
        {
          state = STATE_MOVE_REST;
        }   
        Serial.println(startDelay);
        delay(1);
        break;
      }
      if(diskSize == DISK_NONE) {
        diskSize=getDiskSize();
        if(diskSize == DISK_NONE) {
          digitalWrite(TURNTABLE_DRIVE, LOW); //drive off
          state = STATE_REST;
          Serial.println("no disk!");
          break;
        }
      }
      
      if(digitalRead(ARM_REST) && end1()) startPosition++;
      if(startPosition != diskSize)
      {
        armMotorForward();
      }else
      {
        state=STATE_CUEING_DOWN;
      }
      break;
    case STATE_RETURN_TO_START:
      if(diskSize == DISK_NONE) {
        diskSize=getDiskSize();
        if(diskSize == DISK_NONE)
        {
          state = STATE_MOVE_REST;
          break;
        }
      }
      if(digitalRead(ARM_END2)) 
      {
        startPosition = (DISK_17+1)-diskSize;
        armMotorReverse();
        break;
      }
      
      if(digitalRead(ARM_REST) && end1()) startPosition--;
      if(startPosition != 0)
      {
        armMotorReverse();
      }else
      {
        if(digitalRead(ARM_END1))
          armMotorSlowReverse();
        else
          state=STATE_CUEING_DOWN_REVERSE;
      }
      break;
    case STATE_CUEING_DOWN:
      if(digitalRead(ARM_REST)) //not in rest position
      {
        if(diskSize == DISK_NONE)
          diskSize=getDiskSize();
        if(diskSize == DISK_NONE)
        {
          break;
        }
      }
      if(digitalRead(ARM_LIMIT0))
        armMotorSlowReverse();
      else
      {
        armMotorStop();
        if(!digitalRead(ARM_REST))
          state = STATE_REST;
        else
          state = STATE_PLAY;
      }
      break;
    case STATE_CUEING_DOWN_REVERSE:
      if(digitalRead(ARM_REST)) //not in rest position
      {
        if(diskSize == DISK_NONE)
          diskSize=getDiskSize();
        if(diskSize == DISK_NONE)
        {
          break;
        }
      }
      if(digitalRead(ARM_LIMIT1))
        armMotorSlowForward();
      else
      {
        armMotorStop();
        if(!digitalRead(ARM_REST))
          state = STATE_REST;
        else
          state = STATE_PLAY;
      }
      break;
    case STATE_PLAY: 
      if(!digitalRead(ARM_REST))
      {
          state = STATE_REST;
      }
      if(armUp) state = STATE_CUEING_UP;

      if(digitalRead(ARM_END2) )
      {
        if(diskSize != DISK_17 || digitalRead(ARM_END1) == true)
        {
          if(repeat)
          {
            state = STATE_RETURN_TO_START;
            break;
          }else
          {
            state = STATE_MOVE_REST;
            Serial.println("end");
            break;
          }
          
        }
      }
      if(StartStopKey())
      {
        state = STATE_MOVE_REST;
      }
      /*
      if(diskSize == DISK_NONE)
        diskSize=getDiskSize();
      if(diskSize == DISK_NONE)
      {
        state = STATE_MOVE_REST;
        break;
      }
      */
      break;
    case STATE_MOVE_REST:
      if(digitalRead(ARM_REST))
      {
        autoMove = 1;
        armMotorReverse();
        parkDelay = 1;
      }else
      {
        if(parkDelay)
        {
          armMotorReverse();
          delay(400);
          parkDelay=0;
        }
        if(digitalRead(ARM_LIMIT1))
        {
          autoMove = 1;
          armMotorSlowForward();
        }else
        {
          armMotorStop();
          autoMove = 0;
          state = STATE_REST;
        }
      }
      break;
    case STATE_PAUSE:
      if(!armUp && digitalRead(ARM_LIMIT1)) state = STATE_CUEING_DOWN;
      if(digitalRead(ARM_REST))// low when arm in rest
      {
        if(!digitalRead(TURNTABLE_DRIVE))
        {
          digitalWrite(TURNTABLE_DRIVE, HIGH); //drive on
          diskSize=getDiskSize();
        }
      }else
      {
        digitalWrite(TURNTABLE_DRIVE, LOW); //drive off
      }
      
      break;
    case STATE_REST:
    default:
        diskSize = DISK_NONE;
        if(digitalRead(ARM_REST) && !autoMove && !digitalRead(ARM_LIMIT0))
        {
          state = STATE_PLAY;
        }else
        {
          if(armUp) state = STATE_CUEING_UP;
          else if(digitalRead(ARM_LIMIT1)) state = STATE_CUEING_DOWN;
        }
        if(StartStopKey())
        {
          state = STATE_MOVE_START;
        }
        if(digitalRead(ARM_REST))// low when arm in rest
        {
          digitalWrite(TURNTABLE_DRIVE, HIGH); //drive on
          diskSize=getDiskSize();
        }else
        {
          digitalWrite(TURNTABLE_DRIVE, LOW); //drive off
        }
        startPosition = 0;
      break;
    
  }

  if(CueingKey())
    armUp = !armUp;
  else
  {
    if(state != STATE_CUEING_DOWN && state != STATE_CUEING_UP)
    {
      if(getCueingLever() != armUp)
      {
        armUp = getCueingLever();
      } 
    }
  }

  if(repeatKey())
  {
    repeat = !repeat;
    repeat ? digitalWrite(REPEAT_LED, HIGH):digitalWrite(REPEAT_LED, LOW); 
    delay(200);
  }


  if(state != lastState)
  {
    lastState = state;
    switch(state)
    {
    case STATE_CUEING_UP:
      Serial.println("#state: up");
      break;
    case STATE_MOVE_START:
      Serial.println("#state: move start");
      break;
    case STATE_RETURN_TO_START:
      Serial.println("#state: return to start");
      break;
    case STATE_CUEING_DOWN:
      Serial.println("#state: down");
      break;
    case STATE_CUEING_DOWN_REVERSE:
      Serial.println("#state: down reverse");
      break;
    case STATE_PLAY:
      Serial.println("#state: play");  
      break;
    case STATE_MOVE_REST:
      Serial.println("#state: move rest");
      break;
    case STATE_PAUSE:
      Serial.println("#state: pause");
      break;
    case STATE_REST:
      Serial.println("#state: rest");
      break;
    default:
      Serial.println("#state: unknown");
      break;
    }
  }
}

ISR (PCINT1_vect) 
{
  if(digitalRead(DISK_SIZE))
  {
    count++;
  }
}
