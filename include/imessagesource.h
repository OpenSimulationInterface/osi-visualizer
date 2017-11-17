///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///


#pragma once


class IMessageSource
{
    public:
        IMessageSource(): isPaused_(false), isConnected_(false) {}

        bool isPaused_;
        bool isConnected_;

};
