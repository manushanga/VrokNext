
#include <queue.h>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include "bgpoint.h"
#include <atomic>

#include "test1.h"
#include "test2.h"

#include "common.h"
#include "player.h"
#include "audioout.h"
#include "alsa.h"
#include "preamp.h"
#include "eq.h"
#include "fir.h"
#include "threadpool.h"
#include <QDir>

#define BUF_SIZE 100


using namespace std;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::high_resolution_clock;
struct CBData
{
    Vrok::Player *player;
    Vrok::DriverAlsa *pOut;
    Vrok::EffectSSEQ *pSSEQ;
    Vrok::EffectFIR *pFIR;
    QFileInfoList *list;
};
void NextTrack(void *user)
{
    CBData *data= (CBData *) user;
    Vrok::Resource *res=new Vrok::Resource;
    if (data->list->size())
    {
        res->_filename = (*data->list)[rand() % data->list->size()].absoluteFilePath().toStdString();
        data->player->Flush();
        data->pFIR->Flush();
        data->pOut->Flush();
        data->pSSEQ->Flush();
        data->player->SubmitForPlaybackNow(res);
    }
}
int main(int argc, char *argv[])

{
    Vrok::Player *pl=new Vrok::Player;
    Vrok::DriverAlsa *out= new Vrok::DriverAlsa;
    Vrok::EffectSSEQ *pSSEQ = new Vrok::EffectSSEQ;
    Vrok::EffectFIR *pFIR= new Vrok::EffectFIR;


    //Test1 test;
    srand(time(NULL));
    QString path="./";
    QFileInfoList filelist;

    ThreadPool pool(2);

    //Vrok::EffectFIR pre1;
    DBG("player");
    pFIR->RegisterSource(pl);
    pFIR->RegisterSink(pSSEQ);

    pSSEQ->RegisterSource(pFIR);
    pSSEQ->RegisterSink(out);

    pl->RegisterSink(pFIR);

    out->RegisterSource(pSSEQ);

    DBG("Reg");

    out->Preallocate();
    pl->Preallocate();
    pSSEQ->Preallocate();
    pFIR->Preallocate();


    DBG("pre");
    pool.RegisterWork(0,pl);
    pool.RegisterWork(1,pSSEQ);
    pool.RegisterWork(0,pFIR);
    pool.RegisterWork(1,out);


    CBData data;
    data.player = pl;
    data.pFIR = pFIR;
    data.pOut = out;
    data.pSSEQ = pSSEQ;
    data.list = &filelist;

    if (argc > 1)
    {
        path = QString(argv[1]);
        QDir dir(path);
        filelist = dir.entryInfoList();
    }

    pl->SetNextTrackCallback(NextTrack, &data);


    pool.CreateThreads();
    Vrok::Component *current_comp=nullptr;


    while (true)
    {

        string line;
        getline(cin,line);

        QString query(line.c_str());
        QString command=query.section(' ',0,0);
        bool got_track=false;
        int track_id=query.toInt(&got_track);


        if (got_track)
        {
            if (track_id < filelist.size())
            {
                Vrok::Resource *res=new Vrok::Resource;
                res->_filename = filelist[track_id].absoluteFilePath().toStdString();
                pl->Flush();
                out->Flush();
                pSSEQ->Flush();
                pFIR->Flush();
                pl->SubmitForPlaybackNow(res);
            } else
            {
                WARN("invalid index");
            }
        } else if (command.compare("pause")==0 || command.compare("p")==0)
        {
            pl->Pause();
        } else if (command.compare("resume")==0 || command.compare("r")==0)
        {
            pl->Resume();
        } else if (command.compare("openi")==0)
        {
            track_id=query.section(' ',1,1).toInt();
            if (track_id < filelist.size())
            {

                Vrok::Resource *res=new Vrok::Resource;
                res->_filename = filelist[track_id].absoluteFilePath().toStdString();

                pl->SubmitForPlaybackNow(res);

            } else
            {

                WARN("invalid index");
            }

        } else if (command.compare("open")==0)
        {
            Vrok::Resource *res=new Vrok::Resource;
            res->_filename = query.section(' ',1).toStdString();
            pl->SubmitForPlaybackNow(res);
        } else if (command.compare("setc")==0)
        {
            current_comp = Vrok::ComponentManager::GetSingleton()->GetComponent(
                               query.section(' ', 1).toStdString());
            if (!current_comp)
            {
                WARN("component not found!");
            }
        } else if (command.compare("setp")==0)
        {
            string prop=query.section(' ',1,1).toStdString();
            Vrok::PropertyBase *p= Vrok::ComponentManager::GetSingleton()->GetProperty(current_comp,
                                                                prop);
            if (p)
            {

                Vrok::PropertyType ct= p->GetType();
                switch(ct)
                {
                case Vrok::PropertyType::FLT:
                {
                    float pp=query.section(' ',2,2).toFloat();
                    Vrok::ComponentManager::GetSingleton()->SetProperty(current_comp,p,&pp);
                    break;
                }
                default:
                {
                    WARN("Invalid type");
                }
                }
            } else
            {
                WARN("no property found");
            }

        } else if (command.compare("ls")==0)
        {
            QDir dir;
            dir.setPath(path);
            filelist = dir.entryInfoList();
            int i=0;
            foreach (QFileInfo file,filelist)
            {
                cout<<i<<". "<<file.fileName().toStdString()<<endl;
                i++;
            }

        } else if (command.compare("cd")==0)
        {
            QString dir_name=query.section(' ',1);

            if (dir_name.compare("..")==0) {
                QDir dir(path);
                dir.cdUp();
                path = dir.absolutePath();
                filelist = dir.entryInfoList();
            }
            else if (dir_name.startsWith("/") || dir_name[1] == ':')
            {
                path=dir_name;
                QDir dir(path);
                filelist = dir.entryInfoList();
            }
            else
            {
                bool ok=false;
                int index=dir_name.toInt(&ok);

                if (ok)
                {
                    path+="/"+filelist[index].fileName();
                }
                else
                {
                    path+="/"+dir_name;
                }
                DBG(path.toStdString());
                QDir dir(path);
                filelist = dir.entryInfoList();
            }
        }


       //cout<<res->_filename<<endl;

    }
    pool.JoinThreads();

    return 0;
}
