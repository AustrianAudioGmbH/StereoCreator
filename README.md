# StereoCreator
Our open-source Stereo plug-in developed by [Simon](https://github.com/becksimon) and [AA](https://austrian.audio/).

The StereoCreator allows you to create several different stereo set-ups with one or two OC818 microphones in dual output mode.
Installers for as VST3, AAX and AU are available at [austrian.audio](https://austrian.audio/).

<img width="640" alt="4Ch_StereoCreator" src="https://user-images.githubusercontent.com/70842242/113834931-aafeb680-978b-11eb-9994-449ea329f510.png">

## Building StereoCreator
StereoCreator is based on [JUCE](https://juce.com/). To build StereoCreator, get a recent version of JUCE and open StereoCreator.jucer in Projucer. Select an exporter of your choice (e.g. Visual Studio or Xcode) to create and open a project file in your IDE.

## Requirements
* For building AAX plugins you need to add the [AAX SDK](http://developer.avid.com/) location to your Projucer paths.

## Related repositories
Parts of the code are based on the [IEM Plugin Suite](https://git.iem.at/audioplugins/IEMPluginSuite) - check it out, it's awesome!
