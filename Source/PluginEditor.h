/*
 ==============================================================================
 PluginEditor.h
 Author: Simon Beck
 
 Copyright (c) 2019 - Austrian Audio GmbH
 www.austrian.audio
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "../resources/customComponents/DirSlider.h"


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
    static const int EDITOR_WIDTH = 640;
    static const int EDITOR_HEIGHT = 400;
    
    StereoCreatorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    
    TitleBar<AALogo, NoIOWidget> title;
    Footer footer;
    LaF globalLaF;
    TooltipWindow tooltipWindow;
    
    Slider slMidGain[2], slSideGain[2], slWidth, slXyAngle, slRotation;// , slXyPattern, slMidPattern,
    ComboBox cbStereoMode;
    ToggleButton tbChSwitch, tbAutoLevels;
    TextButton tbAbLayer[2];
    
    DirSlider slXyPattern, slMidPattern;
    
    
    
    std::unique_ptr<ReverseSlider::SliderAttachment> slAttMidGain[2], slAttSideGain[2], slAttWidth, slAttMidPattern, slAttXyPattern, slAttXyAngle, slAttRotation;
    std::unique_ptr<ComboBoxAttachment> cbAttStereoMode;
    std::unique_ptr<ButtonAttachment> tbAttChSwitch, tbAttAutoLevels;
 
    GroupComponent grpStereoMode, grpMidGain[2], grpSideGain[2], grpWidth, grpMidPattern, grpXyPattern, grpXyAngle, grpRotation, grpInputMeters;
    
//    const juce::String wrongBusConfigMessageShort = "Wrong Bus Configuration!";
//    const juce::String wrongBusConfigMessageLong = "Make sure to use a two- or four channel track configuration containing the dual-mode signals from the OC-818";
    
    FirstOrderDirectivityVisualizer dirVis[2];
    Colour colours[3];
    
    Path aaLogoBgPath;
    Image arrayImage2Ch;
    Image arrayImage4Ch;
    Rectangle<float> arrayImageArea;
    
    LevelMeter inputMeter[4];
    LevelMeter outputMeter[2];
    
    const juce::String inMeterLabelText[4] = { "L", "R", "F", "B" };
    const juce::String outMeterLabelText[4] = { "L", "R"};
    
    const float omniFact = 0.0f;
    const float eightFact = 1.0f;
    const float bCardFact = 0.37f;
    const float cardFact = 0.5f;
    const float sCardFact = 0.634f;
    const float hCardFact = 0.75f;
    
    Path bCardPath;
    Path cardPath;
    Path sCardPath;
    Path hCardPath;
    Path eightPath;
    Path omniPath;
    
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoCreatorAudioProcessorEditor)
};
