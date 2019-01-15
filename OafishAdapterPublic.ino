/*
Use Instructions:

Plug in the controller before plugging in from console. Reset could be added but it's not worth the effort and shortcut slot.
You may move the analog stick around your desired deadzone. If you would not like a custom deadzone you may press A at this time.
Once melee has initiated, press B in order to gain access to the analog stick.

Shortcuts:
Y + START		(.2 sec)	:	Recalibrate

Y + Z + START	(.2 sec)	:	Calibrate notches
Push the analog stick to the E, SE, S, SW, W, and N gates, pressing B at each gate.
You may press A at any time to cancel.

D-Down			(.5 sec)	:	Enable/Disable all mods


Modifications done by Oafish
Fixes:
Map: Conflicting function
AnglesFixed: <= instead of < fixed edge cases of exact angles.  Also re-calibration() for new vars and new else statements allowing the g.el and eh replacement fix.
Ang: Fixed math through custom modulo function nmod(n, base) -- I know it's not called a base
Dolphin Mode Timing: Activation also signalled by jump or cursor going upward.
g.el and eh: Fixed math so that s-e overlapping deg origin isn't the only case covered.  You can not calibrate notches upside down!
Arc: Allows angFix to work.
Various other math fixes.

I would add a deadzone and shift values to notches, but it would be more work than it'd be worth.  Factory springs and notches are pretty good already plus center limits range.  Optional deadzone in conjunction with startZero() will work be better with a wider range of notches and sensors.

Additions:
Notch-Setter Function: Press Y+Z+START for 1/5 of a second then push the control stick to the E, SE, S, SW, W, and N gates in order, pressing B at each gate.  This will orient the controller of anglesFixed is set to work.
Serial-Debugging Prints: 
backdash(ooc)ms(): Backdash timed by ms instead of cycles
Lots of other stuff.

If you'd like to see all of the changes you may compare with v3.cpp from the linked repository ( https://github.com/OtaK/GCCArduino ).
*/

float n__notch_x_value = 0.0000;
float n__notch_y_value = 0.9875;

float e__notch_x_value = 0.9875;
float e__notch_y_value = -0.0000;
 
float s__notch_x_value = -0.0000;
float s__notch_y_value = -0.9875;
 
float w__notch_x_value = -0.9875;
float w__notch_y_value = -0.0000;

float sw_notch_x_value = -0.7000;
float sw_notch_y_value = -0.7000;
 
float se_notch_x_value = 0.7000;
float se_notch_y_value = -0.7000;

#include "Nintendo.h"
CGamecubeController controller(3); //sets RX0 on arduino to read data from controller
CGamecubeConsole console(4);       //sets TX1 on arduino to write data to console
Gamecube_Report_t gcc;             //structure for controller state
Gamecube_Data_t data;
struct{int8_t ax, ay, cx, cy; uint8_t l, r;}ini;
bool shield, dolphin=0, off=0, cal=1, button, calN = 0, susIn = 0, pressed = false, start = false, startZone = false, initD = true, standardize = true, offPressed = false;
struct{float n, e, s, w, se, sw;}g;
struct{uint8_t db:5, cr:5;}buf;
struct{bool u, d, l, r;}perf;
struct{uint8_t l, r;}ls;
int8_t ax, ay, cx, cy;
float r, deg, cr, ref;
uint8_t cyclesDB=4;    					//Number of buffer cycles for dbs +1 (default 3)
uint8_t cyclesCDB=4;  					//Number of buffer cycles for dboocs +1 (default 3)
struct{uint8_t DB:5, CDB:5;}cycleRef;
uint16_t cyclesDBms=14; 				//ms buffer for dbs +1 (default 3)
uint16_t cyclesCDBms=14;  				//ms buffer for dboocs +1 (default 3)
struct{uint64_t db:64, cr:64;}bufms;
uint16_t mode;
uint32_t n, c, cN, cD;
uint8_t dMult = 16;
uint8_t index = 0;
uint8_t perfAng = 18; //Do min of 18
uint8_t zoneX = 0, zoneY = 0;
uint8_t hzRatio = 2.666;
uint64_t startTime = 0, standardTime = 3;

