#include <Arduino.h>
#include <NoteEventManager.h>
#include <NoteEventListener.h>

const uint8_t calibratedNotesCount = 44;
const uint8_t calibratedNotes[calibratedNotesCount] = {
//  C    C#   D    D#   E    F    F#   G    G#   A    A#   B
    0  , 5  , 9  , 14 , 18 , 23 , 28 , 33 , 37 , 42 , 46 , 51 ,
    56 , 61 , 65 , 70 , 74 , 79 , 83 , 88 , 93 , 98 , 102, 107,
    112, 117, 121, 126, 130, 135, 139, 144, 149, 154, 158, 163,
    168, 167, 172, 177, 181, 186, 190, 197
};

const NoteEventManager::NoteEvent noteEvents[1] = {
    {2, &OCR1A, 0, 0, 0, NoteEventManager::EventState::NOT_STARTED}
};

static NoteEventManager::EventManager eventManager(noteEvents, 1, calibratedNotes, calibratedNotesCount);
static NoteEventListener::EventListener eventListener(4, &eventManager);

// Generative mode
void setupNextNote();
void noteFromByteArray();

// From serial mode
void noteFromSerial();

void setup() {
    Serial.begin(31250);

    noInterrupts();
    TCCR1A =
        1 << COM1A1 // set compare match register A
        | 1 << WGM10; // Set 8 bit resolution
    TCCR1B =
        1 << WGM12 // Set fast PWM mode
        | 1 << CS10; // Set no pre-scaler
    pinMode(9, OUTPUT);  // Pin 9 is OCR1A
    OCR1A = 0;
    interrupts();

    pinMode(2, OUTPUT);
}

uint32_t last = 0;
uint16_t interval = 1000;
size_t index = 0;
uint16_t gateLength = interval / 2;
void loop() {
    // const uint32_t now = millis();
    // if (now - last >= interval) {
    //     last = now;
    //     // setupNextNote();
    //     noteFromByteArray();
    // }
    // eventManager.updateEvents(now);

    noteFromSerial();
    eventManager.updateEvents(millis());
}

const uint8_t gatePin = 2;
const size_t scaleNoteCount = 10;
const uint8_t scale[scaleNoteCount] = {
    12, 15, 17, 19, 22,  // minor pentatonic
    24, 27, 29, 31, 34,
};
void setupNextNote() {
    interval = random(1000, 10000);
    gateLength = random(interval / 8, interval / 2);
    eventManager.setNoteEvent(gatePin, scale[random(scaleNoteCount)]);
    eventManager.setGateEvent(gatePin, gateLength);
}

void noteFromByteArray()
{
    interval = 100;
    gateLength = random(interval / 8, interval / 2);
    uint8_t msbGateLength = gateLength >> 7 & 0x7F;
    uint8_t lsbGateLength = gateLength & 0x7F;
    uint8_t message[5] = {0x82, scale[random(scaleNoteCount)] & 0x7F, 0x92, msbGateLength, lsbGateLength};
    eventListener.parseBytes(message, 5);
}

void noteFromSerial() {
    int byte = Serial.read();
    while (byte != -1) {
        eventListener.parseByte(static_cast<uint8_t>(byte));
        byte = Serial.read();
    }
}
