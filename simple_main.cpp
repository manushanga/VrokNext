#include "eq.h"
#include "ffmpeg.h"
#include "metadata.h"
#include "notifier_impl.h"
#include "player.h"
#include "pulse.h"
#include "resampler.h"
#include "threadpool.h"
using namespace vrok;

Player player;
Resampler resampler;
EffectSSEQ eq;
DriverPulse out;
ThreadPool pool(2);

std::string filename;

void Play(const std::string &filename);

class PlayerEv : public Player::Events {
public:
    void QueueNext() { Play(filename); }
    void OnMetadataUpdate(Metadata *metadata) {
        for (auto m : *metadata) {
            std::cout << m.first << ":" << m.second << std::endl;
        }
        Metadata::Destory(metadata);
    }
};
PlayerEv playerEv;
void Setup() {
    Notify::GetInstance()->SetNotifier(new CNotifier);
    player.SetEvents(&playerEv);
    player.SetQueueNext(true);

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

void Play(const std::string &filename) {
    DecoderFFMPEG *dec = new DecoderFFMPEG();
    Resource res;
    res._filename = filename;
    if (dec->Open(&res))
        player.SubmitForPlayback(dec);
}
int main(int argc, char **argv) {
    Setup();
    filename = argv[1];
    Play(filename);
    pool.JoinThreads();
}
