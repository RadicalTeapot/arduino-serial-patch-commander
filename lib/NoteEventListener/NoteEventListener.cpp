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
        {
            const auto message = bytes[i];
            const auto commandType = static_cast<MessageType>(message >> 4);

            switch (commandType)
            {
            case MessageType::NoteCommand:
            case MessageType::GateCommand:
            {
                lastMessages[0] = {commandType, message};
                lastMessageIndex = 1;
                // If adding commands without a data byte, call sendNoteEventIfNeeded here too
                break;
            }
            default:
            {
                if (commandType > MessageType::Data || lastMessageIndex <= 0)
                    continue;

                lastMessages[lastMessageIndex] = {MessageType::Data, message};
                lastMessageIndex = (lastMessageIndex + 1) % lastMessageSize;
                sendNoteEventIfNeeded();
                break;
            }
            }
        }
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
            uint8_t note = lastMessages[0].data & 0x0F;
            uint8_t noteData = lastMessages[1].data;
            eventManager->setNoteEvent(note, noteData);
        }
        else if (lastMessageIndex == 3 && commandType == MessageType::GateCommand)
        {
            uint8_t gate = lastMessages[0].data & 0x0F;
            uint16_t gateLength = (lastMessages[1].data & 0x7F) << 7 | (lastMessages[2].data & 0x7F);
            eventManager->setGateEvent(gate, gateLength);
        }

        lastMessageIndex = 0;
    }
}
