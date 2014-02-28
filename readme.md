Create trigger points in space. If movement detected in the area, sends out osc message to trigger sounds. For now, trigger Ableton live set.

Modified from cswell

2014-01-18 Weidong Yang

osc port is 9000
On live side, use liveOsc, need to install python 2.5.1
LiveOsc, has to install Python 2.5.1
http://livecontrol.q3f.org/ableton-liveapi/liveosc/#Installation
http://monome.q3f.org/browser/trunk/LiveOSC/OSCAPI.txt

osc msg:
/live/play/clip int_track_id int_clip_id
/live/volume int_track_id float_volume


adding ofxMidi
1. drag ofxMidi source folder to addons
2. remove unnecessary files
3. in ofApp summary tab, add CoreMIDI.framework


Control:
on midi KORG nano controler:
"cycl" for toggling loop
"<, >" for moving between sets of sound