import hid
import time

vid = 0x727
pid = 0xa55
with hid.Device(vid, pid) as h:
    while True:
        data = h.read(65)
        print(f"{data}")
