
print("sss")
import os, random, sys
import vrok
import time
import threading
import numpy
import pyaudio as pa
import mpris_server as ms

def get_file_list(path):
    list = []
    for dp, dn, fn in os.walk(path):
        for f in fn:
            filepath = os.path.join(dp, f)
            #print(filepath)
            if os.path.isfile(filepath) and (f.endswith("mp3") or f.endswith("MP3") or f.endswith("flac") or f.endswith('webm')):
                list.append(filepath)
    return list
p = pa.PyAudio()
s = p.open(format=pa.paFloat32, channels=1, rate=48000, output=True)

print("sasss")
r = vrok.Resource()
files = []
for path in sys.argv:
    files.extend(get_file_list(path))

pl = vrok.Player()
fir = vrok.EffectSSEQ()
out = vrok.DriverAlsa()
outJ = vrok.DriverJBufferOut();
outPy = vrok.DriverPyOut();
dec = vrok.DecoderFFMPEG.Create()
def PyOutOnBuffer(ndarray):
    data = ndarray[0].astype(numpy.float32)

    
    #ff.write(data.copy().tobytes())
    s.write(data.tobytes())
    

def PyOutOnBufferConfigChange(frames, samplerate, channels):
    global s
    
    print(frames, samplerate, channels)
    s.close()
    
    s = p.open(format=pa.paFloat32, channels=1, rate=samplerate, output=True)
    
outPy.OnBuffer = PyOutOnBuffer
outPy.OnBufferConfigChange = PyOutOnBufferConfigChange

outJ.SetEvents(outPy)

for device in out.GetDeviceInfo():
    print(device.name)

#for device in outJ.GetDeviceInfo():
#    print(device.name)

out.SetDevice(out.GetDeviceInfo()[0].name)

#def worker(fir):
#    meter = fir.GetMeters().__getitem__(0)
#    print (meter.GetValue(0))
#    while (True):
#        time.sleep(0.1)
#        #print (meter.GetValue(0))
#    print("end")

#th = threading.Thread(target=worker,args=[out])

#th.start()

events = vrok.PlayerEvents()
def QueueNext():
    r.filename = random.choice(files)
    dec = vrok.DecoderFFMPEG.Create()
    print(r.filename)
    print(dec.Open(r))
    pl.SubmitForPlayback(dec)
def QueueNextNow():
    r.filename = random.choice(files)
    dec = vrok.DecoderFFMPEG.Create()
    print(r.filename)
    print(dec.Open(r))
    pl.SubmitForPlaybackNow(dec)



def QueueFile(ffile):
    
    r = vrok.Resource()
    r.filename = ffile
    dec = vrok.DecoderFFMPEG.Create()
    print(r.filename)
    print(dec.Open(r))
    pl.SubmitForPlaybackNow(dec)


    
events.QueueNext = QueueNext
pl.SetQueueNext(True)
r.filename = random.choice(files)
print(r.filename)
compman = vrok.ComponentManager.GetSingleton()
comp = compman.GetComponent("SSEQ:0");
if (comp != None):
    print("FIRRRR")
    
print(dec.Open(r))

outX = out
pl.RegisterSink(outX)
fir.RegisterSource(pl)
fir.RegisterSink(outX)
outX.RegisterSource(fir)

pl.Preallocate()
outX.Preallocate()
fir.Preallocate()

outX.SetVolume(0.0)
pl.SetEvents(events)
t = vrok.ThreadPool(2)

t.RegisterWork(1, pl)
t.RegisterWork(0, fir)
t.RegisterWork(0, outX)

pl.SubmitForPlayback(dec)

t.CreateThreads()

while True:
    nn = sys.stdin.readline().strip()
    if nn == 'q':        
        t.StopThreads()
        break
    elif nn == 'pp':
        pl.Resume()
    elif nn == 'p':
        pl.Pause()
    elif nn == 'listeq':
        for i in range(0, 17):
            xx = "Band%02d"%(i)
            print(compman.GetProperty("ShibatchSuperEQ:0", xx).Get())
    elif len(nn.split())==3 and nn.split()[0] == 'setb':
        print("ss")        
        xx = "Band%02d"%(int(nn.split()[1]))
        print(xx)
        compman.SetProperty("ShibatchSuperEQ:0", xx, nn.split()[2])
    elif nn == '':
        QueueNextNow()
    else:
        QueueFile(nn)
t.JoinThreads()


