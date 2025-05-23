
#include "player.h"
#include <unistd.h>

using namespace std;

#define INIT_GAPLESS_SECS 2

vrok::Player::Player()
    : BufferGraph::Point(), _work(true), _queue_next(false), _command_now_queue(new Queue<Command>(5)),
      _decoder_work(false), _events(nullptr), _decoder(nullptr), _state(PlayerState::START),
      _resume_from_pause(false) { }

bool vrok::Player::SubmitForPlayback(vrok::Decoder *decoder) {
    Command cmd;
    cmd.data = decoder;
    cmd.type = CommandType::OPEN;
    _command_now_queue->PushBlocking(cmd);
    //_command_queue->Clear();
    return true;
}

bool vrok::Player::SubmitForPlaybackNow(vrok::Decoder *decoder) {
    Command cmd;
    cmd.data = decoder;
    cmd.type = CommandType::OPEN_NOW;
    _command_now_queue->PushBlocking(cmd);
    //_command_queue->Clear();
    return true;
}

bool vrok::Player::Resume() {
    Command cmd;
    cmd.data = NULL;
    cmd.type = CommandType::RESUME;
    _command_now_queue->PushBlocking(cmd);
    return true;
}

bool vrok::Player::Pause() {
    Command cmd;
    cmd.data = NULL;
    cmd.type = CommandType::PAUSE;
    _command_now_queue->PushBlocking(cmd);
    return true;
}
bool vrok::Player::Stop() {
    Command cmd;
    cmd.data = NULL;
    cmd.type = CommandType::STOP;
    _command_now_queue->PushBlocking(cmd);
    return true;
}

bool vrok::Player::Skip() {
    Command cmd;
    cmd.data = NULL;
    cmd.type = CommandType::SKIP;
    _command_now_queue->PushBlocking(cmd);
    return true;
}

void vrok::Player::SetEvents(vrok::Player::Events *events) {
    _events = events;
}

