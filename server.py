from flask import Flask, jsonify, request, render_template
app = Flask(__name__)

import os, random, sys
import vrok
import time
import threading
import re

max_matches = 20

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
dec = vrok.DecoderFFMPEG.Create()

for device in out.GetDeviceInfo():
    print(device.name)

out.SetDevice(out.GetDeviceInfo()[0].name)

def worker(fir):
    meter = fir.GetMeters().__getitem__(0)
    print (meter.GetValue(0))
    while (True):
        time.sleep(2)
        print (meter.GetValue(0))
    print("end")

th = threading.Thread(target=worker,args=[out])

th.start()

events = vrok.PlayerEvents()
def QueueNext():
    r.filename = random.choice(files)
    print(r.filename)
    decNew = vrok.DecoderFFMPEG.Create()
    decNew.Open(r)
    pl.SubmitForPlayback(decNew)
    
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

out.SetVolume(-10.0)
pl.SetEvents(events)
t = vrok.ThreadPool(2)

t.RegisterWork(0, pl)
t.RegisterWork(0, fir)
t.RegisterWork(1, out)


t.CreateThreads()

pl.SubmitForPlayback(dec)
@app.route("/play/<int:index>")
def play(index):
    print("---track change ---")
    r.filename = files[index]
    print(files[index])
    
    decNew = vrok.DecoderFFMPEG.Create()
    decNew.Open(r)
    pl.SubmitForPlayback(decNew)

    pl.Flush()
    fir.Flush()
    out.Flush()
    
@app.route("/find")
def find():
    match = request.args.get('match', 0, type=str)
    i = 0
    j = 0
    results = []
    regx = re.compile(match, re.IGNORECASE)
    while i< len(files):
        file = files[i]
        if file.find(match) != -1:
            row = {}
            row["index"] = i
            row["file"] = file
            results.append(row)
            j+=1
        if j>= max_matches:
            break
        i+=1
    return jsonify(result=results)
    

@app.route("/")
def hello():
    return render_template('index.html')

if __name__ == "__main__":
    app.run(host='0.0.0.0')





t.JoinThreads()

