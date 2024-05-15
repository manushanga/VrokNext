#include "test1.h"

Test1::Test1() {
    a.RegisterSink(&b);
    b.RegisterSource(&a);

    a.Preallocate();
    b.Preallocate();
}

Test1::~Test1() { }

void NodeA::Run()

{

    int j = 0;
    while (true) {

        auto b = (AcquireBuffer());

        auto xx = GetBufferConfig();

        // DBG(::this_thread::get_id()<<" w"<<b);
        /*
                for (int x=0;x<xx.frames*xx.channels;x++)
                {
                    b->GetData()[x]=-999999999990;

                }*/
        b->GetData()[1] = j;

        j++;

        PushBuffer(b);
    }
}

void NodeB::Run() {
    int j = 0;
    while (true) {

        auto bb = PeakAllSources();
        // DBG(::this_thread::get_id()<<" "<<bb[0]->GetData()[1]);

        if (bb[0]->GetData()[1] != j) {
            std::cout << "fail" << endl;
            while (1) {
            }
        }
        j = bb[0]->GetData()[1] + 1;
        ReleaseAllSources(bb);
    }
}
