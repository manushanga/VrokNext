
#include "bgpoint.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <queue.h>
#include <thread>
#include <unistd.h>

#include "test1.h"
#include "test2.h"
#include <QApplication>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>

#include "alsa.h"
#include "audioout.h"
#include "common.h"
#include "effect.h"
#include "eq.h"
#include "ffmpeg.h"
#include "fir.h"
#include "player.h"
#include "preamp.h"
#include "threadpool.h"
#include <QDir>

#include "disp.h"

#define BUF_SIZE 100

using namespace std;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
struct CBData {
    vrok::Player *player;
    vrok::DriverAlsa *pOut;
    vrok::EffectSSEQ *pSSEQ;
    vrok::EffectFIR *pFIR;
    QFileInfoList *list;
};

class PlayerEvents : public vrok::Player::Events {
public:
    CBData data;
    void QueueNext() {
        vrok::Resource *res = new vrok::Resource();
        vrok::Decoder *decoder = new vrok::DecoderFFMPEG();
        std::vector<std::string> files;
        foreach (QFileInfo f, *(data.list)) {
            if (f.isFile()) {
                files.push_back(f.absoluteFilePath().toStdString());
            }
        }
        if (data.list->size()) {
            std::cout << "-----------" << std::endl;

            res->_filename = files[rand() % files.size()];
            DBG(0, res->_filename);

            if (decoder->Open(res)) {
                data.pOut->Flush();
                data.pSSEQ->Flush();
                data.player->Flush();
                data.pFIR->Flush();
                data.player->SubmitForPlayback(decoder);
            }
        }
    }
};

class CNotifier : public vrok::Notify::Notifier {
public:
    void OnError(int level, std::string msg) {
        _guard.lock();
        std::cout << "ERR: " << msg << std::endl;
        _guard.unlock();
    }
    void OnWarning(int level, std::string msg) {
        _guard.lock();
        std::cout << "WRN: " << msg << std::endl;
        _guard.unlock();
    }
    void OnDebug(int level, std::string msg) {
        _guard.lock();
        std::cout << "DBG: " << msg << std::endl;
        _guard.unlock();
    }
    void OnInformation(std::string msg) {
        _guard.lock();
        std::cout << "INF: " << msg << std::endl;
        _guard.unlock();
    }

private:
    std::mutex _guard;
};
vrok::Player *pl;
vrok::DriverAlsa *out;
vrok::EffectSSEQ *pSSEQ;
vrok::EffectFIR *pFIR;
vrok::Component *current_comp = nullptr;
QString path = "./";
QFileInfoList filelist;

