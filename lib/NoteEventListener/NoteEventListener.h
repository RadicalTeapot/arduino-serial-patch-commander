#ifndef NoteEventListener_h
#define NoteEventListener_h

#include <Arduino.h>
#include <NoteEventManager.h>

namespace NoteEventListener
{
    enum class MessageType : uint8_t
    {
        Data = 0x7,
        NoteCommand = 0x8,
        GateCommand = 0x9
    };

    struct Message
    {
        MessageType type;
        uint8_t data;
    };

    struct MessageEvent
    {
        const uint8_t gatePin;
        Message *messageBuffer;
        const size_t messageBufferSize;
        size_t bufferWriteIndex;
    };

    class EventListener
    {
    public:
        EventListener(const MessageEvent *messageEvents, const size_t messageEventsSize, NoteEventManager::IEventManager *eventManager);
        void parseBytes(uint8_t *bytes, size_t size);
        void parseByte(uint8_t byte);

    private:
        const size_t messageEventsSize;
        const MessageEvent *messageEvents;
        uint8_t currentPinNumber;
        NoteEventManager::IEventManager *eventManager;

        bool sendCommandIfPossible(const MessageEvent *messageEvent);
        MessageEvent* findEvent(uint8_t pinNumber);
    };
}
#endif
