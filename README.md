
# FragM
This is derived from Mikael Hvidtfeldt Christensen's Fragmentarium representing a compilation of features and fixes contributed by many users at FractalForums.com over 3-4 years.

Continuous integration
----------------------
[![Release][release-image]][releases]
[![License][license-image]][license]

[release-image]: https://img.shields.io/badge/release-2.5.0-green.svg?style=flat
[releases]: https://github.com/3Dickulus/FragM/releases

[license-image]: https://img.shields.io/badge/license-GPL3-green.svg?style=flat
[license]: https://github.com/3Dickulus/FragM/blob/master/LICENSE

[![Build Status](https://travis-ci.org/3Dickulus/FragM.svg?branch=master)](https://travis-ci.org/3Dickulus/FragM)
[![Coverity Status][coverity-image]][coverity-scan]

[coverity-image]: https://scan.coverity.com/projects/14365/badge.svg?flat=1
[coverity-scan]: https://scan.coverity.com/projects/3dickulus-fragm
 
# Changes since v0.9.5
+ Stops playing/rendering at the end of animation
+ Saves render "frames per second" as app setting so it gets set where you left it from the last run
+ Time represented as frame ticks rather than floating point seconds
+ Setting and saving camera spline path controlpoints as "KeyFrame" presets. Menu item "Edit->Insert Command->Presets->Insert Preset From Current Settings" or hotkey "F8" (see tutorial 30)
+ Tests for user added AutoFocus checkbox in Raytracer tab, focalplane tracks target when enabled (see tutorial 31)
+ Setting and saving In/Out easing curves for any/all single "Float" parameter Menu item "Edit->Add Easing Curve" or hotkey F7 (see tutorial 32)
+ Shows spline paths and controlpoints when in Progressive/Stop mode (still not quite right but definitely helps me as an animator/artist)
+ Toggle setting for spline rendering in case your projection or engine doesn't display them properly
+ Camera follows spline path when in Animation/Play mode
+ Camera follows spline path when in Animation/Stop mode when the time slider is moved
+ When in Progressive/Stop mode the "view vector" connects points between the Eye and Target paths relative to frame number when the time slider is moved
+ Displays current rendered frame when saving animation to disk so that if something isn't set right or didn't hookup the way you expected you can stop that 45000 frame animation and fix it without waiting until it's finished 8 hours later (depending on your hardware)
+ Ticks on TimeSlider representing keyframes
+ Saves current animation FPS and duration with default preferences settings on close
+ Syntax Highlighter fixed?
+ Added braket highlighting
+ Animation switches from play to stop at the end
+ glu library dependancy removed
+ Uses QMatrix4x4 perspective and lookat
+ Now Float 1 -2 -3 -4 widgets can have easing curves
+ Easing curves can loop multiple times
+ Easing curve loops can ping pong
+ Tabs remember thier fragment and settings when switching
+ Calls for QTextEdit replaced with Fragmentarium::Gui::TextEdit() (our class)
+ Reworked the OutputDialog class so it uses a Ui file and now it remembers last state and allows for rendering a subset of total frames
+ Reworked the Preferences Dialog class so it uses a Ui file
+ Added line numbers to editor and preferences flag to turn on/off
+ Version sensitive GLSL source highlighter 1.0 - 4.4 defaults to 1.1
+ Using glsl.xml from http://renderingpipeline.com/2013/12/glsl-syntax-highlighting-for-opengl-4-4/ 
+ Find text function Ctrl+F
+ Spline paths occluded by object (lin & win)
+ Added GPU asm browser displays vertex and fragment asm code of fragshader and buffershader
+ if no nVidia card is found the Shader Asm menu item is removed
+ now can save only Easing Curve Settings in preset named "Range-nnnn-nnnn"
+ Preferences option to save easing curve settings with named preset
+ when a preset keyframe is selected the coresponding text and controlpoint is highlighted
+ F9 will blockmark the currently selected keyframe (F8 will replace it with the current settings)
+ [spacebar] toggles animation play/stop
+ added preferences option for infinite loop play
+ added preferences option for text editor stylesheet
+ Removed platform build folders, Using QtCreator to load the Fragmentarium.pro file should work on all platforms, CmakeLists.txt should also work on all platforms. Input and tweaks are welcomed
+ added scripting abilities
+ fixed the input widgets allowing user to type input
+ increased the decimal places for input boxes from 5 to 9
+ implemented OpenEXR RGBA16F image output for v1.0.10
+ added OpenEXR RGBA16F textures for Sampler2D v1.0.10.
+ can save depth values to alpha channel in OpenEXR image output
+ added preferences option to save easing curve settings when creating named preset
+ reworked easing curve settings, no "///" preprocessor flag required
+ when a preset name begins with "Range" it will contain only currently active easing curve settings
+ Anisotropic filtering for textures
+ Mipmaps for textures
+ mklinux.sh and mkmingw.bat will compile OpenEXR static libs and Fragmentarium and install exe + Examples/* + Misc/* to a working folder
+ mklinux.sh should also work for OSX ( thanks visual.bermarte @ fractalforums.com :D and valera-rozuvan@FF )
+ Tool bars remain as user set them when switching to/from fullscreen
+ Textures are persistent when switching to/from fullscreen and when switching textures
+ LoopPlay only loops in "Play" mode
+ Fixed the timeSlider max value to keep in sync with framerate/duration
+ EXR textures use RGBAF format
+ Context menu available in full screen mode (Shift+RMB)
+ Adding language translations.
+ All motion actions are now scaled to step size, set with shift->wheel or keypad 1-2-3, 5 resets to default
+ fixed Easing Curve Ping Pong Loop bug

# These came about after collaborating with Matt Benesi
+ Added special "feedback array" uniform so frag code can get mouse click locations
+ enabled screenTo3D function, read out object XYZ @ mouse pos in status bar tnx M Benesi
+ enabled MiddleButton (Wheel) click = center view at this location
+ Shift+Tilde key resets camera to look at origin 0,0,0
+ Shift+Alt+LMB rotates around target
+ CTRL+LMB record mouse pos and increment feedback array counter
+ CTRL+Alt+LMB record mouse pos to feedback array
+ CTRL+SHIFT+LMB erase last mouse pos from feedback array and decrement counter
+ added zappa toolbar when all the fedback vars are detected
+ added zaplock, when checked normal controls are locked out and zap control is enabled
+ added zap index selection so you can select which zap gets affected by mouse and wheel
+ added Zap Clr button to reset/clear the feedback arrays

# Most recent
+ CTRL+MMB sets light pos when using DE-Kn2.frag (uniform vec3 LightPos in frag)
+ fixed Texture Persistence bug (again?)
+ Qt5.4 loads EXR format but doesn't save this format so still need OpenEXR static libs for that
+ hide unused widgets unless they are locked
+ added VariableEditor::getWidgetNames(), returns a QStringList of all variable widget names
+ fixed broken cmdline option -s (Thanks visual.bermarte@FF)
+ added ETA to progress dialog
+ changed "F9" behavior: if keyframes exist highlight the current keyframe preset in the text editor, if no keyframes highlight the current preset
+ added "Save" feature to image preview dialog
+ fixed comboslider highlight for locked uniforms
+ adjust slider step with RMB click on slider, no longer on arrow keys
+ added script entries to the help menu
+ prefs option to use #define or const for locked uniforms
+ added a timeline editor (for easing curves) RMB click on time slider
+ added video encoder dialog (mencoder and ffmpeg option files in Misc folder)
+ fixed file path with QDir::separator()
+ fixed file path when using unique name
+ added parameter saving in screenshot PNG files
+ created an online installer for Windows version
+ added the installer maintenance tool to the user menu
+ fixed 2D camera mouse control
+ fixed 2D camera crash on keyframes
+ fixed jumpy sec:frame readout on time slider
+ fixed start/stop bug when animation length is shorter than easingcurve duration
+ remembers ToolBar state (Sabine)
+ tabs in Variable Editor automatically switch from East to North for Dock/UnDock event (05/22/17 Sabine)
+ removed Math/* now using Qt vector and matrix
+ implemented double types
+ move to 2.0-beta
+ Added OpenEXR Tools menu, displays some help info on usage
+ Added samplerCube variable widget
+ Timeline easing curves
+ settings are stored in "frAg" chunk of screenshots and full size rendered image files
+ started adding functionality to the TimeLine editor (RMB on time slider)
+ Complex.frag now includes double types with trig functions thanks to Claude and Clacker
+ fixed 2D.frag, now single pass render frags save image instead of empty buffer. tnx charPixel ;)

# GUI Language Translations
+ German by Sabine
+ Dutch by Sabine
+ Russian by Crist-JRoger and SCORPION

# Tested on Linux desktops
+ Ubuntu 14.04.5 LTS Qt-5.7.1 OpenEXR 2.2 
+ SuSE Leap 42.2 RPM Qt-5.6.2 OpenEXR 2.2 

# Tested on Windows
+ Windows 7,8,10 Qt-5.6.2  OpenEXR 2.1

# Testing on Mac
+ OSX 10.11 GCC 4.8 Qt-5.9.1 OpenEXR 2.2

# Version bump 2.5.0-180909

+ Removed deprecated QGLWidget now using QOpenGL variants
+ Shift+Right mouse button: shows more menus when in fullscreen mode.
+ some improvements in the timeline editor ( RMB click on time slider )
+ added --verbose switch to commandline, turns on variable and info output to console
+ dock widgets are now stackable (uniform editor and log windows)
+ GL area shows scaled tiles when rendering images in tiled mode
+ cubemap sampler image format +x-x+y-y+z-z
+ reports "special", "gl", "uniform" and "unused" variables to console
- removed NVIDIAGL4PLUS #define, now tests context
- removed M Benesi spraygun code (loved it but nobody used it)
+ Progressive mode remembers last view when switching from Animation mode
+ "New" provides minimum working frag code
+ keyframe and easingcurves are handled with QMap<T,T> lists

#  Tested on Linux desktop
+  Compiled with gcc (SuSE Leap 42.3) Qt 5.6.2

#  Tested on Windows 10
+  Compiled with MinGW 4.9.2 Qt 5.6.2
