#include "player.h"
#include "audioout.h"
#include "preamp.h"
#include "threadpool.h"
#include "fir.h"

Vrok::Player *pl;
Vrok::DriverAudioOut *out;
Vrok::EffectFIR *pre;
Vrok::EffectFIR *pre1;
ThreadPool *pool;
static Vrok::Player *plx=NULL;


void NextTrackCallback(void *user) 
{

}

extern "C"
{
    void CreateContext()
    {
        pl = new Vrok::Player;
        out = new Vrok::DriverAudioOut;
        pre = new Vrok::EffectFIR;
        pre1 = new Vrok::EffectFIR;
        pool = new ThreadPool(1);
    }
    Vrok::Resource *CreateResource(const char *filename)
    {
        Vrok::Resource *res=new Vrok::Resource();
        res->_filename = std::string(filename);
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
        pre->RegisterSource(pl);
        pre->RegisterSink(out);
        pl->RegisterSink(pre);
    	out->RegisterSource(pre);

    	out->Preallocate();
        pl->Preallocate();
        pre->Preallocate();
//        pre1->Preallocate();

        pool->RegisterWork(0,pl);
        pool->RegisterWork(0,pre);
        pool->RegisterWork(0,out);
        
        pl->SetNextTrackCallback(NextTrackCallback,nullptr);
        
        pool->CreateThreads();
        
        return pl;
    }
    void JoinPlayer()
    {
        pool->JoinThreads();
    }
    void SubmitForPlayback(Vrok::Resource *resource)
    {
        pl->SubmitForPlayback(resource);
    }
    void SubmitForPlaybackNow(Vrok::Resource *resource)
    {
        pl->SubmitForPlaybackNow(resource);
    }
    void DeleteContext()
    {
        delete pl;
        delete out;
        delete pre;
        delete pool;
        delete pre1;
    }
}

