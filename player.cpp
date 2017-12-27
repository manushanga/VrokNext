
#include "player.h"
#include "ffmpeg.h"

#define INIT_GAPLESS_SECS 2

Vrok::Player::Player() :
    BufferGraph::Point(),
    _new_resource(false),
    _work(true),
    _command_queue(new Queue<Command>(5)),
    _queue_next(false),
    _command_now_queue(new Queue<Command>(5)),
    _decoder_work(false),
    _events(nullptr),
    _paused(false),
    _decoder(nullptr)
{
}

bool Vrok::Player::SubmitForPlayback(Vrok::Decoder *decoder)
{
    Command cmd;
    cmd.data = decoder;
    cmd.type = OPEN;
    _command_now_queue->PushBlocking(cmd);
    //_command_queue->Clear();
    return true;
}

bool Vrok::Player::Resume()
{
    Command cmd;
    cmd.data = NULL;
    cmd.type = RESUME;
    _command_now_queue->PushBlocking(cmd);
    return true;
}

bool Vrok::Player::Pause()
{
    Command cmd;
    cmd.data = NULL;
    cmd.type = PAUSE;
    _command_now_queue->PushBlocking(cmd);
    return true;
}
bool Vrok::Player::Stop()
{
    Command cmd;
    cmd.data = NULL;
    cmd.type = STOP;
    _command_now_queue->PushBlocking(cmd);
    return true;
}

bool Vrok::Player::Skip()
{
    Command cmd;
    cmd.data = NULL;
    cmd.type = SKIP;
    _command_now_queue->PushBlocking(cmd);
    return true;
}

void Vrok::Player::SetEvents(Vrok::Player::Events *events)
{
    _events = events;
}

void Vrok::Player::Run()
{
    Command cmd;
    bool got;
    bool stream_start = false;
    bool force_stop = false;
    bool gap_less_done = false;

    got = _command_now_queue->Pop(cmd);
    if (got)
    {
        // we got something on the play queue
        // close playing song and start next
        if (cmd.type == OPEN) {
            if (_decoder) {
                _decoder->Close();
                delete _decoder;
            }
            stream_start = true;
            gap_less_done = false;
            _decoder = (Vrok::Decoder *) cmd.data;
            BufferConfig config;
            _decoder->GetBufferConfig(&config);
            SetBufferConfig(&config);
        } else if (cmd.type == STOP)
        {
            if (_decoder) {
                _decoder->Close();
                delete _decoder;
            }
            force_stop = true;
            _decoder = nullptr;

        } else if (cmd.type == PAUSE)
        {
            _paused=true;
        } else if (cmd.type == RESUME)
        {
            _paused=false;
        } else if (cmd.type == SKIP)
        {
            if (_events  && _queue_next)
                _events->QueueNext();
        }
    }
    else if (!_decoder)
    {
        if (_events && _queue_next)
            _events->QueueNext();
    }

    if (_decoder && !_paused)
    {
        auto b=AcquireBuffer();
        if (stream_start)
            b->SetBufferType(Buffer::Type::StreamStart);
        else
            b->SetBufferType(Buffer::Type::StreamBuffer);

        if (b)
        {
            b->SetStreamId(_cur_stream_id);
            if (*b->GetBufferConfig() != *GetBufferConfig())
            {
                b->Reset(GetBufferConfig());
            }
            b->GetWatch().Reset();
            _decoder_work = _decoder->DecoderRun(b, GetBufferConfig());
            atomic_thread_fence(memory_order_seq_cst);

            if (_decoder->GetPositionInSeconds() + INIT_GAPLESS_SECS > _decoder->GetDurationInSeconds() && !gap_less_done)
            {
                DBG(0,"gapless request")
                if (_events && _queue_next)
                    _events->QueueNext();

                gap_less_done = true;
            }
            // don't push out failed buffers
            if (!_decoder_work)
            {
                _decoder->Close();
                delete _decoder;
                _decoder = NULL;

                if (_queue_next == false)
                    b->SetBufferType(Buffer::Type::StreamStop);
                else
                    b->SetBufferType(Buffer::Type::StreamEnd);
            }

            if (force_stop)
            {
                WARN(0, "force stop");
                b->SetBufferType(Buffer::Type::StreamStop);
            }

            PushBuffer(b);
        }
    }
    else if (_decoder == nullptr && _queue_next == false)
    {
       Vrok::Sleep(100);
    }
}

void Vrok::Player::SetQueueNext(bool queue_next)
{
    _queue_next = queue_next;
}

