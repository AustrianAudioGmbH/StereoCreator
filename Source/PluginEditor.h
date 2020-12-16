/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../resources/lookAndFeel/AA_LaF.h"
#include "../resources/customComponents/TitleBar.h"
#include "../resources/customComponents/SimpleLabel.h"
#include "../resources/customComponents/ReverseSlider.h"
#include "../resources/customComponents/FirstOrderDirectivityVisualizer.h"


typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

//==============================================================================
/**
*/
class StereoCreatorAudioProcessorEditor  : public juce::AudioProcessorEditor, private ComboBox::Listener, private Slider::Listener, private Timer
{
public:
    StereoCreatorAudioProcessorEditor (StereoCreatorAudioProcessor&, AudioProcessorValueTreeState&);
    ~StereoCreatorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void comboBoxChanged (ComboBox* cb) override;
    void sliderValueChanged (Slider* slider) override;
    

private:
    static const int EDITOR_WIDTH = 580;
    static const int EDITOR_HEIGHT = 350;
    
    StereoCreatorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    
    TitleBar<AALogo, NoIOWidget> title;
    Footer footer;
    LaF globalLaF;
    TooltipWindow tooltipWindow;
    
    Slider slMidGain[2], slSideGain[2], slWidth, slMidPattern;
    ComboBox cbStereoMode;
    ToggleButton tbChSwitch, tbAutoLevels;
    
    std::unique_ptr<ReverseSlider::SliderAttachment> slAttMidGain[2], slAttSideGain[2], slAttWidth, slAttMidPattern;
    std::unique_ptr<ComboBoxAttachment> cbAttStereoMode;
    std::unique_ptr<ButtonAttachment> tbAttChSwitch, tbAttAutoLevels;
 
    GroupComponent grpStereoMode, grpMidGain[2], grpSideGain[2], grpWidth, grpMidPattern;
    
//    const juce::String wrongBusConfigMessageShort = "Wrong Bus Configuration!";
//    const juce::String wrongBusConfigMessageLong = "Make sure to use a two- or four channel track configuration containing the dual-mode signals from the OC-818";
    
    FirstOrderDirectivityVisualizer dirVis[2];
    Colour colours[3];
    
    Path aaLogoBgPath;
    Image arrayImage;
    Rectangle<float> arrayImageArea;
    
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoCreatorAudioProcessorEditor)
};