void process(QString query) {

    QString command = query.section(' ', 0, 0);
    bool got_track = false;
    int track_id = query.toInt(&got_track);

    if (got_track) {
        if (track_id < filelist.size()) {
            vrok::Resource *res = new vrok::Resource;
            res->_filename = filelist[track_id].absoluteFilePath().toStdString();

            pl->Flush();
            out->Flush();
            pSSEQ->Flush();
            pFIR->Flush();
            vrok::Decoder *decoder = new vrok::DecoderFFMPEG();
            if (decoder->Open(res))
                pl->SubmitForPlayback(decoder);
        } else {
            WARN(9, "invalid index");
        }
    } else if (command.compare("pause") == 0 || command.compare("p") == 0) {
        pl->Pause();
    } else if (command.compare("resume") == 0 || command.compare("r") == 0) {
        pl->Resume();
    } else if (command.compare("openi") == 0) {
        track_id = query.section(' ', 1, 1).toInt();
        if (track_id < filelist.size()) {

            vrok::Resource *res = new vrok::Resource;
            res->_filename = filelist[track_id].absoluteFilePath().toStdString();

            vrok::Decoder *decoder = new vrok::DecoderFFMPEG();
            if (decoder->Open(res))
                pl->SubmitForPlayback(decoder);

        } else {

            WARN(9, "invalid index");
        }

    } else if (command.compare("open") == 0) {
        vrok::Resource *res = new vrok::Resource;
        res->_filename = query.section(' ', 1).toStdString();

        vrok::Decoder *decoder = new vrok::DecoderFFMPEG();
        if (decoder->Open(res))
            pl->SubmitForPlayback(decoder);

    } else if (command.compare("setc") == 0) {
        current_comp =
                vrok::ComponentManager::GetSingleton()->GetComponent(query.section(' ', 1).toStdString());
        if (!current_comp) {
            WARN(9, "component not found!");
        }
    } else if (command.compare("setp") == 0) {
        string prop = query.section(' ', 1, 1).toStdString();
        vrok::PropertyBase *p = vrok::ComponentManager::GetSingleton()->GetProperty(current_comp, prop);
        if (p) {
            vrok::ComponentManager::GetSingleton()->SetProperty(current_comp, p,
                                                                query.section(' ', 2, 2).toStdString());
        } else {
            WARN(9, "no property found");
        }

    } else if (command.compare("ls") == 0) {
        QDir dir;
        dir.setPath(path);
        filelist = dir.entryInfoList();
        int i = 0;
        foreach (QFileInfo file, filelist) {
            cout << i << ". " << file.fileName().toStdString() << endl;
            i++;
        }

    } else if (command.compare("cd") == 0) {
        QString dir_name = query.section(' ', 1);

        if (dir_name.compare("..") == 0) {
            QDir dir(path);
            dir.cdUp();
            path = dir.absolutePath();
            filelist = dir.entryInfoList();
        } else if (dir_name.startsWith("/") || dir_name[1] == ':') {
            path = dir_name;
            QDir dir(path);
            filelist = dir.entryInfoList();
        } else {
            bool ok = false;
            int index = dir_name.toInt(&ok);

            if (ok) {
                path += "/" + filelist[index].fileName();
            } else {
                path += "/" + dir_name;
            }
            QDir dir(path);
            filelist = dir.entryInfoList();
        }
    } else if (command.compare("s") == 0) {
        pl->Skip();
    } else if (command.compare("vol") == 0) {
        auto vol = query.section(' ', 1).toDouble();
        out->SetVolume(vol);
    } else if (command.compare("q") == 0) {
        exit(0);
    } else {
        WARN(0, "unknown command");
    }
}

int main(int argc, char *argv[])

{
    CNotifier notifier;
    vrok::Notify::GetInstance()->SetNotifier(&notifier);

    pl = new vrok::Player;
    out = new vrok::DriverAlsa;
    pSSEQ = new vrok::EffectSSEQ;
    pFIR = new vrok::EffectFIR;

    // Test1 test;
    srand(time(NULL));

    vrok::ThreadPool pool(2);

    // Vrok::EffectFIR pre1;
    pFIR->RegisterSource(pSSEQ);
    pFIR->RegisterSink(out);

    pSSEQ->RegisterSource(pl);
    pSSEQ->RegisterSink(pFIR);

    pl->RegisterSink(pSSEQ);

    out->RegisterSource(pFIR);

    out->Preallocate();
    pl->Preallocate();
    pSSEQ->Preallocate();
    pFIR->Preallocate();

    pool.RegisterWork(0, pl);
    pool.RegisterWork(1, pSSEQ);
    pool.RegisterWork(0, pFIR);
    pool.RegisterWork(1, out);

    CBData data;
    data.player = pl;
    data.pFIR = pFIR;
    data.pOut = out;
    data.pSSEQ = pSSEQ;
    data.list = &filelist;
    PlayerEvents events;
    events.data = data;
    pl->SetEvents(&events);
    auto vec = out->GetDeviceInfo();
    for (int i = 0; i < vec.size(); i++) {
        std::cout << vec[i].name << std::endl;
    }
    out->SetDevice(vec[0].name);
    if (argc > 1) {
        path = QString(argv[1]);
        QDir dir(path);
        filelist = dir.entryInfoList();
    }

    QFile inputFile(QStandardPaths::locate(QStandardPaths::ConfigLocation, "/MXE/default.vsh"));
    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        while (!in.atEnd()) {

            QString line = in.readLine();
            DBG(0, line.toStdString());
            process(line);
        }
        inputFile.close();
    };
    pool.CreateThreads();

    QApplication a(argc, argv);

    Disp disp(pFIR->GetMeters()[0]);
    disp.show();

    Disp disp1(out->GetMeters()[0]);
    disp1.show();
    a.exec();

    pool.JoinThreads();

    /*while (true)
    {

        string line;
        getline(cin,line);

        process(line.c_str());

       //cout<<res->_filename<<endl;

    }*/
    return 0;
}
