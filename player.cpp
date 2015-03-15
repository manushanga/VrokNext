
#include "player.h"
#include "ffmpeg.h"

Vrok::Player::Player() :
    _new_resource(false),
    _work(true),
    _command_queue(new Queue<Command>(5)),
    _command_now_queue(new Queue<Command>(5))
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


void Vrok::Player::Run()
{
    while (_work)
    {
        Command cmd;
        bool got;
        bool decoder_work=false;

        got = _command_queue->Pop(cmd);
        if (got)
        {
            switch (cmd.type) {
            case OPEN:
            {
                _decoder->Close();
                decoder_work=_decoder->Open((Resource *)cmd.data);
                BufferConfig bufc_new;
                _decoder->SetBufferConfig(&bufc_new);
                SetBufferConfig(&bufc_new);
                delete (Resource *)cmd.data;
                break;
            }
            }
        } else {

            decoder_work = _command_now_queue->Peak(cmd);
            if (!decoder_work)
                this_thread::sleep_for(chrono::seconds(1));
        }



        while (decoder_work)
        {
            got = _command_now_queue->Pop(cmd);
            if (got)
            {
                DBG("opening");
                // we got something on the play queue
                // close playing song and start next
                switch (cmd.type) {
                case OPEN:
                {
                    _decoder->Close();
                    _decoder->Open((Resource *)cmd.data);
                    BufferConfig bufc_new;
                    _decoder->SetBufferConfig(&bufc_new);
                    SetBufferConfig(&bufc_new);
                    delete (Resource *)cmd.data;
                    break;
                }
                }
            }
            auto b=AcquireBuffer();
            if (*b->GetBufferConfig() != *GetBufferConfig())
            {
                b->Reset(GetBufferConfig());
            }
           // b->GetBufferConfig()->Print();
            decoder_work = _decoder->DecoderRun(b, GetBufferConfig());
            atomic_thread_fence(memory_order_seq_cst);
            //DBG("p"<<b);

            PushBuffer(b);
        }

    }
    DBG("play thread ending");
}

