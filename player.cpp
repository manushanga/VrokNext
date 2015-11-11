
#include "player.h"
#include "ffmpeg.h"

Vrok::Player::Player() :
    BufferGraph::Point(),
    _new_resource(false),
    _work(true),
    _command_queue(new Queue<Command>(5)),
    _command_now_queue(new Queue<Command>(5)),
    _decoder_work(false),
    _callback(nullptr),
    _paused(false)
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

void Vrok::Player::SetNextTrackCallback(Vrok::Player::NextTrackCallback callback, void *user)
{

    _callback = callback;
    _callback_user = user;
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
                    _decoder->GetBufferConfig(&bufc_new);
                    SetBufferConfig(&bufc_new);
                    delete (Resource *)cmd.data;
                }
            }
        } else {
            if (_callback)
                _callback(_callback_user);

            _decoder_work = _command_now_queue->Peak(cmd);
            if (!_decoder_work)
                this_thread::sleep_for(chrono::seconds(1));
        }
    } else
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
                    _decoder->GetBufferConfig(&bufc_new);
                    SetBufferConfig(&bufc_new);
                    delete (Resource *)cmd.data;
                    return;
                }
                else
                {
                    DBG("decoder fail");
                    return;
                }
            } else if (cmd.type == PAUSE)
            {
                _paused=true;
            } else if (cmd.type == RESUME)
            {
                _paused=false;
            }
        }
        if (!_paused)
        {
            auto b=AcquireBuffer();
            if (b )
            {

                std::cout<<"sid"<<_cur_stream_id<<std::endl;
                if (*b->GetBufferConfig() != *GetBufferConfig())
                {
                    b->Reset(GetBufferConfig());
                }
                b->GetWatch().Reset();
                _decoder_work = _decoder->DecoderRun(b, GetBufferConfig());
                atomic_thread_fence(memory_order_seq_cst);

                // don't push out failed buffers
                if (_decoder_work)
                    PushBuffer(b);
            }
        }
    }
}