float toSetX[] = {e__notch_x_value, se_notch_x_value, s__notch_x_value, sw_notch_x_value, w__notch_x_value, n__notch_x_value};
float toSetY[] = {e__notch_y_value, se_notch_y_value, s__notch_y_value, sw_notch_y_value, w__notch_y_value, n__notch_y_value};

void mods(){		//to remove mods delete any lines that you do not want here
  initZone();		//Takes initial position and creates a custom rectangle deadzone based on movement before pressing B. Might mess with single sensor precise values. Cross deadzone would not be good for Melee and have difficult scaling. Might need to re-calibrate notches if using.
  anglesfixed();	//reallocates angles properly based on the given cardinal notches
  perfectangles();	//gives steepest/shallowest angles when on or near the gate. Requires anglesfixed().
  maxvectors();		//snaps sufficiently high cardinal inputs to vectors of 1.0 magnitude of analog stick and c stick, overwrites middle section of perfectangles()
  shielddrops();	//gives an 8 degree range of shield dropping centered on SW and SE gates
  backdash();		//fixes dashback by imposing a 1 frame buffer upon tilt turn values
  backdashooc();	//allows more leniency for dash back out of crouch
  //dolphinfix();	//ensures close to 0 values are reported as 0 on the sticks to fix dolphin calibration and fixes poll speed issues, only necessary for backdash(ooc) without ms() -- Even on dolphin. Not neccessary if standardize is set to true.
  startZero();		//Place stick in middle until B is pressed - aids with setting bad controllers to perfectangles();
}

void anglesfixed(){
  ref = deg;
  if(deg>g.e&&deg<=g.n)      deg = mapa(deg, g.e, g.n,    0,  90);
  else if(deg>g.n&&deg<=g.w)  deg = mapa(deg, g.n,  g.w,   90, 180);
  else if(deg>g.w&&deg<=g.s)  deg = mapa(deg, g.w,  g.s,  180, 270);
  else if(deg>g.s&&deg<=g.e) deg = mapa(deg, g.s,  g.e, 270, 360);
  else
  {
    if(g.s>g.e) 	//Opting for else if to save performance.  Wonky notch configs will be more wonky though if not in order.
      deg = mapa(deg+360, g.s, nmod(g.e, 360) + 360,    270,  360);
    else if(g.w>g.s)
      deg = mapa(deg+360, g.w, nmod(g.s, 360) + 360,    180,  270);
    else if(g.n>g.w)
      deg = mapa(deg+360, g.n, nmod(g.w, 360) + 360,    90,  180);
    else if(g.e>g.n)
      deg = mapa(deg+360, g.e, nmod(g.n, 360) + 360,    0,  90);
  }
  perf.u = (arc(g.n) < perfAng); perf.d = (arc(g.s) < perfAng);
  perf.l = (arc(g.w) < perfAng); perf.r = (arc(g.e) < perfAng);
  gcc.xAxis=128+r*cos(deg/57.3);  gcc.yAxis=128+r*sin(deg/57.3);  
}

void perfectangles(){	//Made for regular use configurations, wonky notch configs (cardinals within 34 degrees of each other or out of order) will be wonky :(
  if(deg != 0 && deg != 90 && deg != 180 && deg != 270)
  {
    Serial.println("PerfectAngles Active");
    if(r>75)
    {
      if(perf.u){gcc.xAxis = (deg<90)?151:105; gcc.yAxis = 204;}
      else if(perf.r){gcc.yAxis = (deg>0&&deg<90)?151:105; gcc.xAxis = 204;}
      else if(perf.d){gcc.xAxis = (deg>270)?151:105; gcc.yAxis =  52;}
      else if(perf.l){gcc.yAxis = (deg<180)?151:105; gcc.xAxis =  52;}
    }
  }
}

void maxvectors(){
  if(r>75){
    if(arc(g.n)<6)
  {
    gcc.xAxis = 128; gcc.yAxis = 255;
    Serial.println("Snapping to max value n");
  }
    if(arc(g.e)<6)
  {
    gcc.xAxis = 255; gcc.yAxis = 128;
    Serial.println("Snapping to max value e");
  }
    if(arc(g.s)<6)
  {
    gcc.xAxis = 128; gcc.yAxis =   1;
    Serial.println("Snapping to max value s");
  }
    if(arc(g.w)<6)
  {
    gcc.xAxis =   1; gcc.yAxis = 128;
    Serial.println("Snapping to max value w");
  }
  }
  if(abs(cx)>75&&abs(cy)<23){gcc.cxAxis = (cx>0)?255:1; gcc.cyAxis = 128;}
  if(abs(cy)>75&&abs(cx)<23){gcc.cyAxis = (cy>0)?255:1; gcc.cxAxis = 128;}
}

