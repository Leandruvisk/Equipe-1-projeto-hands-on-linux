#define ledPin 4
#define ldrPin 2

#define ldrMax 200
int ledValue;
int led_threshold = 0;

uint32_t last_tick = 0;

void setup() {
    Serial.begin(9600);
    
    pinMode(ledPin, OUTPUT);
    pinMode(ldrPin, INPUT);
    
    Serial.printf("SmartLamp Initialized.\n");
    last_tick = millis();

}

// Função loop será executada infinitamente pelo ESP32
void loop() {
    
    if (Serial.available() > 0) {
      last_tick = millis();
      String dadoRecebido = Serial.readString();
      Serial.print("Voce digitou: ");
      Serial.println(dadoRecebido);
    
      processCommand(dadoRecebido);
    }
    ledUpdate();
    periodic_message();
    //Obtenha os comandos enviados pela serial 
    //e processe-os com a função processCommand
}


void processCommand(String command) {
    // compare o comando com os comandos possíveis e execute a ação correspondente      
    if (command.substring(0, sizeof("SET_LED") - 1) == "SET_LED") {
        String string_valor_led = command.substring(sizeof("SET_LED"));
        int int_valor_led = string_valor_led.toInt();
    
        if (int_valor_led > 0 && int_valor_led <= 100) {
          ledValue = int_valor_led;
          Serial.println("RES SET_LED 1");
        } else {
          Serial.println("RES SET_LED -1");
        }
    }
    else if(command.substring(0, sizeof("GET_LED") - 1) == "GET_LED"){
      Serial.print("RES GET_LED ");
      Serial.println(ledValue);
    }
    else if(command.substring(0, sizeof("GET_LDR") - 1) == "GET_LDR"){
      int ldr_read_data = ldrGetValue();
      Serial.print("RES GET_LDR ");
      Serial.println(ldr_read_data);
    }
    else if (command.substring(0, sizeof("SET_THRESHOLD") - 1) == "SET_THRESHOLD") {
        String string_valor_threshold = command.substring(sizeof("SET_THRESHOLD"));
        int int_valor_threshold = string_valor_threshold.toInt();
    
        if (int_valor_threshold > 0 && int_valor_threshold <= 100) {
          led_threshold = int_valor_threshold;
          Serial.print("RES SET_THRESHOLD ");
          Serial.println(led_threshold);
        } else {
          Serial.println("ERR Unknown command");
        }
    }
    else if(command.substring(0, sizeof("GET_THRESHOLD") - 1) == "GET_THRESHOLD"){
      Serial.print("RES GET_THRESHOLD ");
      Serial.println(led_threshold);
    }
    else{
      Serial.print("ERR Unknown command");
    }
}

// Função para atualizar o valor do LED
void ledUpdate() {
    // Valor deve convertar o valor recebido pelo comando SET_LED para 0 e 255
    // Normalize o valor do LED antes de enviar para a porta correspondente
    if(ldrGetValue() > led_threshold){
      analogWrite(ledPin, 0);
    }
    else{
      int ledValue_normalizated = ledValue*255/100;
      analogWrite(ledPin, ledValue_normalizated);
    }
}

// Função para ler o valor do LDR
int ldrGetValue() {
  int ldr_read_data = analogRead(ldrPin);
  ldr_read_data = (ldr_read_data*100/4095);
  return ldr_read_data;
}

void periodic_message(){
  if(millis() - last_tick >= 2000){
    last_tick = millis();
    Serial.print("RES GET_LDR ");
    Serial.println(ldrGetValue());
  }
}
