
print("sss")
import os, random, sys
import vrok
import time
import threading
import numpy
import pyaudio as pa
import mpris_server as ms
import cuesheet as cs
from mpris_server.adapters import MprisAdapter
from mpris_server.events import EventAdapter
from mpris_server.server import Server
from mpris_server.base import PlayState
from mpris_server.base import Metadata
from gi.repository import GLib as glib

def get_file_list(path):
    list = []
    for dp, dn, fn in os.walk(path):
        for f in fn:
            filepath = os.path.join(dp, f)
            #print(filepath)
            if os.path.isfile(filepath):
                if (f.endswith("mp3") or f.endswith("MP3") or f.endswith("flac") or f.endswith('opus') or f.endswith('m4a')):
                    list.append(filepath)
    return list
#p = pa.PyAudio()
#s = p.open(format=pa.paFloat32, channels=1, rate=48000, output=True)

print("sasss")
r = vrok.Resource()
files = []
cue_sheet = None
cue_file = None
if sys.argv[1].endswith(".cue"):
    cue = cs.CueSheet()
    cue.setOutputFormat("%file%", "%title%")
    cue.setData(open(sys.argv[1], "r").read())
    cue.parse()
    cue_sheet = []
    cue_file = os.path.dirname(sys.argv[1]) + "/" + cue.file
    for d in cue.tracks:
        cue_sheet.append(int(cs.offsetToTimedelta(d.offset).total_seconds()))
else:
    for path in sys.argv:
        files.extend(get_file_list(path))

pl = vrok.Player()
fir = vrok.EffectSSEQ()
#out = vrok.DriverAlsa()
out = vrok.DriverPulse()
#outJ = vrok.DriverJBufferOut();
#outPy = vrok.DriverPyOut();
dec = vrok.DecoderFFMPEG.Create()
#def PyOutOnBuffer(ndarray):
#    data = ndarray[0].astype(numpy.float32)

    
    #ff.write(data.copy().tobytes())
#    s.write(data.tobytes())
    

#def PyOutOnBufferConfigChange(frames, samplerate, channels):
#    global s
    
#    print(frames, samplerate, channels)
#    s.close()
    
#    s = p.open(format=pa.paFloat32, channels=1, rate=samplerate, output=True)
    
#outPy.OnBuffer = PyOutOnBuffer
#outPy.OnBufferConfigChange = PyOutOnBufferConfigChange

#outJ.SetEvents(outPy)

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

def QueueNextCueSheet():
    r.filename = cue_file
    offset = random.choice(cue_sheet)
    dec = vrok.DecoderFFMPEG.Create()
    print(r.filename)
    print(dec.Open(r))
    print("00000ss")
    dec.SetPositionInSeconds(offset)
    pl.SubmitForPlayback(dec)
def QueueNextCueSheetNow():
    r.filename = cue_file
    offset = random.choice(cue_sheet)
    dec = vrok.DecoderFFMPEG.Create()
    print(r.filename)
    print(dec.Open(r))
    print("00000ss")
    dec.SetPositionInSeconds(offset)
    pl.SubmitForPlaybackNow(dec)


def QueueNext():
    r.filename = random.choice(files)
    dec = vrok.DecoderFFMPEG.Create()
    print(r.filename)
    print(dec.Open(r))
    print("00000ss")
    dec.SetPositionInSeconds(100)
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

if not cue_sheet is None:
    events.QueueNext = QueueNextCueSheet
    r.filename = cue_file
else:
    events.QueueNext = QueueNext
    r.filename = random.choice(files)

pl.SetQueueNext(True)
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

dec.SetPositionInSeconds(100)
pl.SubmitForPlayback(dec)


class MyAppAdapter(MprisAdapter):
    # Make sure to implement all methods on MprisAdapter, not just metadata()
    def metadata(self) -> Metadata :
        return dict()
    def get_playstate(self):
        return PlayState.PLAYING
    # and so on


class MyAppEventHandler(EventAdapter):
    # EventAdapter has good default implementations for its methods.
    # Only override the default methods if it suits your app.

    def on_app_event(self, event: str):
        # trigger DBus updates based on events in your app
        if event == 'pause':
            self.on_playpause()
        pass
    # and so on

# create mpris adapter and initialize mpris server
my_adapter = MyAppAdapter()
mpris = Server('MyApp', adapter=my_adapter)
#mpris.publish()
#mpris.loop()
# initialize app integration with mpris
#event_handler = MyAppEventHandler(root=mpris.root, player=mpris.player)
#ml = glib.MainLoop()
#mc = ml.get_context()
#ml.run()
t.CreateThreads()
# publish and serve
while True:
    #mc.iteration(False)
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
    elif len(nn.split())==2 and nn.split()[0] == 'setp':
        compman.SetProperty("ShibatchSuperEQ:0", "Preamp", nn.split()[1])
    elif len(nn.split())==3 and nn.split()[0] == 'setb':
        print("ss")        
        xx = "Band%02d"%(int(nn.split()[1]))
        print(xx)
        compman.SetProperty("ShibatchSuperEQ:0", xx, nn.split()[2])
    elif nn == '':
        if not cue_sheet is None:
            QueueNextCueSheetNow()
        else:
            QueueNextNow()
    else:
        QueueFile(nn)
t.JoinThreads()