void vrok::Player::Run() {
    Command cmd;
    bool got;

    switch (_state) {
    case PlayerState::START: {
        got = _command_now_queue->Pop(cmd);
        if (got) {
            if (cmd.type == CommandType::OPEN || cmd.type == CommandType::OPEN_NOW) {
                _command_now_queue->Push(cmd);
                _state = PlayerState::PLAYING;
            }
        }
        break;
    }
    case PlayerState::PLAYING: {
        got = _command_now_queue->Pop(cmd);

        Buffer::Type bType = Buffer::Type::StreamBuffer;
        Buffer *b = nullptr;

        if (got) {
            // guarantee buffer for StreamStart otherwise,
            // this state would not propagate
            do {
                b = AcquireBuffer();
                if (b == nullptr) {
                    DBG(0, "waiting for free buffer");
                    usleep(50000);
                }
            } while (b == nullptr);

            if (cmd.type == CommandType::OPEN_NOW) {
                ResetDecoder();
                bType = Buffer::Type::StreamStart;
                _decoder = (vrok::Decoder *)cmd.data;
                BufferConfig config;
                _decoder->GetBufferConfig(&config);
                SetBufferConfig(&config);

                _state = PlayerState::PLAYING;
            } else if (cmd.type == CommandType::OPEN) {
                ResetDecoder();
                bType = Buffer::Type::StreamBuffer;
                _decoder = (vrok::Decoder *)cmd.data;
                BufferConfig config;
                _decoder->GetBufferConfig(&config);
                SetBufferConfig(&config);

                _state = PlayerState::PLAYING;

            } else if (cmd.type == CommandType::STOP) {
                ResetDecoder();
                bType = Buffer::Type::StreamStop;

                _state = PlayerState::STOPPED;
            } else if (cmd.type == CommandType::PAUSE) {
                bType = Buffer::Type::StreamPause;
                _state = PlayerState::PAUSED;
            } else if (cmd.type == CommandType::SKIP) {

                if (_events && _queue_next) {
                    _events->QueueNext();
                }
                _state = PlayerState::START;
                bType = Buffer::Type::StreamStop;
            }
        }

        if (b == nullptr) {
            b = AcquireBuffer();
            if (b == nullptr) {
                usleep(500);
                break;
            } else {
                /* hack for sending StreamResume on first buffer after pause */
                if (_resume_from_pause) {
                    b->SetBufferType(Buffer::Type::StreamResume);
                    _resume_from_pause = false;
                }
            }
        }
        b->SetBufferType(bType);
        /* _decoder might be not set if the track ends before next track is quequed */
        if (_decoder != nullptr) {
            b->SetStreamId(_cur_stream_id);
            if (*b->GetBufferConfig() != *GetBufferConfig()) {
                b->Reset(GetBufferConfig());
            }
            b->GetWatch().Reset();
            _decoder_work = _decoder->DecoderRun(b, GetBufferConfig());
            Metadata *metadata = nullptr;
            while ((metadata = _decoder->PopMetadataEvent()) != nullptr) {
                if (_events) {
                    _events->OnMetadataUpdate(metadata);
                }
            }

            if (_decoder->GetPositionInSeconds() + INIT_GAPLESS_SECS > _decoder->GetDurationInSeconds()) {
                WARN(0, "gapless request")
                if (_events && _queue_next) {
                    _events->QueueNext();

                    _state = PlayerState::PLAYING_GAPLESS_DONE;
                }
            }

            if (!_decoder_work) {
                b->Silence();
                ResetDecoder();

                if (_queue_next == false) {
                    b->SetBufferType(Buffer::Type::StreamStop);

                    _state = PlayerState::STOPPED;
                } else {
                    b->SetBufferType(Buffer::Type::StreamEnd);

                    _state = PlayerState::PLAYING;
                    if (_events) {
                        _events->QueueNext();
                    }
                }
            }
        }
        PushBuffer(b);
        break;
    }
    case PlayerState::PLAYING_GAPLESS_DONE: {
        auto b = AcquireBuffer();

        if (b) {
            b->SetStreamId(_cur_stream_id);
            if (*b->GetBufferConfig() != *GetBufferConfig()) {
                b->Reset(GetBufferConfig());
            }
            b->GetWatch().Reset();
            _decoder_work = _decoder->DecoderRun(b, GetBufferConfig());
            Metadata *metadata = nullptr;
            while ((metadata = _decoder->PopMetadataEvent()) != nullptr) {
                if (_events) {
                    _events->OnMetadataUpdate(metadata);
                }
            }

            // don't push out failed buffers
            if (!_decoder_work) {
                b->Silence();
                ResetDecoder();

                if (_queue_next == false) {
                    b->SetBufferType(Buffer::Type::StreamStop);

                    _state = PlayerState::STOPPED;
                } else {
                    b->SetBufferType(Buffer::Type::StreamEnd);

                    _state = PlayerState::PLAYING;
                    // no need to re-submit, the gapless request already
                    // loaded next track
                }
            }

            PushBuffer(b);
        }
        break;
    }
    case PlayerState::PAUSED: {
        got = _command_now_queue->Pop(cmd);
        if (got) {
            if (cmd.type == CommandType::RESUME) {
                _resume_from_pause = true;
                _state = PlayerState::PLAYING;
            }
        }
        break;
    }
    case PlayerState::STOPPED: {
        got = _command_now_queue->Pop(cmd);
        if (got) {
            if (cmd.type == CommandType::OPEN || cmd.type == CommandType::OPEN_NOW) {
                _command_now_queue->Push(cmd);
                _state = PlayerState::PLAYING;
            }
        }
        break;
    }
    }
}
void vrok::Player::SetQueueNext(bool queue_next) {
    _queue_next = queue_next;
}

void vrok::Player::ResetDecoder() {
    if (_decoder) {
        _decoder->Close();
        delete _decoder;
    }
    _decoder = nullptr;
}