void shielddrops(){
  shield = gcc.l||gcc.r||ls.l>74||ls.r>74||gcc.z;
  if(shield){
    if(ay<0&&r>72){
      if(arc(g.sw)<4)
      {
        gcc.yAxis = 73; gcc.xAxis =  73; 
        Serial.println("Snapping to shield drop value sw");
      }
      if(arc(g.se)<4)
      {
        gcc.yAxis = 73; gcc.xAxis = 183;
        Serial.println("Snapping to shield drop value se");
      }
    }
  }
}

void backdash()				//Can't tell which way character is facing, so have to buffer both sides :(
{ 
  button=gcc.a||gcc.b||gcc.x||gcc.y||gcc.z||gcc.l||gcc.r||ls.l>74||ls.r>74;
  if(abs(ay)<23&&!button)
  { 
    if(abs(ax)<23)
      buf.db = cyclesDB;
    if(buf.db>0)
    {
      buf.db--; 
      if(abs(ax)<64) 
      {
        gcc.xAxis = 128+ax*(abs(ax)<23);
        Serial.println("Replacing db value");
      }
      else 
        buf.db = 0;			//Modification for instant dashback and buffer deletion if already in smash zone before buffer completes
    } 
  }
  else buf.db = 0;
}

void backdashooc()			//Can't tell which way character is facing, so have to buffer both sides :(
{
  if(ay<23)
  {
    if(ay<-49) 
      buf.cr = cyclesCDB;
    if(buf.cr>0)
    {
      buf.cr--; 
      if((ay>-50)&&(abs(ax)<64)) 
      {
        gcc.yAxis = 78;
        Serial.println("Replacing dbooc value");
      }
      else if((abs(ax)>63)&&(ay>-50)) 
        buf.cr = 0;			//Modification for instant dashback and buffer deletion out of crouch if already in smash zone before buffer completes
    } 
  }
  else buf.cr = 0;
}

void backdashms()			//Can't tell which way character is facing, so have to buffer both sides :(
{ 
  button=gcc.a||gcc.b||gcc.x||gcc.y||gcc.z||gcc.l||gcc.r||ls.l>74||ls.r>74;
  if(abs(ay)<23&&!button)
  {
    if(abs(ax)<23)
      bufms.db = millis();	//millis() resets after 50 days so for cyclesDBms/1000 seconds after 50 days of the Arduino being continuously on the buffer will be triggered.
    if((millis() - bufms.db) < cyclesDBms / hzRatio)
      if(abs(ax)<64) 
      {
        gcc.xAxis = 128+ax*(abs(ax)<23);//gcc.xAxis = 255;
        Serial.println("Replacing db ms value");
      }
      else
        bufms.db = 0;		//Modification for instant dashback and buffer deletion if already in smash zone before buffer completes
  }
  else 
    bufms.db = 0;
}

void backdashoocms()		//Can't tell which way character is facing, so have to buffer both sides :(
{ 
  if(ay<23)
  {
    if(ay<-49) 
      bufms.cr = millis();	//millis() resets after 50 days so for cyclesDBms/1000 seconds after 50 days of the Arduino being continuously on the buffer will be triggered.
    if((millis() - bufms.cr) < cyclesCDBms / hzRatio)
    {
      if((ay>-50)&&(abs(ax)<64)) 
      {
        gcc.yAxis = 78;
        Serial.println("Replacing dbooc ms value");
      }
      else if((abs(ax)>63)&&(ay>-50)) 
        bufms.cr = 0;		//Modification for instant dashback and buffer deletion out of crouch if already in smash zone before buffer completes
    } 
  }
  else 
    bufms.cr = 0;
}

void dolphinfix(){
  if(r<8)         {gcc.xAxis  = 128; gcc.yAxis  = 128;}
  if(mag(cx,cy)<8){gcc.cxAxis = 128; gcc.cyAxis = 128;}
  if(!gcc.dup)
    cD = millis();
  if((cD + 500) < millis()) 
  {
    dolphin = true;
    gcc.yAxis = 255;
  } 
  cyclesDB = cycleRef.DB + (dMult*dolphin); 
  cyclesCDB = cycleRef.CDB + (dMult*dolphin);
}

