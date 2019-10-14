///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_IMESSAGE_SOURCE_H
#define OSI_VISUALIZER_IMESSAGE_SOURCE_H

class IMessageSource
{
  public:
    IMessageSource() : isPaused_(false), isConnected_(false) {}

    bool isPaused_;
    bool isConnected_;
};
#endif  // OSI_VISUALIZER_IMESSAGE_SOURCE_H