#include "audioout.h"
#include "fir.h"
#include "player.h"
#include "preamp.h"
#include "threadpool.h"

vrok::Player *pl;
vrok::DriverAudioOut *out;
vrok::EffectFIR *pre;
vrok::EffectFIR *pre1;
ThreadPool *pool;
static vrok::Player *plx = NULL;

void NextTrackCallback(void *user) { }

extern "C" {
void CreateContext() {
    pl = new vrok::Player;
    out = new vrok::DriverAudioOut;
    pre = new vrok::EffectFIR;
    pre1 = new vrok::EffectFIR;
    pool = new ThreadPool(1);
}
vrok::Resource *CreateResource(const char *filename) {
    vrok::Resource *res = new vrok::Resource();
    res->_filename = std::string(filename);
    return res;
}
void DestroyResource(vrok::Resource *resource) {
    delete resource;
}
void RegisterSource(BufferGraph::Point *parent, BufferGraph::Point *source) {
    parent->RegisterSource(source);
}
void RegisterSink(BufferGraph::Point *parent, BufferGraph::Point *sink) {
    parent->RegisterSource(sink);
}

vrok::Player *CreatePlayer() {
    pre->RegisterSource(pl);
    pre->RegisterSink(out);
    pl->RegisterSink(pre);
    out->RegisterSource(pre);

    out->Preallocate();
    pl->Preallocate();
    pre->Preallocate();
    //        pre1->Preallocate();

    pool->RegisterWork(0, pl);
    pool->RegisterWork(0, pre);
    pool->RegisterWork(0, out);

    pl->SetNextTrackCallback(NextTrackCallback, nullptr);

    pool->CreateThreads();

    return pl;
}
void JoinPlayer() {
    pool->JoinThreads();
}
void SubmitForPlayback(vrok::Resource *resource) {
    pl->SubmitForPlayback(resource);
}
void SubmitForPlaybackNow(vrok::Resource *resource) {
    pl->SubmitForPlaybackNow(resource);
}
void DeleteContext() {
    delete pl;
    delete out;
    delete pre;
    delete pool;
    delete pre1;
}
}
