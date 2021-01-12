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
#include "../resources/customComponents/LevelMeter.h"


typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

//==============================================================================
/**
*/
class StereoCreatorAudioProcessorEditor  : public juce::AudioProcessorEditor, private ComboBox::Listener, private Slider::Listener, private Button::Listener, private Timer
{
public:
    StereoCreatorAudioProcessorEditor (StereoCreatorAudioProcessor&, AudioProcessorValueTreeState&);
    ~StereoCreatorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void comboBoxChanged (ComboBox* cb) override;
    void sliderValueChanged (Slider* slider) override;
    void buttonClicked (Button* button) override;
    
    void setComboBoxItemsEnabled(bool twoChannelInput);
    void setSliderVisibility(bool msTwoCh, bool msFourCh, bool width, bool pattern, bool rotation, bool xyPattern, bool xyAngle);
    
    int getControlParameterIndex (Component& control) override;
    
    void setAbButtonAlphaFromLayerState(int layerState);
    
private:
    static const int EDITOR_WIDTH = 580;
    static const int EDITOR_HEIGHT = 400;
    
    StereoCreatorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    
    TitleBar<AALogo, NoIOWidget> title;
    Footer footer;
    LaF globalLaF;
    TooltipWindow tooltipWindow;
    
    Slider slMidGain[2], slSideGain[2], slWidth, slMidPattern, slXyPattern, slXyAngle, slRotation;
    ComboBox cbStereoMode;
    ToggleButton tbChSwitch, tbAutoLevels;
    TextButton tbAbLayer[2];
    
    std::unique_ptr<ReverseSlider::SliderAttachment> slAttMidGain[2], slAttSideGain[2], slAttWidth, slAttMidPattern, slAttXyPattern, slAttXyAngle, slAttRotation;
    std::unique_ptr<ComboBoxAttachment> cbAttStereoMode;
    std::unique_ptr<ButtonAttachment> tbAttChSwitch, tbAttAutoLevels;
 
    GroupComponent grpStereoMode, grpMidGain[2], grpSideGain[2], grpWidth, grpMidPattern, grpXyPattern, grpXyAngle, grpRotation, grpInputMeters;
    
//    const juce::String wrongBusConfigMessageShort = "Wrong Bus Configuration!";
//    const juce::String wrongBusConfigMessageLong = "Make sure to use a two- or four channel track configuration containing the dual-mode signals from the OC-818";
    
    FirstOrderDirectivityVisualizer dirVis[2];
    Colour colours[3];
    
    Path aaLogoBgPath;
    Image arrayImage;
    Rectangle<float> arrayImageArea;
    
    LevelMeter inputMeter[4];
    LevelMeter outputMeter[2];
    
    const juce::String inMeterLabelText[4] = { "L", "R", "F", "B" };
    const juce::String outMeterLabelText[4] = { "L", "R"};
    
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoCreatorAudioProcessorEditor)
};
