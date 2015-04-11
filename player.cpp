
#include "player.h"
#include "ffmpeg.h"

Vrok::Player::Player() :
    _new_resource(false),
    _work(true),
    _command_queue(new Queue<Command>(5)),
    _command_now_queue(new Queue<Command>(5)),
    _decoder_work(false)
{
    _decoder = new DecoderFFMPEG();
}

bool Vrok::Player::SubmitForPlayback(Resource *resource)
{
    Command cmd;
    cmd.data = resource;
    cmd.type = OPEN;
    _command_queue->PushBlocking(cmd);
    return true;
}
bool Vrok::Player::SubmitForPlaybackNow(Resource* resource)
{
    Command cmd;
    cmd.data = resource;
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

void Vrok::Player::Run()
{
    Command cmd;
    bool got;


    if (!_decoder_work)
    {
        got = _command_queue->Pop(cmd);
        if (got)
        {
            if (cmd.type == OPEN)
            {
                _decoder->Close();
                _decoder_work=_decoder->Open((Resource *)cmd.data);
                if (_decoder_work)
                {
                    BufferConfig bufc_new;
                    _decoder->SetBufferConfig(&bufc_new);
                    SetBufferConfig(&bufc_new);
                    delete (Resource *)cmd.data;
                }
            }
        } else {

            _decoder_work = _command_now_queue->Peak(cmd);
            if (!_decoder_work)
                this_thread::sleep_for(chrono::seconds(1));
        }
    }

    if (_decoder_work)
    {
        got = _command_now_queue->Pop(cmd);
        if (got)
        {
            DBG("opening");
            // we got something on the play queue
            // close playing song and start next
            if (cmd.type == OPEN)
            {
                _decoder->Close();
                _decoder_work = _decoder->Open((Resource *)cmd.data);
                if (_decoder_work)
                {
                    BufferConfig bufc_new;
                    _decoder->SetBufferConfig(&bufc_new);
                    SetBufferConfig(&bufc_new);
                    delete (Resource *)cmd.data;
                }
                else
                {
                    return;
                }
            } else if (cmd.type == PAUSE)
            {

            }
        }
        auto b=AcquireBuffer();
        if (*b->GetBufferConfig() != *GetBufferConfig())
        {
            b->Reset(GetBufferConfig());
        }
        _decoder_work = _decoder->DecoderRun(b, GetBufferConfig());
        atomic_thread_fence(memory_order_seq_cst);

        PushBuffer(b);
    }
}

