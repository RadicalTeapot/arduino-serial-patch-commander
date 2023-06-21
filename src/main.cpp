#include <Arduino.h>
#include <NoteEventManager.h>
#include <NoteEventListener.h>

const uint8_t calibratedNotesSize = 44;
const uint8_t calibratedNotes[calibratedNotesSize] = {
//  C    C#   D    D#   E    F    F#   G    G#   A    A#   B
    0  , 5  , 9  , 14 , 18 , 23 , 28 , 33 , 37 , 42 , 46 , 51 ,
    56 , 61 , 65 , 70 , 74 , 79 , 83 , 88 , 93 , 98 , 102, 107,
    112, 117, 121, 126, 130, 135, 139, 144, 149, 154, 158, 163,
    168, 167, 172, 177, 181, 186, 190, 197
};

const size_t outputPinSize = 2;
const uint8_t pins[outputPinSize] = {2, 3};
const NoteEventManager::NoteEvent noteEvents[outputPinSize] = {
    {pins[0], &OCR1A, 0, 0, 0, NoteEventManager::EventState::NOT_STARTED},
    {pins[1], &OCR1B, 0, 0, 0, NoteEventManager::EventState::NOT_STARTED},
};

const size_t messageEventBufferSize = 4;
const NoteEventListener::MessageEvent messageEvents[outputPinSize] = {
    {pins[0], new NoteEventListener::Message[messageEventBufferSize], messageEventBufferSize, 0},
    {pins[1], new NoteEventListener::Message[messageEventBufferSize], messageEventBufferSize, 0},
};

static NoteEventManager::EventManager eventManager(noteEvents, outputPinSize, calibratedNotes, calibratedNotesSize);
static NoteEventListener::EventListener eventListener(messageEvents, outputPinSize, &eventManager);

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
        | 1 << COM1B1 // set compare match register B
        | 1 << WGM10; // Set 8 bit resolution
    TCCR1B =
        1 << WGM12 // Set fast PWM mode
        | 1 << CS10; // Set no pre-scaler
    pinMode(9, OUTPUT);  // Pin 9 is OCR1A
    pinMode(10, OUTPUT);  // Pin 10 is OCR1B
    OCR1A = 0;
    OCR1B = 0;
    interrupts();

    for (size_t i = 0; i < outputPinSize; i++)
        pinMode(noteEvents[i].gatePin, OUTPUT);
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
    interval = random(100, 1000);
    gateLength = random(interval / 8, interval / 2);
    uint8_t msbGateLength = gateLength >> 7 & 0x7F;
    uint8_t lsbGateLength = gateLength & 0x7F;
    uint8_t message[5] = {0x82, scale[random(scaleNoteCount)] & 0x7F, 0x92, msbGateLength, lsbGateLength};
    eventListener.parseBytes(message, 5);
    // message[0] = 0x83;
    // message[1] = 12;
    // message[2] = 0x93;
    // message[3] = 0;
    // message[4] = 100;
    // eventListener.parseBytes(message, 5);
}

void noteFromSerial() {
    int byte = Serial.read();
    while (byte != -1) {
        eventListener.parseByte(static_cast<uint8_t>(byte));
        byte = Serial.read();
    }
}
