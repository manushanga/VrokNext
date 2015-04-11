
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
#include "preamp.h"
#include "fir.h"
#include "threadpool.h"
#include <QDir>

#define BUF_SIZE 100


using namespace std;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

int main(int argc, char *argv[])

{
    //Test1 test;

    Vrok::Player pl;
    Vrok::DriverAudioOut out;
    Vrok::EffectFIR pre;
    ThreadPool pool(2);

    //Vrok::EffectFIR pre1;

    out.RegisterSource(&pre);
    pre.RegisterSource(&pl);
    pre.RegisterSink(&out);
    pl.RegisterSink(&pre);
    //pre1.RegisterSource(&pre);
   // pre1.RegisterSink(&out);

    out.Preallocate();
    pl.Preallocate();
    pre.Preallocate();
    //pre1.Preallocate();
    pool.RegisterWork(0,&pl);
    pool.RegisterWork(0,&pre);
    pool.RegisterWork(1,&out);

    pool.CreateThreads();

    float g=10.0,lp_freq=60.0f;
    Vrok::Component *current_comp=nullptr;

   // pre1.CreateThread();


    QString path="./";
    QFileInfoList filelist;

    while (true)
    {

        string line;
        getline(cin,line);

        QString query(line.c_str());
        QString command=query.section(' ',0,0);
        if (command.compare("openi")==0)
        {
            Vrok::Resource *res=new Vrok::Resource;
            res->_filename = filelist[query.section(' ',1,1).toInt()].absoluteFilePath().toStdString();
            pl.SubmitForPlaybackNow(res);
        }
        if (command.compare("open")==0)
        {
            Vrok::Resource *res=new Vrok::Resource;
            res->_filename = query.section(' ',1).toStdString();
            pl.SubmitForPlaybackNow(res);
        } else if (command.compare("setc")==0)
        {
            current_comp = Vrok::ComponentManager::GetSingleton()->GetComponent(
                               query.section(' ', 1).toStdString());
            if (current_comp)
            {
                DBG("component found");
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
                    DBG("set"<<pp);
                    break;
                }
                }
            } else
            {
                DBG("no property found");
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
            QString dir=query.section(' ',1);

            if (dir.compare("..")==0) {
                QDir dir(path);
                dir.cdUp();
                path = dir.absolutePath();
                filelist = dir.entryInfoList();
            }
            else if (dir.startsWith("/"))
            {
                path=dir;
            }
            else
            {
                bool ok=false;
                int index=dir.toInt(&ok);

                if (ok)
                {
                    path+="/"+filelist[index].fileName();
                }
                else
                {
                    path+=dir;
                }
            }
        }


       //cout<<res->_filename<<endl;

    }
    pool.JoinThreads();

    return 0;
}
