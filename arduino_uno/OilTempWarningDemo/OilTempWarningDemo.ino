void setup() {
    Serial.begin(9600);
}

void loop() {
    manager.UpdateAll();
    delay(100);
}