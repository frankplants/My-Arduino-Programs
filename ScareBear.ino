
//SCAREBEAR

int RELAY1 = 1;
int RELAY2 = 2;
//The pin that the relay is connected to.

long randNumber;

int Triggered = 8;
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


 //If the headphone jack is pulled, on for a minute to start.
 digitalWrite(RELAY, LOW);
 delay(60000); 

 digitalWrite(RELAY, HIGH);
 delay(60000); //Off for a minute
 
 do {
  randNumber = random(30000 , 120000); 
  //randomizes between 30 seconds and 2 minutes
  Serial.println(randNumber);
  digitalWrite(RELAY, LOW);
  delay(randNumber);
  
  randNumber = random(30000 , 120000);
  Serial.println(randNumber);
  digitalWrite(RELAY, HIGH);
  delay(randNumber);
  counter--; //Count down from 3
 
 } while(counter > 0);

}
else {

digitalWrite(RELAY, HIGH);
}
}
