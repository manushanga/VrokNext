import vrok
import time
import threading

r = vrok.Resource()

pl = vrok.Player()
fir = vrok.EffectFIR()
out = vrok.DriverAlsa()
dec = vrok.DecoderFFMPEG()

for device in out.GetDeviceInfo():
    print(device.name)


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
    r.filename = "/home/madura/Downloads/Adarayada Me - W.D. Amaradewa ( Official HD Music Video ).mp3"
    print(dec.Open(r))
    pl.SubmitForPlayback(dec)
    
events.QueueNext = QueueNext

r.filename = "/home/madura/Downloads/Adarayada Me - W.D. Amaradewa ( Official HD Music Video ).mp3"
#r.filename = "https://r2---sn-nau-jhcs.googlevideo.com/videoplayback?upn=pgdMfggesK0&expire=1451313458&mm=31&source=youtube&mn=sn-nau-jhcs&mt=1451291823&mv=m&ms=au&fexp=9405348%2C9407169%2C9416126%2C9420452%2C9422596%2C9423236%2C9423662%2C9423715%2C9424114%2C9424822%2C9424988%2C9425222%2C9425362%2C9425637%2C9425978&pl=22&dur=219.057&requiressl=yes&ipbits=0&mime=video%2F3gpp&ip=112.134.98.113&key=yt6&itag=17&id=o-AFxd4Qd10FJbNeeG6rZnLGYD6Dlo2R3_uM1eVvr8Xy4W&initcwndbps=617500&lmt=1394294361048236&signature=809DA56792D8FC26A8FD7AE6D72B6414BDAC4F7E.512E6620600E67592BAF09153BA8ABC7305024D6&sver=3&sparams=dur%2Cid%2Cinitcwndbps%2Cip%2Cipbits%2Citag%2Clmt%2Cmime%2Cmm%2Cmn%2Cms%2Cmv%2Cpl%2Crequiressl%2Csource%2Cupn%2Cexpire&ratebypass=yes"
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

pl.SetEvents(events)
t = vrok.ThreadPool(2)

t.RegisterWork(0, pl)
t.RegisterWork(1, fir)
t.RegisterWork(1, out)

pl.SubmitForPlayback(dec)

t.CreateThreads()

t.JoinThreads()


