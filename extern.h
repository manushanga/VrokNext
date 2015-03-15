#include "player.h"
#include "audioout.h"
#include "preamp.h"
#include "fir.h"

Vrok::Player pl;
Vrok::DriverAudioOut out;
Vrok::EffectFIR pre;
Vrok::EffectFIR pre1;




extern "C"
{
    Vrok::Resource *CreateResource(char *filename)
    {
        Vrok::Resource *res=new Vrok::Resource();
        res->_filename = std::string(filename);
        DBG(res);
        return res;
    }
    void DestroyResource(Vrok::Resource *resource)
    {
        delete resource;
    }
    void RegisterSource(BufferGraph::Point *parent, BufferGraph::Point *source)
    {
        parent->RegisterSource(source);
    }
    void RegisterSink(BufferGraph::Point *parent, BufferGraph::Point *sink)
    {
        parent->RegisterSource(sink);
    }

    Vrok::Player *CreatePlayer()
    {
        out.RegisterSource(&pre1);
        pre.RegisterSource(&pl);
        pre.RegisterSink(&pre1);
        pl.RegisterSink(&pre);
        pre1.RegisterSource(&pre);
        pre1.RegisterSink(&out);

        out.Preallocate();
        pl.Preallocate();
        pre.Preallocate();
        pre1.Preallocate();

        pre.CreateThread();
        pre1.CreateThread();
        out.CreateThread();
        pl.CreateThread();


        return &pl;
    }
    void JoinPlayer()
    {
        pl.JoinThread();
        pre.JoinThread();
        pre1.JoinThread();
        out.JoinThread();
    }
    void SubmitForPlayback(Vrok::Resource *resource)
    {
        pl.SubmitForPlayback(resource);
    }
    void SubmitForPlaybackNow(Vrok::Resource *resource)
    {
        pl.SubmitForPlaybackNow(resource);
    }
}
