#include <SPI.h>
#include <SD.h>

#define DAC_PIN 26
#define SAMPLE_RATE 14500

#define TRIG_PIN 14
#define ECHO_PIN 27
#define SOUND_SPEED 0.034

#define CS    5
#define SCK   18
#define MISO  19
#define MOSI  23

const char *audioFilePath = "/audio.raw";
File audioFile;

SPIClass spi = SPIClass(VSPI);

long duration;
float distanceCm;
uint8_t contagem = 10;
uint16_t timer10 = 10000;
unsigned long lastAbove30Time = 0;

bool isPlaying = false;

void setup() {
    Serial.begin(115200);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

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
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH);
    distanceCm = duration * SOUND_SPEED / 2;

    Serial.print("Distância: ");
    Serial.print(distanceCm);
    Serial.println(" cm");

    if (contagem) {
        contagem--;
        return;
    }

    if (distanceCm <= 30.0) {
        if (!isPlaying) {
            playAudio();
        }
        lastAbove30Time = millis();
    } else {
        if (millis() - lastAbove30Time > timer10) {
            if (isPlaying) {
                Serial.println("Objeto afastado por mais de 10s. Parando áudio...");
                audioFile.close();
                isPlaying = false;
            }
        }
    }
    delay(200);
}

void playAudio() {
    audioFile = SD.open(audioFilePath);
    if (!audioFile) {
        Serial.println("Erro ao abrir arquivo de áudio.");
        return;
    }

    Serial.println("Reproduzindo áudio...");
    isPlaying = true;
    lastAbove30Time = millis();

    while (audioFile.available()) {
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);

        duration = pulseIn(ECHO_PIN, HIGH, 25000);
        distanceCm = duration * SOUND_SPEED / 2;

        if (distanceCm > 30.0) {
            if (millis() - lastAbove30Time > timer10) {
                Serial.println("Distância > 30 cm por %d. Interrompendo áudio...", timer10);
                break;
            }
        } else {
            lastAbove30Time = millis();
        }
        uint8_t sample = audioFile.read();
        dacWrite(DAC_PIN, sample);
        delayMicroseconds(1000000 / SAMPLE_RATE);
    }
    audioFile.close();
    isPlaying = false;
    Serial.println("Áudio finalizado ou interrompido.");
}
