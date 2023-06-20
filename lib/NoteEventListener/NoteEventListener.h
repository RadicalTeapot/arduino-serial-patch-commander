#ifndef NoteEventListener_h
#define NoteEventListener_h

#include <Arduino.h>
#include <NoteEventManager.h>

namespace NoteEventListener
{
    class EventListener
    {
    public:
        EventListener(size_t maxCommandSize, NoteEventManager::IEventManager *eventManager);
        void parseBytes(uint8_t *bytes, size_t size);

    private:
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

        size_t lastMessageSize;
        size_t lastMessageIndex;
        Message *lastMessages;
        NoteEventManager::IEventManager *eventManager;

        void sendNoteEventIfNeeded();
    };
}
#endif
