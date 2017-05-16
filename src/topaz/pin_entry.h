#ifndef TOPAZ_PIN_ENTRY_H
#define TOPAZ_PIN_ENTRY_H

/**
 * Topaz - PIN Entry
 *
 * Functions and definitions for securely entering PINs and passwords.
 *
 * Copyright (c) 2016, T Parys
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string>
#include <signal.h>

namespace topaz
{
    /**
     * Set terminal echo
     *
     * \param echo Enable echo on true, false otherwise
     */
    void set_terminal_echo(bool echo);

    /**
     * PIN Entry Signal Handler
     */
    void pin_signal_handler(int signum, siginfo_t *info, void *ctx);

    /**
     * Read a PIN from file
     *
     * \param path File path to read
     * \return String containing file contents
     */
    std::string pin_from_file(char const *path);

    /**
     * Read a PIN from console
     *
     * \param prompt Name of PIN to request
     * \return String containing entered PIN
     */
    std::string pin_from_console(char const *prompt);

    /**
     * Read a PIN from console with confirmation
     *
     * \param prompt Name of PIN to request
     * \return String containing entered PIN
     */
    std::string pin_from_console_check(char const *prompt);

};

#endif
