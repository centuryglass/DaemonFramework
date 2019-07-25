/**
 * @file  Pipe_Listener.h
 *
 * @brief  Handles data read from a pipe.
 */

#pragma once
#include <cstddef>

namespace DaemonFramework { namespace Pipe { class Listener; }}

class DaemonFramework::Pipe::Listener
{
    private:
        friend class Reader;

        /**
         * @brief  Processes new data received from a pipe.
         *
         * @param data  A raw data array.
         *
         * @param size  Size in bytes of the data array.
         */
        virtual void processData
        (const unsigned char* data, const size_t size) = 0;
};
