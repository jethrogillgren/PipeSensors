///*
// * --------------------------------------------------------------------------------------------------------------------
// * Listens to sensors in the Pipe, which let us know how players are turning the two lever taps.
// * --------------------------------------------------------------------------------------------------------------------
// * 
// * 
// */
//

#include <Wire.h>
#include <VL6180X.h> //1.2 by pololu


#include <Printers.h>
#include <elapsedMillis.h> //1.0.4 Paul Stroffregen

//TOF sensors
#define RANGE 1
/* List of adresses for each sensor - after reset the address can be configured */
#define address0 0x20
#define address1 0x22

///* These Arduino pins must be wired to the IO0 pin of VL6180x */
int enablePin0 = 2; //D2
int enablePin1 = 3; //D3


///* Create a new instance for each sensor */
VL6180X sensor0;
VL6180X sensor1;

bool sensor0IsOpen = false; //Are we currently registered as closed (false) or open (true)
int s0Changes = 0;
float sensor0OpenDist = 35; // A min value we can read to consider the tap opened.

bool sensor1IsOpen = false; //Are we currently registered as closed (false) or open (true)
int s1Changes = 0;
float sensor1OpenDist = 25; // A min value we can read to consider the tap opened.

//how many readings in a row does it take to convince us we have properly switched position
float readingsToRequireForChange = 3; 

elapsedMillis timeElapsed; //declare global if you don't want it reset every time loop runs


void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  Wire.begin();
  
  //Distance Sensors
  SetSensorI2CAddresses();

  //Trigger an initial send.
  timeElapsed = 60000;
  
  //Serial.print("SETUP OK");
}

void loop() {

  /*Serial.print("\tDistance0: ");
  Serial.print(sensor0.readRangeContinuousMillimeters());
  if (sensor0.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  Serial.print("\tDistance1: ");
  Serial.print(sensor1.readRangeContinuousMillimeters());
  if (sensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

//  Serial.print("\tDistance2: ");
//  Serial.print(sensor2.readRangeContinuousMillimeters());
//  if (sensor2.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  Serial.println();
  return;*/


  //// HANDLE LEVER 0
  uint8_t sensor0CurrentValue = sensor0.readRangeContinuousMillimeters();
  if(sensor0IsOpen && sensor0CurrentValue < sensor0OpenDist )
  {
    s0Changes++;
    if( s0Changes >= readingsToRequireForChange )
    {
      sensor0IsOpen = false;
      SendS0Status();
    }
  } else if ( !sensor0IsOpen && sensor0CurrentValue > sensor0OpenDist )
  {
    s0Changes++;
    if( s0Changes >= readingsToRequireForChange )
    {
      sensor0IsOpen = true;
      SendS0Status();
    }
  } else { //We stayed as we were, open or closed
    s0Changes = 0;
  }


  //// HANDLE LEVER 1
  uint8_t sensor1CurrentValue = sensor1.readRangeContinuousMillimeters();
  if(sensor1IsOpen && sensor1CurrentValue < sensor1OpenDist )
  {
    s1Changes++;
    if( s1Changes >= readingsToRequireForChange )
    {
      sensor1IsOpen = false;
      SendS1Status();
    }
  } else if ( !sensor1IsOpen && sensor1CurrentValue > sensor1OpenDist )
  {
    s1Changes++;
    if( s1Changes >= readingsToRequireForChange )
    {
      sensor1IsOpen = true;
      SendS1Status();
    }
  } else { //We stayed as we were, open or closed
    s1Changes = 0;
  }



  //POLL UPDATES
  if (timeElapsed >= 60000 )
  {
    //Serial.println("Sending 60000ms packet");
    
    SendS0Status();
    SendS1Status();
  }
}

void SendS0Status()
{
  s0Changes = 0;
  timeElapsed = 0; // reset the counter to 0 so the counting starts over...
      
  if(sensor0IsOpen)
    Serial.print("A"); //Command understood by server
  else
    Serial.print("a"); //Command understood by server
}
void SendS1Status()
{
  s1Changes = 0;
  timeElapsed = 0; // reset the counter to 0 so the counting starts over...
      
  if(sensor1IsOpen)
    Serial.print("B"); //Command understood by server
  else
    Serial.print("b"); //Command understood by server
}


void ResetSensors()
{
  //TODO
}


// UTIL FUNCTIONS
void printHex(int num, int precision) {
     char tmp[16];
     char format[128];

     sprintf(format, "0x%%.%dX", precision);

     sprintf(tmp, format, num);
     Serial.print(tmp);
}



void SetSensorI2CAddresses()
{
  // Reset all connected sensors
  pinMode(enablePin0,OUTPUT);
  pinMode(enablePin1,OUTPUT);

  
  digitalWrite(enablePin0, LOW);
  digitalWrite(enablePin1, LOW);

  
  delay(1000);
  
  SetSensorI2CAddress(0, enablePin0, &sensor0, address0 );
  SetSensorI2CAddress(1, enablePin1, &sensor1, address1 );

  delay(1000);
 
  //Serial.println("Sensors ready! Start reading sensors in 3 seconds ...!");
  delay(3000);

}

void SetSensorI2CAddress( int i, int enablePin, VL6180X *sensor, int address )
{

  /*Serial.print("Start Sensor: ");
  Serial.print(i);
  Serial.print(" using pin ");
  Serial.print(enablePin);
  Serial.print(" as I2C Address ");
  Serial.print( address);
  Serial.println();*/
  
  digitalWrite(enablePin, HIGH);
  delay(50);
  sensor->init();
  sensor->configureDefault();
  sensor->setAddress(address);
  //Serial.println(sensor->readReg(0x212),HEX); // read I2C address
  sensor->writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  sensor->writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
  sensor->setTimeout(500);
  sensor->stopContinuous();
  sensor->setScaling(RANGE); // configure range or precision 1, 2 oder 3 mm
  delay(300);
  sensor->startInterleavedContinuous(100);
  delay(100);
}
