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
    EventListener::EventListener(size_t maxCommandSize, NoteEventManager::IEventManager *eventManager)
        : lastMessageSize(maxCommandSize),
          lastMessageIndex(0),
          lastMessages(new Message[lastMessageSize]),
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

    void EventListener::parseByte(uint8_t byte)
    {
        const auto commandType = static_cast<MessageType>(byte >> 4);

        switch (commandType)
        {
        case MessageType::NoteCommand:
        case MessageType::GateCommand:
        {
            lastMessages[0] = {commandType, byte};
            lastMessageIndex = 1;
            break;
        }
        default:
        {
            if (commandType > MessageType::Data || lastMessageIndex <= 0)
                return;

            lastMessages[lastMessageIndex] = {MessageType::Data, byte};
            lastMessageIndex = (lastMessageIndex + 1) % lastMessageSize;
            break;
        }
        }
        sendNoteEventIfNeeded();
    }

    /**
     * Sends a note event to the event manager if either the last message index
     * is 2 and the message type is MessageType::NoteCommand, or if the last
     * message index is 3 and the message type is MessageType::GateCommand.
     */
    void EventListener::sendNoteEventIfNeeded()
    {
        // TODO Abstract the validation logic away if adding more commands to make this method more generic and robust

        if (lastMessageIndex != 2 && lastMessageIndex != 3)
            return;

        MessageType commandType = lastMessages[0].type;

        if (lastMessageIndex == 2 && commandType == MessageType::NoteCommand)
        {
            uint8_t pin = lastMessages[0].data & 0x0F;
            uint8_t noteData = lastMessages[1].data;
            eventManager->setNoteEvent(pin, noteData);
            lastMessageIndex = 0;
        }
        else if (lastMessageIndex == 3 && commandType == MessageType::GateCommand)
        {
            uint8_t pin = lastMessages[0].data & 0x0F;
            uint16_t gateLength = (lastMessages[1].data & 0x7F) << 7 | (lastMessages[2].data & 0x7F);
            eventManager->setGateEvent(pin, gateLength);
            lastMessageIndex = 0;
        }
    }
}
