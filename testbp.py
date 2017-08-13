import os, random, sys
import vrok
import time
import threading
import numpy
import pyaudio as pa

def get_file_list(path):
    list = []
    for dp, dn, fn in os.walk(path):
        for f in fn:
            filepath = os.path.join(dp, f)
            #print(filepath)
            if os.path.isfile(filepath) and (f.endswith("mp3") or f.endswith("MP3") or f.endswith("flac")):
                list.append(filepath)
    return list
p = pa.PyAudio()
s = p.open(format=pa.paFloat32, channels=1, rate=48000, output=True)


r = vrok.Resource()
files = []
for path in sys.argv:
    files.extend(get_file_list(path))

pl = vrok.Player()
fir = vrok.EffectFIR()
out = vrok.DriverAlsa()
outJ = vrok.DriverJBufferOut();
outPy = vrok.DriverPyOut();
dec = vrok.DecoderFFMPEG.Create()
ff = open('test','bw+')
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

for device in outJ.GetDeviceInfo():
    print(device.name)

out.SetDevice(out.GetDeviceInfo()[2].name)

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
    
events.QueueNext = QueueNext

r.filename = random.choice(files)
print(r.filename)
compman = vrok.ComponentManager.GetSingleton()
comp = compman.GetComponent("FIR filter:0");
if (comp != None):
    prop = compman.GetProperty(comp,"wet_vol");
    compman.SetProperty(comp,prop, "2.0"); 
    
print(dec.Open(r))

outX = outJ
pl.RegisterSink(outX)
outX.RegisterSource(pl)

pl.Preallocate()
outX.Preallocate()

outX.SetVolume(0.0)
pl.SetEvents(events)
t = vrok.ThreadPool(2)

t.RegisterWork(1, pl)
#t.RegisterWork(0, fir)
t.RegisterWork(0, outX)

pl.SubmitForPlayback(dec)

t.CreateThreads()

t.JoinThreads()


