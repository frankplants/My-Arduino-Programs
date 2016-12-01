
//SCAREBEAR

int RELAY1 = 1
//The pin that the relay is connected to.

long randNumber;

int Triggered = 3;
//The pin that the our headphone jack is connected to.

int counter = 3; 
//The amount of times it randomizes being on and off

void setup() {
 
pinMode(RELAY1, OUTPUT);
pinMode(RELAY2, OUTPUT);
pinMode(Triggered, INPUT);


}

void loop() {

 if (digitalRead(Triggered) == HIGH) {

 //RELAY1 is on for a minute and RELAY2 is off for a minute
 digitalWrite(RELAY1, HIGH);
 delay(60000); 

 digitalWrite(RELAY1, LOW);               
 delay(60000); 
 
 do {
  randNumber = random(30000 , 120000); 
  //randomizes between 30 seconds and 2 minutes
  Serial.println(randNumber);
  digitalWrite(RELAY1, HIGH);
  delay(randNumber);
  
  randNumber = random(30000 , 120000);
  digitalWrite(RELAY1, LOW); 
  delay(randNumber);
  counter--; //Count down from 3
 
 } while(counter > 0);

}
else {
digitalWrite(RELAY1, LOW);
}
}
