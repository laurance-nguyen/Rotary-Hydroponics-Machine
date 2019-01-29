/***************************************************
 This example reads Water flow sensor Sensor.
 
 Created 2016-3-13
 By berinie Chen <bernie.chen@dfrobot.com>
 
 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
 1.Connection and Diagram can be found here  http://www.dfrobot.com/wiki/index.php?title=Water_Flow_Sensor_-_1/8%22_SKU:_SEN0216
 2.This code is tested on Arduino Uno.
 ****************************************************/
volatile double waterFlow;
void setup() {
  Serial.begin(9600);  //baudrate
  waterFlow = 0;
  attachInterrupt(0, pulse, RISING);  //DIGITAL Pin 2: Interrupt 0
}
void loop() {
  Serial.print("Water Flow:");
  Serial.print(waterFlow);
  Serial.println(" mL");
  delay(500);
}

void pulse()   //measure the quantity of square wave (mL)
{
  waterFlow += 1000.0 / 5880.0;
}