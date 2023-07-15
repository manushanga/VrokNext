#include "preamp.h"

#include <cmath>

vrok::EffectPreamp::EffectPreamp()
    : delay(GetBufferConfig()->channels * GetBufferConfig()->frames),
      delay1(GetBufferConfig()->channels * GetBufferConfig()->frames) { }

bool vrok::EffectPreamp::EffectRun(Buffer *out_buffer, Buffer **in_buffer_set, int buffer_count) {
    int len = GetBufferConfig()->frames * GetBufferConfig()->channels;
    delay.FillDelayed(in_buffer_set[0]->GetData(), out_buffer->GetData(), 100);
    delay1.MixDelayed(in_buffer_set[0]->GetData(), out_buffer->GetData(), 1000, 1.5);

    /*for (int i=0;i<buffer_count;i++)
    {
        for (int j=0;j<len;j++)
        {
            out_buffer->GetData()[j] += in_buffer_set[i]->GetData()[j];
            out_buffer->GetData()[j] /= 1;
        }
    }*/
    return true;
}