void nocode(){
  if(gcc.ddown)
  {
    if(n == 0)
      n = millis();
    if(millis()-n>500)
    {
      if(!offPressed)
        off = !off;
      offPressed = true;
      if(!off)
      {
        gcc.yAxis = 255;
        gcc.xAxis = 128;
      }
      else
      {
        gcc.yAxis = 0;
        gcc.xAxis = 128;
      }
    }
  }
  else 
  {
    n = 0;
    offPressed = false;
  }
}

void recalibrate(){
  if(cal)
  {
    susIn = true; 
    cal = gcc.y&&gcc.start&&!gcc.z;
    Serial.println("Recalibration");
    ini.ax = gcc.xAxis -128; ini.ay = gcc.yAxis -128;   
    ini.cx = gcc.cxAxis-128; ini.cy = gcc.cyAxis-128;
    ini.l  = gcc.left;       ini.r  = gcc.right;
    if(!cal)
    {
      susIn = false;
    }
  }else if(gcc.y&&gcc.start&&!gcc.z){
    if(c == 0) 
      c = millis();
    cal = (millis() - c) > 200 / hzRatio;
  }
  else 
  {
    c = 0;
  }
}

void notchcalibrate(){
  if(calN)
  {
    susIn = true;
    notchloop();
  }
  else if(gcc.y&&gcc.start&&gcc.z)
  {
    if(cN == 0) 
      cN = millis();
    calN = (millis() - cN) > 200 / hzRatio;
  }
  else 
  {
    cN = 0;
  }
}

void notchloop(){
  Serial.println("Notch Calibrating");
  if(index <= 5)
  {
    if(gcc.b&&!pressed)
    {
      toSetX[index] = gcc.xAxis - 128;
      toSetY[index] = gcc.yAxis - 128;
      index++;
      pressed = true;
    }
    else if (!gcc.b&&pressed)
      pressed = false;
    if(gcc.a)
    {
      Serial.println("Escaping Notch Calibration");
      susIn = false;
      calN = false;
      index = 0;
      pressed = false;
    }
  }
  else
  {
    setnotches();
    Serial.println("Notches Calibrated");
    susIn = false;
    calN = false;
    index = 0;
    pressed = false;
  }
}

void setnotches(){
  Serial.println("Setting Received Notches");
  
  e__notch_x_value = toSetX[0];
  e__notch_y_value = toSetY[0];
  
  se_notch_x_value = toSetX[1];
  se_notch_y_value = toSetY[1];
  
  s__notch_x_value = toSetX[2];
  s__notch_y_value = toSetY[2];
  
  sw_notch_x_value = toSetX[3];
  sw_notch_y_value = toSetY[3];
 
  w__notch_x_value = toSetX[4];
  w__notch_y_value = toSetY[4];
  
  n__notch_x_value = toSetX[5];
  n__notch_y_value = toSetY[5];
  
  
  g.n  = ang(n__notch_x_value, n__notch_y_value);        //calculates angle of N notch
  g.e  = ang(e__notch_x_value, e__notch_y_value);        //calculates angle of E notch
  g.s  = ang(s__notch_x_value, s__notch_y_value);        //calculates angle of S notch
  g.w  = ang(w__notch_x_value, w__notch_y_value);        //calculates angle of W notch
  g.sw = ang(sw_notch_x_value, sw_notch_y_value);        //calculates angle of SW notch
  g.se = ang(se_notch_x_value, se_notch_y_value);        //calculates angle of SE notch
}

void initZone()
{
  if(initD)
  {
    if(!startZone&&!gcc.b)
    {
      if(abs(gcc.xAxis - 128) > zoneX)
        zoneX = abs(gcc.xAxis - 128);
      if(abs(gcc.yAxis - 128) > zoneY)
        zoneY = abs(gcc.yAxis - 128);
      if(gcc.a)
        initD = false;
    }
    else
      startZone = true;
      
      if((abs(gcc.xAxis - 128) < zoneX)&&(abs(gcc.yAxis - 128) < zoneY))
      {
        gcc.xAxis = 128 + ini.ax;
        gcc.yAxis = 128 + ini.ay;
        calibration();
      }
  }
}

