NerDisco
========

Is a simple VJ tool leveraging OpenGL/GLSL to provide live-editing functions and is meant to be connected to a [Boblight](https://code.google.com/p/boblight/) display of the ["Adalight"](http://www.adafruit.com/product/461) type via a serial port. It was tested with an Arduino Pro connected to an LED strip using [LPD8806 chips](http://www.adafruit.com/product/306). The code running on the Arduino was [Adalights' LPD8806 LEDstream sketch](https://github.com/adafruit/Adalight/blob/master/Arduino/LEDstream_LPD8806/LEDstream_LPD8806.pde). The code was compiled and tested on a Windows 7 and Ubuntu 14.04 machine and may or may not work on other systems.
It is in parts inspired by the live shader-editing tool [quint](https://gitorious.org/quint) and uses the nice [RtMidi](https://github.com/thestk/rtmidi) library for MIDI controller input. So hats off to those guys...

Please note that the tool was rather quickly hacked together for a party and obviously needs some refactoring and improvements. Planned features are some audio input facilities, so e.g. the music spectrum can be used in the scripts.

License
========

[BSD-2-Clause](http://opensource.org/licenses/BSD-2-Clause), see [LICENSE.md](LICENSE.md.).  
For [RtMidi](https://github.com/thestk/rtmidi), see [the RtMidi readme](https://github.com/thestk/rtmidi/blob/master/readme).

Building
========
A .pro file for Qt Creator / QMake is provided, so you can just compile it from the Qt Creator IDE or import it into Visual Studio using the Qt addin. If you want to use the command line:

**Windows / MSVC**

Open up a Visual Studio command prompt of x86 or x64 flavor (depending on your Qt installation).
<pre>
cd NerDisco
set QTDIR="PATH_TO_QT_INSTALLATION"
set PATH=%PATH%;%QTDIR%\bin
qmake
nmake
</pre>

**Unix /GCC**
<pre>
cd NerDisco
qmake
make
</pre>

The Qt framework version 5.1 or higher is required for GUI, audio and serial port functionality. You might need to additionally install the "qtmultimedia5-dev" package for audio input support.
If NerDisco does not find any audio devices your system might lack the [Qt5 multimedia plugins](http://stackoverflow.com/questions/21939759/qaudiodeviceinfo-finds-no-default-audio-device-on-ubuntu). Install the "libqt5multimedia5-plugins" package.
[RtMidi](https://github.com/thestk/rtmidi) is used for MIDI input support (thank you!). It should come to you as an external GIT submodule in the "\rtmidi" subfolder. If it is not there, put it there...
G++ 4.7 (for C++11) might be needed to compile NerDisco. For installing G++ 4.7 see [here](http://lektiondestages.blogspot.de/2013/05/installing-and-switching-gccg-versions.html).

Overview
========
![GUI overview](NerDisco_gui.png?raw=true)

Scripts
========
The render scripts are actually GLSL 1.20 fragment shaders. Those ".fs" script files are read from the "effects" directory and should have the extension ".fs" to be found and displayed in the menu.
The dials A-C and the trigger button can be used in scripts via the float uniform variables "valueA", "valueB", "valueC" and "trigger". Dial values range from [0,1] and trigger [false,true].
Also the uniforms "vec2 renderSize" (render area pixel resolution) and "float time" (application runtime in seconds) are available. A good example is "rect.fs" in the effects sub directory.
```
#version 120

uniform vec2 renderSize;
uniform float time;
uniform float valueA;
uniform float valueB;
uniform float valueC;
uniform float trigger;

varying vec2 texcoordVar;

void main() {
	float l = texcoordVar.x > 0.2+0.1*sin(time) ? (texcoordVar.x < 0.8+0.2*sin(0.88*time+1) ? 1 : 0) : 0;
	l = texcoordVar.y > 0.2+0.1*sin(0.69*time) ? (texcoordVar.y < 0.8+0.2*sin(0.45*time) ? l : 0) : 0;
	float r = l * valueA + 0.1 * trigger;
	float g = l * valueB + 0.1 * trigger;
	float b = l * valueC + 0.1 * trigger;
	gl_FragColor = vec4(r, g, b, 1.0);
}
```

MIDI controllers
========
The dials A-C, the trigger buttons in both decks and the crossfader can be controller via MIDI controllers. NerdDisco can learn a MIDI to GUI control mapping if you select a MIDI device and start capturing from it.
Then select the "Learn MIDI->control mapping" menu entry. Turn the dial, push the trigger or move fader you want to connect, then move the physical MIDI control element. The two should be connected and the GUI should follow the MIDI control.
You can still choose a different GUI element or MIDI control until you select the menu option "Store learned connection" (to store the current connection) or leave the learn mode again via "Learn MIDI->control mapping".
Then all stored connections you have made before should work.

FAQ
========
**Q:** I'm on linux and I can not access the serial port somehow...  
**A:** You might need to add your USERNAME to the dialout group: ```sudo usermod -a -G dialout USERNAME```.  

I found a bug or have a suggestion
========

The best way to report a bug or suggest something is to post an issue on GitHub. Try to make it simple, but descriptive and add ALL the information needed to REPRODUCE the bug. **"Does not work" is not enough!** If you can not compile, please state your system, compiler version, etc! You can also contact me via email if you want to.
