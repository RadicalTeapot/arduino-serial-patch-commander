#include <NoteEventManager.h>

namespace NoteEventManager
{
    EventManager::EventManager(const NoteEvent *events, const size_t maxEventCount, const uint8_t *noteLookupTable, const size_t noteLookupTableSize)
        : noteEventsSize(maxEventCount),
          noteLookupTableSize(noteLookupTableSize),
          noteEvents(events),
          noteLookupTable(noteLookupTable) {}

    /**
     * Starts a NoteEvent by turning on the gatePin.
     *
     * @param current_event the NoteEvent to start
     */
    void EventManager::startEvent(NoteEvent *current_event)
    {
        digitalWrite(current_event->gatePin, HIGH);
    }

    /**
     * Stops the current event by setting the 'running' flag to false and
     * turning off the gatePin.
     *
     * @param current_event the NoteEvent to stop
     */
    void EventManager::stopEvent(NoteEvent *current_event)
    {
        digitalWrite(current_event->gatePin, LOW);
    }

    /**
     * Updates the events in the event manager by checking their current state
     * against the current time.
     *
     * @param now the current time in uint32_t format
     */
    void EventManager::updateEvents(uint32_t now)
    {
        for (size_t i = 0; i < noteEventsSize; i++)
        {
            NoteEvent *current_event = (NoteEvent *)&noteEvents[i];
            switch (current_event->state)
            {
            case EventState::NOT_STARTED:
                if (current_event->start <= now)
                {
                    startEvent(current_event);
                    current_event->state = EventState::RUNNING;
                }
                break;
            case EventState::RUNNING:
                if (current_event->end <= now)
                {
                    stopEvent(current_event);
                    current_event->state = EventState::FINISHED;
                }
                break;
            case EventState::FINISHED:
                break;
            }
        }
    }

    /**
     * Sets the note event for the given pin.
     *
     * @param pin the pin to set the note event for
     * @param note the note to set
     */
    void EventManager::setNoteEvent(uint8_t pin, uint8_t note)
    {
        auto noteEvent = findEvent(pin);

        if (noteEvent == nullptr)
            return;

        noteEvent->note = note;
        *(noteEvent->pwmRegisterAddress) = noteLookupTable[note % noteLookupTableSize];
    }

    /**
     * Sets a gate event for a given pin with a given duration.
     *
     * @param pin the pin number for the event
     * @param duration the duration of the event in milliseconds
     */
    void EventManager::setGateEvent(uint8_t pin, uint16_t duration)
    {
        auto noteEvent = findEvent(pin);

        if (noteEvent == nullptr)
            return;

        noteEvent->start = millis();
        noteEvent->end = noteEvent->start + duration;
        noteEvent->state = EventState::NOT_STARTED;
    }

    /**
     * Finds an event with the given pin number and returns a pointer to the corresponding NoteEvent object.
     *
     * @param pinNumber the pin number to search for
     *
     * @return a pointer to the matching NoteEvent object, or nullptr if no event was found
     */
    NoteEvent *EventManager::findEvent(uint8_t pinNumber)
    {
        for (size_t i = 0; i < noteEventsSize; i++)
        {
            if (noteEvents[i].gatePin == pinNumber)
                return (NoteEvent *)&noteEvents[i];
        }

        return nullptr;
    }
}