void startZero()
{
  if(!start&&!gcc.b)
  {
    gcc.xAxis = 128;
    gcc.yAxis = 128;
  }
  else
  {
    start = true;
  }
}

void calibration(){
  ax = constrain(gcc.xAxis -128-ini.ax,-128,127); //offsets from nuetral position of analog stick x axis
  ay = constrain(gcc.yAxis -128-ini.ay,-128,127); //offsets from nuetral position of analog stick y axis
  cx = constrain(gcc.cxAxis-128-ini.cx,-128,127); //offsets from nuetral position of c stick x axis
  cy = constrain(gcc.cyAxis-128-ini.cy,-128,127); //offsets from nuetral position of c stick y axis
  r  = mag(ax, ay); deg  = ang(ax, ay);           //obtains polar coordinates for analog stick
  cr = mag(cx, cy);                               //obtains magnitude of c stick value
  ls.l = constrain(gcc.left -ini.l,0,255);        //fixes left trigger calibration
  ls.r = constrain(gcc.right-ini.r,0,255);        //fixes right trigger calibration
  gcc.left = ls.l; gcc.right = ls.r;              //sets proper analog shield values   
  gcc.xAxis  = 128+ax; gcc.yAxis  = 128+ay;       //reports analog stick values
  gcc.cxAxis = 128+cx; gcc.cyAxis = 128+cy;       //reports c stick values
}

float ang(float x, float y){return nmod(atan2(y,x)*57.3, 360);}		//returns angle in degrees when given x and y components
float mag(char  x, char  y){return sqrt(sq(x)+sq(y));}				//returns vector magnitude when given x and y components
bool  mid(float val, float n1, float n2){return val>n1&&val<n2;}	//returns whether val is between n1 and n2
float arc(float val)												//returns degrees between the deg and val
{
  nmod(val, 360);
  float least = abs(val - ref);
  float temp = abs(least - 360);
  if(temp < least)
    least = temp;
  return least;
}
int   dis(float val){return abs(fmod(val,90)-90*(fmod(val,90)>45));}	//returns how far off the given angle is from a cardinal
float mapa(long val, float in, float ix, float on, float ox){return (val-in)*(ox-on)/(ix-in)+on;}

float nmod(float n, float base)
{
  float num = fmod(n, base);
  while(num < 0)
  {
    num += base;
  }
  return num;
}

void setup(){
  Serial.println("Initial Setup");
  cycleRef.DB = 16;
  cycleRef.DB = 16;
  gcc.origin=0;gcc.errlatch=0;gcc.high1=0;gcc.errstat=0; //init values
  g.n  = ang(n__notch_x_value, n__notch_y_value);        //calculates angle of N notch
  g.e  = ang(e__notch_x_value, e__notch_y_value);        //calculates angle of E notch
  g.s  = ang(s__notch_x_value, s__notch_y_value);        //calculates angle of S notch
  g.w  = ang(w__notch_x_value, w__notch_y_value);        //calculates angle of W notch
  g.sw = ang(sw_notch_x_value, sw_notch_y_value);        //calculates angle of SW notch
  controller.read(); gcc = controller.getReport();       //reads controller once for calibration
  recalibrate();                                         //calibrates the controller for initial plug in
}

void loop()
{
  if(millis() - startTime >= standardTime || !standardize)
  {
    startTime = millis();
    controller.read();							//reads the controller
    data = defaultGamecubeData;					//this line is necessary for proper rumble
    gcc = controller.getReport();				//gets a report of the controller read
    calibration(); recalibrate();				//fixes normal calibration and allows resetting with x+y+start
    notchcalibrate();
    if(!off) mods();							//implements all the mods (remove this line to unmod the controller)
    nocode();									//function to disable all code if dpad down is held for 3 seconds
    if(!susIn)                                  //3ms before write
      data.report = gcc; console.write(data);   //sends controller data to the console
    controller.setRumble(data.status.rumble);   //allows for rumble
    /*
    // Time delay debugger
    if((millis() - startTime) > standardTime)
    {
      gcc.xAxis = 255;
      data.report = gcc; console.write(data);   //sends controller data to the console
    }
    */
  }
}
