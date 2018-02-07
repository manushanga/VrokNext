#include "threadpool.h"
#include "player.h"
#include "alsa.h"
#include "resampler.h"
#include "eq.h"
#include "ffmpeg.h"
#include "notifier_impl.h"
using namespace Vrok;

Player player;
Resampler resampler;
EffectSSEQ eq;
DriverAlsa out;
ThreadPool pool(2);

void Setup()
{
    Notify::GetInstance()->SetNotifier(new CNotifier);
    player.RegisterSink(&eq);
    eq.RegisterSource(&player);
    eq.RegisterSink(&resampler);
    resampler.RegisterSink(&out);
    resampler.RegisterSource(&eq);
    out.RegisterSource(&resampler);

    player.Preallocate();
    eq.Preallocate();
    resampler.Preallocate();
    out.Preallocate();

    pool.RegisterWork(0, &player);
    pool.RegisterWork(1, &eq);
    pool.RegisterWork(0, &resampler);
    pool.RegisterWork(1, &out);

    pool.CreateThreads();

}

void Play(const std::string& filename)
{
    DecoderFFMPEG* dec = new DecoderFFMPEG();
    Resource res;
    res._filename = filename;
    if (dec->Open(&res))
        player.SubmitForPlayback(dec);

}
int main()
{
    Setup();
    Play("/media/madura/Data1/pramod/ALL/SIMPSONS.wav");
    pool.JoinThreads();
}
