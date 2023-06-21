#include "NoteEventListener.h"

namespace NoteEventListener
{
    /**
     * Constructs an EventListener object with the specified maximum size
     * of the message queue and the instance of NoteEventManager::IEventManager
     * to register to.
     *
     * @param maxCommandSize The maximum size of the message queue, should be one more than the max command length in bytes.
     * @param eventManager The NoteEventManager::IEventManager instance to register to.
     */
    EventListener::EventListener(const MessageEvent *messageEvents, const size_t messageEventsSize, NoteEventManager::IEventManager *eventManager)
        : messageEventsSize(messageEventsSize),
          messageEvents(messageEvents),
          currentPinNumber(0),
          eventManager(eventManager) {}

    /**
     * Parse incoming bytes and execute the corresponding action.
     *
     * @param bytes pointer to an array of bytes to parse
     * @param size size of the bytes array
     */
    void EventListener::parseBytes(uint8_t *bytes, size_t size)
    {
        for (size_t i = 0; i < size; i++)
            parseByte(bytes[i]);
    }

    /**
     * Parses a single byte of data and updates the message event buffer if necessary.
     *
     * @param byte the byte of data to parse
     */
    void EventListener::parseByte(uint8_t byte)
    {
        const auto commandType = static_cast<MessageType>(byte >> 4);
        if (commandType == MessageType::NoteCommand || commandType == MessageType::GateCommand)
            currentPinNumber = byte & 0x0F;

        auto messageEvent = findEvent(currentPinNumber);
        if (messageEvent == nullptr)
            return;

        if (commandType == MessageType::NoteCommand || commandType == MessageType::GateCommand)
        {
            messageEvent->messageBuffer[0] = {commandType, byte};
            messageEvent->bufferWriteIndex = 1;
        }
        else if (commandType <= MessageType::Data && messageEvent->bufferWriteIndex > 0)
        {
            messageEvent->messageBuffer[messageEvent->bufferWriteIndex] = {MessageType::Data, byte};
            messageEvent->bufferWriteIndex = (messageEvent->bufferWriteIndex + 1) % messageEvent->messageBufferSize;
        }
        else
        {
            return;
        }

        if (sendCommandIfPossible(messageEvent))
        {
            messageEvent->bufferWriteIndex = 0;
        }
    }

    /**
     * Checks if the message event is valid to trigger an event and sends the
     * appropriate command to the event manager.
     *
     * @param messageEvent pointer to the message event containing the buffer write
     * index, gate pin, message buffer and command type.
     *
     * @return true if the command was sent successfully, false otherwise.
     */
    bool EventListener::sendCommandIfPossible(const MessageEvent *messageEvent)
    {
        const auto writeIndex = messageEvent->bufferWriteIndex;
        const auto pin = messageEvent->gatePin;
        const auto &messageBuffer = messageEvent->messageBuffer;
        const auto commandType = messageBuffer[0].type;

        if (writeIndex != 2 && writeIndex != 3)
            return false;

        if (writeIndex == 2 && commandType == MessageType::NoteCommand)
        {
            const auto noteData = messageBuffer[1].data;
            // Serial.println("Sending note command on pin " + String(pin) + " of value " + String(noteData));
            eventManager->setNoteEvent(pin, noteData);
            return true;
        }

        if (writeIndex == 3 && commandType == MessageType::GateCommand)
        {
            const auto highByte = messageBuffer[1].data & 0x7F;
            const auto lowByte = messageBuffer[2].data & 0x7F;
            const auto gateLength = (highByte << 7) | lowByte;
            // Serial.println("Sending gate command on pin " + String(pin) + " of duration " + String(gateLength));
            eventManager->setGateEvent(pin, gateLength);
            return true;
        }

        return false;
    }

    /**
     * Finds a MessageEvent object in the messageEvents array based on given pinNumber.
     *
     * @param pinNumber an unsigned 8-bit integer representing the pin number to search for
     *
     * @return a pointer to the MessageEvent object if found, otherwise a null pointer
     */
    MessageEvent *EventListener::findEvent(uint8_t pinNumber)
    {
        for (size_t i = 0; i < messageEventsSize; i++)
        {
            if (messageEvents[i].gatePin == pinNumber)
                return (MessageEvent *)&messageEvents[i];
        }

        return nullptr;
    }
}
