import threading
import sys

from ctypes import *
lib = cdll.LoadLibrary('./libvrok.so')
lib.CreatePlayer.argtypes = []
lib.CreatePlayer.restypes = [c_void_p]

lib.CreateResource.argtypes = [c_char_p]
lib.CreateResource.restype = c_void_p

lib.SubmitForPlayback.argtypes = [c_void_p]
lib.SubmitForPlayback.restype = None

lib.SubmitForPlaybackNow.argtypes = [c_void_p]
lib.SubmitForPlaybackNow.restype = None

player=None

def worker():
    player=lib.CreatePlayer()
    lib.JoinPlayer();

th=threading.Thread(target=worker)
th.start()

while (True):
    filename=sys.stdin.readline()[:-1]
    c_filename=create_string_buffer(filename.encode('utf-8'))
    o=lib.CreateResource(c_filename);
    lib.SubmitForPlaybackNow(o)
