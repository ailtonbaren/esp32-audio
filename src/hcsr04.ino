#include <SPI.h>
#include <SD.h>

#define CS    5
#define SCK   18
#define MISO  19
#define MOSI  23

short int contagem = 10;
#define trigPin 14
#define echoPin 27
#define SOUND_SPEED 0.034

#define DAC_PIN 26
#define SAMPLE_RATE 14500

const char *audioFilePath = "/audio.raw";
File audioFile;

SPIClass spi = SPIClass(VSPI);

long duration;
float distanceCm;

void setup() {
    Serial.begin(115200);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    spi.begin(SCK, MISO, MOSI, CS);

    if (!SD.begin(CS, spi, 80000000)) {
        Serial.println("Falha ao montar o cartão SD");
        while (true);
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("Nenhum cartão SD detectado");
        while (true);
    }

    Serial.println("Cartão SD inicializado com sucesso.");
    Serial.println("Sistema pronto. Aproxime algo do sensor...");
}

void loop() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distanceCm = duration * SOUND_SPEED / 2;

    Serial.print("Distância: ");
    Serial.print(distanceCm);
    Serial.println(" cm");
    if(contagem){
        contagem--;
        return;
    }

    if (distanceCm <= 30) {
        playAudio();
    }

    delay(1000);
}

void playAudio() {
    audioFile = SD.open(audioFilePath);
    if (!audioFile) {
        Serial.println("Erro ao abrir arquivo de áudio.");
        return;
    }
    Serial.println("Reproduzindo áudio...");

    while (audioFile.available()) {
        uint8_t sample = audioFile.read();
        dacWrite(DAC_PIN, sample);
        delayMicroseconds(1000000 / SAMPLE_RATE);
    }

    Serial.println("Áudio finalizado.");
    audioFile.close();
}
