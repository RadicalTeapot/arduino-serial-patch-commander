#ifndef NoteEventManager_h
#define NoteEventManager_h

#include <Arduino.h>

namespace NoteEventManager
{
    enum class EventState : uint8_t
    {
        NOT_STARTED = 0,
        RUNNING,
        FINISHED
    };

    struct NoteEvent
    {
        uint8_t gatePin;
        volatile uint16_t* pwmRegisterAddress;
        uint8_t note;
        uint32_t start;
        uint32_t end;
        EventState state;
    };

    class IEventManager
    {
    public:
        virtual void setNoteEvent(uint8_t pin, uint8_t note) = 0;
        virtual void setGateEvent(uint8_t pin, uint16_t duration) = 0;
        virtual ~IEventManager() = default;
    };

    class EventManager : public IEventManager
    {
    public:
        EventManager(const NoteEvent* events, const size_t maxEventCount, const uint8_t* noteLookupTable, const size_t noteLookupTableSize);
        void updateEvents(uint32_t now);

        virtual void setNoteEvent(uint8_t pin, uint8_t note) override;
        virtual void setGateEvent(uint8_t pin, uint16_t duration) override;

    private:
        const size_t noteEventsSize;
        const size_t noteLookupTableSize;
        const NoteEvent* noteEvents;
        const uint8_t* noteLookupTable;

        void startEvent(NoteEvent *event);
        void stopEvent(NoteEvent *event);
        NoteEvent* findEvent(uint8_t pinNumber);
    };
}
#endif
