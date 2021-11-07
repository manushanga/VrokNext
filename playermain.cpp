#include <filesystem>
#include <cstdio>
#include <cstring>

#include "cuesheet.h"
#include "eventcallbacks.h"
#include "notify.h"
#include "player.h"
#include "pulse.h"
#include "effect.h"
#include "resource.h"
#include "threadpool.h"
#include "ffmpeg.h"
#include "resource.h"
#include "notifier_impl.h"
#include "mpris2.h"
namespace fs = std::filesystem;
using fs::recursive_directory_iterator;
using dp = std::filesystem::directory_options;

struct resource
{
    std::string url;
    std::string title;
    unsigned long seekto;
};
struct playlist
{
    std::vector<resource> list;
};
struct file_handler
{
    char extension[8];
    void (*file_handler_func)(playlist* pl, const char* filename);
};
struct playercontext
{
    Vrok::Player player;
    Vrok::DriverPulse out;
    Vrok::ThreadPool pool;
    
    bool ispaused;
    playercontext() 
        : pool(2), ispaused(false)
    {}
};

void randomplay(playercontext* ctx, playlist* pl);
class playerevents : public Vrok::Player::Events
{
public:
    playerevents(playercontext* ctx, playlist* pl) :
        _ctx(ctx),
        _pl(pl)
    {}
    void QueueNext()
    {
        randomplay(_ctx, _pl);
    }
private:
    playlist* _pl;
    playercontext* _ctx;
};


void enque(playlist* pl, const char* filename)
{
    const auto& p = fs::path(filename);
    pl->list.push_back(resource{filename, p.filename(), 0});
}
unsigned long time_to_secs(int hours, int mins, int secs)
{
    return (hours * 3600) + (mins * 60) + secs;
}
void enque_cuesheet(playlist* pl, const char* filename)
{
    cuesh_data* cd = cuesh_init(filename);
    if (cd == nullptr)
    {
        return;
    }
    const auto& p = fs::path(filename);
    auto parent = p.parent_path();
    parent += "/";
    parent += cd->filename;

    for (auto& track : cd->tracks)
    {
        printf("%d %d\n", track->minutes, track->seconds);
        pl->list.push_back(resource{
                .url = parent, .title = track->title, 
                .seekto = time_to_secs(track->hours, track->minutes, track->seconds) });
    }
}
file_handler handlers[] = 
{
    { "mp3", enque },
    { "webm", enque },
    { "mp4", enque },
    { "m4a", enque },
    { "aac", enque },
    { "flac", enque },
    { "cue", enque_cuesheet }
};
playlist mainpl;
void enum_files(const char* dirname)
{
    for (const auto& file : recursive_directory_iterator(dirname, dp::skip_permission_denied))
    {
        std::string pathstr = file.path();
        std::string extstr = file.path().extension();
        if (extstr.size() > 1)
        {
            extstr = extstr.substr(1);
            for (const auto& handler : handlers)
            {
                if (strcmp(handler.extension, extstr.c_str()) == 0)
                {
                    handler.file_handler_func(&mainpl, pathstr.c_str());
                }
            }
        }
    }
}
void list_pl()
{
    for (auto& r : mainpl.list)
    {
        printf("%s %f\n", r.url.c_str(), r.seekto);
    }
}
playercontext* plctx_create()
{
    Vrok::Notify::GetInstance()->SetNotifier(new CNotifier);
    playercontext* ctx = new playercontext();
    ctx->player.SetQueueNext(true);
    ctx->player.SetEvents(new playerevents(ctx, &mainpl));
    ctx->player.Preallocate();
    ctx->out.Preallocate();

    ctx->out.RegisterSource(&ctx->player);
    ctx->player.RegisterSink(&ctx->out);

    ctx->pool.RegisterWork(0, &ctx->player);
    ctx->pool.RegisterWork(1, &ctx->out);
    return ctx;
}
void randomplay(playercontext* ctx, playlist* pl)
{
    Vrok::DecoderFFMPEG* dec = new Vrok::DecoderFFMPEG();
    int index = rand() % pl->list.size(); 
    Vrok::Resource* res = new Vrok::Resource { pl->list[index].url };
    printf("Playing: %s %lu\n", pl->list[index].title.c_str(), pl->list[index].seekto);
    dec->Open(res);
    dec->SetPositionInSeconds(pl->list[index].seekto);
    ctx->player.SubmitForPlayback(dec);
}
void on_event_playpause(void *user)
{
    playercontext* ctx = (playercontext*) user;
    if (ctx->ispaused)
    {
        ctx->player.Resume();
        ctx->ispaused = false;
    }
    else
    {
        ctx->player.Pause();
        ctx->ispaused = true;
    }
}
void on_event_next(void *user)
{
    playercontext* ctx = (playercontext*) user;
    ctx->player.Stop();
    ctx->player.Flush();
    randomplay(ctx, &mainpl);
}
int main(int argc, const char** argv)
{
    srand(time(NULL));
    event_callbacks events {
        .on_playpause = on_event_playpause,
        .on_next = on_event_next
    };
    const auto& ctx = plctx_create();
    mpris2_init(&events, ctx);
    ctx->pool.CreateThreads();
    enum_files(argv[1]);
    list_pl();
    randomplay(ctx, &mainpl);
    while (1)
    {
        mpris2_process_events();
    }
    ctx->pool.JoinThreads();
    return 0;
}
