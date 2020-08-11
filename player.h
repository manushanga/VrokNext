/** Player interface
 * Copyright (C) Madura A.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02111-1307, USA.
 */
#pragma once

#include "common.h"
#include "bgpoint.h"
#include "queue.h"
#include "decoder.h"
#include "resource.h"

#include <string>
#include <atomic>

using namespace std;

namespace Vrok {

    class Player : public BufferGraph::Point, public Component
    {
    public:
        class Events
        {
        public:
            virtual void QueueNext() = 0;
        };

    private:
        enum class CommandType
        {
            OPEN,
            OPEN_NOW,
            PAUSE,
            RESUME,
            STOP,
            SEEK,
            SKIP
        };
        enum class PlayerState
        {
            START,
            PLAYING,
            PLAYING_GAPLESS_DONE,
            PAUSED,
            STOPPED
        };
        struct Command
        {
            CommandType type;
            void *data;
        };
        PlayerState _state;
    protected:

        atomic<bool> _work;
        bool _decoder_work;
        bool _queue_next;
        bool _resume_from_pause;
        Events *_events;
        // _play_queue: queue tracks needed to be played
        // one after other here
        // _play_now_queue: queue tracks will interrupt
        // the playback here

        Queue<Command> *_command_now_queue;

        Decoder *_decoder;

        void ResetDecoder();


    public:
        Player();

        virtual ~Player() {}

        // before submission to a decoder instance the track
        // must be checked for compatibility with the decoder
        // if a track that can not be played by the decoder
        // has been submitted, decoder will stop or goto the
        // next playable file

        bool SubmitForPlayback(Decoder* decoder);
        bool SubmitForPlaybackNow(Decoder *decoder);
        bool Resume();
        bool Pause();
        bool Stop();
        bool Skip();
        void SetQueueNext(bool queue_next);
        void SetEvents(Events *events);

        void Run();

        Vrok::ComponentType ComponentType()
        {
            return Vrok::ComponentType::Player;
        }
        Component *CreateSelf()
        {
            return new Player();
        }
        const char *ComponentName()
        {
            return "Player";
        }
        const char *Description()
        {
            return "Player";
        }
        const char *Author()
        {
            return "Madura A.";
        }
        const char *License()
        {
            return "GPL v2";
        }
    };
}
