import os, random, sys
import vrok
import time
import threading

def get_file_list(path):
    list = []
    for dp, dn, fn in os.walk(path):
        for f in fn:
            filepath = os.path.join(dp, f)
            #print(filepath)
            if os.path.isfile(filepath) and (f.endswith("mp3") or f.endswith("MP3") or f.endswith("flac")):
                list.append(filepath)
    return list

r = vrok.Resource()
files = []
for path in sys.argv:
    files.extend(get_file_list(path))

pl = vrok.Player()
fir = vrok.EffectFIR()
out = vrok.DriverAlsa()
dec = vrok.DecoderFFMPEG()

for device in out.GetDeviceInfo():
    print(device.name)

out.SetDevice(out.GetDeviceInfo()[0].name)

def worker(fir):
    meter = fir.GetMeters().__getitem__(0)
    print (meter.GetValue(0))
    while (True):
        time.sleep(0.1)
        print (meter.GetValue(0))
    print("end")

th = threading.Thread(target=worker,args=[out])

th.start()

events = vrok.PlayerEvents()
def QueueNext():
    r.filename = random.choice(files)
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

pl.RegisterSink(fir)
fir.RegisterSource(pl)
fir.RegisterSink(out)
out.RegisterSource(fir)

pl.Preallocate()
out.Preallocate()
fir.Preallocate()

out.SetVolume(0.0)
pl.SetEvents(events)
t = vrok.ThreadPool(2)

t.RegisterWork(0, pl)
t.RegisterWork(1, fir)
t.RegisterWork(1, out)

pl.SubmitForPlayback(dec)

t.CreateThreads()

t.JoinThreads()


