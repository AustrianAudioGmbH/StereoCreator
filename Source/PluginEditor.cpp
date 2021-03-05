/*
 ==============================================================================
 PluginEditor.cpp
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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../resources/customComponents/ImgPaths.h"

//==============================================================================
StereoCreatorAudioProcessorEditor::StereoCreatorAudioProcessorEditor (StereoCreatorAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
    setSize (EDITOR_WIDTH, EDITOR_HEIGHT);
    
    setLookAndFeel(&globalLaF);
    
    addAndMakeVisible (&title);
    title.setTitle (String("AustrianAudio"),String("StereoCreator"));
    title.setFont (globalLaF.aaMedium,globalLaF.aaRegular);
//    title.setAlertMessage(wrongBusConfigMessageShort, wrongBusConfigMessageLong);
//    title.showAlertSymbol(false);
    
    
    
    addAndMakeVisible(&footer);
    tooltipWindow.setLookAndFeel(&globalLaF);
    tooltipWindow.setMillisecondsBeforeTipAppears(500);
    
    // loading image data
    arrayImage4Ch = ImageCache::getFromMemory (arrayPng4Ch, arrayPng4ChSize);
    arrayImage2Ch = ImageCache::getFromMemory (arrayPng2Ch, arrayPng2ChSize);
    
//    arrayImage4Ch = arrayImage4Ch.rescaled(arrayImage4Ch.getWidth() * 4, arrayImage4Ch.getHeight() * 4);
//    arrayImage2Ch = arrayImage2Ch.rescaled(arrayImage2Ch.getWidth() * 4, arrayImage2Ch.getHeight() * 4);
    
    bCardPath.loadPathFromData (bCardData, sizeof (bCardData));
    cardPath.loadPathFromData (cardData, sizeof (cardData));
    sCardPath.loadPathFromData (sCardData, sizeof (sCardData));
    hCardPath.loadPathFromData (hCardData, sizeof (hCardData));
    eightPath.loadPathFromData (eightData, sizeof (eightData));
    omniPath.loadPathFromData (omniData, sizeof (omniData));
    
    aaLogoBgPath.loadPathFromData (aaLogoData, sizeof (aaLogoData));
    
    // colours
    colours[0] = Colour(0xFD49BA64);
    colours[1] = Colour(0xFDBA4949);
    colours[2] = Colour(0xFDBAAF49);
    
    
    // combo box
    addAndMakeVisible(cbStereoMode);
    cbAttStereoMode.reset(new ComboBoxAttachment (valueTreeState, "stereoMode", cbStereoMode));
    cbStereoMode.addItem("pseudo-m/s", eStereoMode::pseudoMsIdx);
    cbStereoMode.addItem("pseudo-stereo", eStereoMode::pseudoStereoIdx);
    cbStereoMode.addItem("true-m/s", eStereoMode::trueMsIdx);
    cbStereoMode.addItem("true-stereo", eStereoMode::trueStereoIdx);
    cbStereoMode.addItem("blumlein", eStereoMode::blumleinIdx);
    cbStereoMode.setEditableText(false);
    cbStereoMode.setJustificationType(Justification::centred);
    cbStereoMode.setSelectedId(processor.getStereoModeIdx());
    cbStereoMode.addListener(this);
    cbStereoMode.setSelectedId( processor.getStereoModeIdx() );
    
//            zeroDelayModeActive() ? oldProxDistance = 0 : oldProxDistance = oldProxDistanceB;
    
    // rotary sliders
    addAndMakeVisible(&slMidGain[0]);
    slAttMidGain[0].reset(new ReverseSlider::SliderAttachment (valueTreeState, "msMidGain", slMidGain[0]));
    slMidGain[0].setSliderStyle (Slider::Rotary);
    slMidGain[0].setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slMidGain[0].setTextValueSuffix(" dB");
    slMidGain[0].setColour (Slider::rotarySliderOutlineColourId, colours[0]);
    slMidGain[0].addListener (this);
    setDirVisAlphaFromSliderValues(&slMidGain[0], 0);
    
    addAndMakeVisible(&slMidGain[1]);
    slAttMidGain[1].reset(new ReverseSlider::SliderAttachment (valueTreeState, "msMidGain", slMidGain[1]));
    slMidGain[1].setSliderStyle (Slider::Rotary);
    slMidGain[1].setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slMidGain[1].setTextValueSuffix(" dB");
    slMidGain[1].setColour (Slider::rotarySliderOutlineColourId, colours[0]);
    slMidGain[1].addListener (this);
    setDirVisAlphaFromSliderValues(&slMidGain[1], 0);
    
    addAndMakeVisible(&slSideGain[0]);
    slAttSideGain[0].reset(new ReverseSlider::SliderAttachment (valueTreeState, "msSideGain", slSideGain[0]));
    slSideGain[0].setSliderStyle (Slider::Rotary);
    slSideGain[0].setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slSideGain[0].setTextValueSuffix(" dB");
    slSideGain[0].setColour (Slider::rotarySliderOutlineColourId, colours[1]);
    slSideGain[0].addListener (this);
    setDirVisAlphaFromSliderValues(&slSideGain[0], 1);
    
    addAndMakeVisible(&slSideGain[1]);
    slAttSideGain[1].reset(new ReverseSlider::SliderAttachment (valueTreeState, "msSideGain", slSideGain[1]));
    slSideGain[1].setSliderStyle (Slider::Rotary);
    slSideGain[1].setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slSideGain[1].setTextValueSuffix(" dB");
    slSideGain[1].setColour (Slider::rotarySliderOutlineColourId, colours[1]);
    slSideGain[1].addListener (this);
    setDirVisAlphaFromSliderValues(&slSideGain[1], 1);
    
    addAndMakeVisible(&slPseudoStPattern);
    slAttPseudoStPattern.reset(new ReverseSlider::SliderAttachment (valueTreeState, "lrWidth", slPseudoStPattern));
//    slPseudoStPattern.setSliderStyle(Slider::Rotary);
//    slPseudoStPattern.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
//    slPseudoStPattern.setTextValueSuffix(" %");
    slPseudoStPattern.setTooltipEditable(true);
    slPseudoStPattern.setColour(Slider::rotarySliderOutlineColourId, colours[2]);
    slPseudoStPattern.addListener(this);
    slPseudoStPattern.dirStripTop.setPatternPathsAndFactors(bCardPath, cardPath, bCardFact, cardFact);
    slPseudoStPattern.dirStripBottom.setPatternPathsAndFactors(omniPath, hCardPath, omniFact, hCardFact);
    
    addAndMakeVisible(&slMidPattern);
    slAttMidPattern.reset(new ReverseSlider::SliderAttachment (valueTreeState, "msMidPattern", slMidPattern));
//    slMidPattern.setSliderStyle(Slider::Rotary);
//    slMidPattern.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slMidPattern.setTooltipEditable(true);
    slMidPattern.setColour(Slider::rotarySliderOutlineColourId, colours[2]);
    slMidPattern.addListener(this);
    slMidPattern.dirStripTop.setPatternPathsAndFactors(bCardPath, cardPath, bCardFact, cardFact);
    slMidPattern.dirStripBottom.setPatternPathsAndFactors(omniPath, hCardPath, omniFact, hCardFact);
    
    addAndMakeVisible(&slXyPattern);
    slAttXyPattern.reset(new ReverseSlider::SliderAttachment (valueTreeState, "trueStXyPattern", slXyPattern));
    //slXyPattern.setSliderStyle(Slider::Rotary);
    slXyPattern.setTooltipEditable (true);
    //slXyPattern.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slXyPattern.setColour(Slider::rotarySliderOutlineColourId, colours[2]);
    slXyPattern.addListener(this);
    slXyPattern.dirStripTop.setPatternPathsAndFactors(cardPath, sCardPath, cardFact, sCardFact);
    slXyPattern.dirStripBottom.setPatternPathsAndFactors(bCardPath, sCardPath, bCardFact, hCardFact);
    
    for (int i = 0; i < 5; i++)
    {
        addAndMakeVisible(&slCompensationGain[i]);
        slAttCompensationGain[i].reset(new ReverseSlider::SliderAttachment (valueTreeState, "compensationGain"+String(i+1), slCompensationGain[i]));
        slCompensationGain[i].setSliderStyle(Slider::Rotary);
        slCompensationGain[i].setColour(Slider::rotarySliderOutlineColourId, globalLaF.AARed);
        slCompensationGain[i].addListener(this);
        slCompensationGain[i].setTextValueSuffix(" dB");
        slCompensationGain[i].setTextBoxStyle(Slider::TextBoxBelow, false, 60, 15);
    }
    
    // linear sliders
    addAndMakeVisible(&slXyAngle);
    slAttXyAngle.reset(new ReverseSlider::SliderAttachment (valueTreeState, "trueStXyAngle", slXyAngle));
    slXyAngle.setSliderStyle(Slider::LinearHorizontal);
    slXyAngle.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slXyAngle.setColour(Slider::thumbColourId, globalLaF.AARed);
//    slXyAngle.setColour(Slider::rotarySliderOutlineColourId, Colours::black);
    slXyAngle.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
    slXyAngle.addListener(this);
    
    addAndMakeVisible(&slRotation);
    slAttRotation.reset(new ReverseSlider::SliderAttachment (valueTreeState, "blumleinRot", slRotation));
    slRotation.setSliderStyle(Slider::LinearHorizontal);
    slRotation.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slRotation.setColour(Slider::thumbColourId, globalLaF.AARed);
//    slRotation.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    slRotation.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
    slRotation.addListener(this);
    
    // buttons
    addAndMakeVisible(&tbChSwitch);
    tbAttChSwitch.reset(new ButtonAttachment (valueTreeState, "channelSwitch", tbChSwitch));
    tbChSwitch.setButtonText("L/R channel swap");
    
    addAndMakeVisible(&tbCalcCompGain);
    tbAttCalcCompGain.reset(new ButtonAttachment (valueTreeState, "calcCompGain", tbCalcCompGain));
    tbCalcCompGain.setButtonText("calculate");
    tbCalcCompGain.addListener(this);
    tbCalcCompGain.setToggleState(false, NotificationType::dontSendNotification);
    
    addAndMakeVisible(&tbAbLayer[0]);
    tbAbLayer[0].setButtonText("A");
    tbAbLayer[0].addListener(this);
    tbAbLayer[0].setClickingTogglesState(true);
    tbAbLayer[0].setRadioGroupId(1);
    tbAbLayer[0].setToggleState(true, NotificationType::dontSendNotification);
    
    addAndMakeVisible(&tbAbLayer[1]);
    tbAbLayer[1].setButtonText("B");
    tbAbLayer[1].addListener(this);
    tbAbLayer[1].setClickingTogglesState(true);
    tbAbLayer[1].setRadioGroupId(1);
    tbAbLayer[1].setToggleState(false, NotificationType::dontSendNotification);
    
    setAbButtonAlphaFromLayerState(eCurrentActiveLayer::layerA);
    
    // group components and labels
    addAndMakeVisible(&grpStereoMode);
    grpStereoMode.setText("setup");
    grpStereoMode.setTextLabelPosition (Justification::centredLeft);
    
    addAndMakeVisible(&grpMidGain[0]);
    grpMidGain[0].setText("mid gain");
    grpMidGain[0].setTextLabelPosition(Justification::centred);
    
    addAndMakeVisible(&grpMidGain[1]);
    grpMidGain[1].setText("mid gain");
    grpMidGain[1].setTextLabelPosition(Justification::centred);
    
    addAndMakeVisible(&grpSideGain[0]);
    grpSideGain[0].setText("side gain");
    grpSideGain[0].setTextLabelPosition(Justification::centred);
    
    addAndMakeVisible(&grpSideGain[1]);
    grpSideGain[1].setText("side gain");
    grpSideGain[1].setTextLabelPosition(Justification::centred);
    
    addAndMakeVisible(&grpPseudoStPattern);
    grpPseudoStPattern.setText("pattern");
    grpPseudoStPattern.setTextLabelPosition(Justification::centred);
    
    addAndMakeVisible(&grpMidPattern);
    grpMidPattern.setText("mid pattern");
    grpMidPattern.setTextLabelPosition(Justification::centred);
    
    addAndMakeVisible(&grpXyPattern);
    grpXyPattern.setText("pattern");
    grpXyPattern.setTextLabelPosition(Justification::centred);
    
    addAndMakeVisible(&grpXyAngle);
    grpXyAngle.setText("recording angle");
    grpXyAngle.setTextLabelPosition(Justification::centred);
    
    addAndMakeVisible(&grpRotation);
    grpRotation.setText("rotation");
    grpRotation.setTextLabelPosition(Justification::centred);
    
    addAndMakeVisible(&grpCompensationGain);
    grpCompensationGain.setText("compensation gain");
    grpCompensationGain.setTextLabelPosition (Justification::centredLeft);

    addAndMakeVisible(&grpInputMeters);
    grpInputMeters.setText("input - output levels");
    grpInputMeters.setTextLabelPosition(Justification::centredLeft);
    
    // directivity visualiser
    addAndMakeVisible(&dirVis[0]);
    dirVis[0].setDirWeight(0.5f);
    dirVis[0].setColour(colours[0]);
    
    addAndMakeVisible(&dirVis[1]);
    dirVis[1].setDirWeight(-0.5f);
    dirVis[1].setColour(colours[1]);
    
    // meters
    for (int i = 0; i < 4; ++i)
    {
        addAndMakeVisible(&inputMeter[i]);
        inputMeter[i].setColour(globalLaF.AARed);
        inputMeter[i].setLabelText(inMeterLabelText[i]);
    }
    addAndMakeVisible(&outputMeter[0]);
    outputMeter[0].setColour(globalLaF.AARed);
    outputMeter[0].setLabelText(outMeterLabelText[0]);
    addAndMakeVisible(&outputMeter[1]);
    outputMeter[1].setColour(globalLaF.AARed);
    outputMeter[1].setLabelText(outMeterLabelText[1]);
    
    setSliderVisibility(false, false, false, false, false, false, false);
    
    startTimer(80);
}

StereoCreatorAudioProcessorEditor::~StereoCreatorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void StereoCreatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    const int currHeight = getHeight();
    const int currWidth = getWidth();
    
    g.fillAll (globalLaF.ClBackground);
    
    if (processor.getNumInpCh() == 2) // two channel input
    {
        title.setLineBounds(true, 0, 0, 0); // deafult line
        
        g.drawImage(arrayImage2Ch, 8, 72, arrayImageArea.getWidth() + 15, currHeight - 90, 0, 0, arrayImage2Ch.getWidth(), arrayImage2Ch.getHeight());
    }
    else // four channel input
    {
        title.setLineBounds(false, 0, 28, 102);
        g.drawImage(arrayImage4Ch, 24, 2, arrayImageArea.getWidth() - 8, currHeight + 35, 0, 0, arrayImage4Ch.getWidth(), arrayImage4Ch.getHeight());
    }
    
    // background logo
    aaLogoBgPath.applyTransform (aaLogoBgPath.getTransformToScaleToFit (0.50f * currWidth, 0.25f * currHeight,
                                                                        0.58f * currWidth, 0.58f * currWidth, true, Justification::centred));
    g.setColour (Colours::white.withAlpha(0.1f));
    g.strokePath (aaLogoBgPath, PathStrokeType (0.1f));
    g.fillPath (aaLogoBgPath);
}

void StereoCreatorAudioProcessorEditor::resized()
{
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 15;
    const int topMargin = 10;
    const int rotarySliderHeight = 70;
    const int rotarySliderWidth = 80;
    const int grpHeight = 20;
    const int comboBoxWidth = 140;
    const int comboBoxHeight = 20;
    const int toggleBtHeight = 20;
    const int sideAreaWidth = comboBoxWidth;
    const int arrayWidth = 160;
    const int linearSliderHeight = 40;
    const int dirVisHeight = 150;
    const float meterWidth = 12;
    const float meterHeight = 150;
    const float meterSpacing = 2;
    const float abLayerButtonHeight = 28;
    const float compGainHeight = 60;
    const float compGainWidth = 60;
    
    const int vSpace = 5;
    const int hSpace = 10;
    
    const int threeSlWidth = 3 * rotarySliderWidth + 2 * hSpace;
    const int twoSlWidth = 2 * rotarySliderWidth + hSpace;
    
    Rectangle<int> area (getLocalBounds());
    
    // header and footer
    Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);
    
    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    Rectangle<int> abButtonArea = headerArea.removeFromRight(3 * abLayerButtonHeight);
    abButtonArea.removeFromTop((headerArea.getHeight() / 2) - (abLayerButtonHeight / 2));
    abButtonArea.removeFromBottom((headerArea.getHeight() / 2) - (abLayerButtonHeight / 2));
    tbAbLayer[0].setBounds(abButtonArea.removeFromLeft(abLayerButtonHeight));
    abButtonArea.removeFromLeft(abLayerButtonHeight / 2);
    tbAbLayer[1].setBounds(abButtonArea.removeFromLeft(abLayerButtonHeight));
    

    area.removeFromTop(topMargin);
    arrayImageArea = area.removeFromLeft(arrayWidth).toFloat();
//    arrayImageArea.removeFromTop(headerArea.getHeight());
//    arrayImageArea.removeFromLeft(leftRightMargin);
    
    //--------------- SIDE AREA ----------------
    Rectangle<int> sideArea (area.removeFromLeft(sideAreaWidth));
    grpStereoMode.setBounds(sideArea.removeFromTop(grpHeight));
    sideArea.removeFromTop(vSpace);
    cbStereoMode.setBounds(sideArea.removeFromTop(comboBoxHeight));
    sideArea.removeFromTop(vSpace);
    tbChSwitch.setBounds(sideArea.removeFromTop(toggleBtHeight));
    sideArea.removeFromTop(3 * vSpace);
    
    Rectangle<int> compGainArea (sideArea.removeFromTop(compGainHeight + grpHeight + vSpace));
    grpCompensationGain.setBounds(compGainArea.removeFromTop(grpHeight));
    compGainArea.removeFromTop(vSpace);
    slCompensationGain[0].setBounds(compGainArea.removeFromLeft(compGainWidth));
    for (int i = 1; i < 5; i++)
    {
        slCompensationGain[i].setBounds(slCompensationGain[0].getBounds());
    }
    compGainArea.removeFromLeft(hSpace);
    compGainArea.removeFromTop(3.5f * vSpace);
    compGainArea.removeFromBottom(3.5f * vSpace);
    
    tbCalcCompGain.setBounds(compGainArea);
    
    sideArea.removeFromTop(3 * vSpace);
    grpInputMeters.setBounds(sideArea.removeFromTop(grpHeight));
    sideArea.removeFromTop(vSpace);
    
    Rectangle<int> inputMeterArea (sideArea.removeFromTop(meterHeight));
    inputMeterArea.removeFromLeft(hSpace - 1);
    for (int i = 0; i < 4; i++)
    {
        inputMeter[i].setBounds(inputMeterArea.removeFromLeft(meterWidth));
        inputMeterArea.removeFromLeft(meterSpacing);
    }
    inputMeterArea.removeFromLeft(4 * hSpace);
    outputMeter[0].setBounds(inputMeterArea.removeFromLeft(meterWidth));
    inputMeterArea.removeFromLeft(meterSpacing);
    outputMeter[1].setBounds(inputMeterArea.removeFromLeft(meterWidth));
    
    //--------------- MAIN AREA ----------------
    Rectangle<int> mainArea (area.removeFromRight(area.getWidth() - hSpace));
    
    Rectangle<int> rotarySlArea (mainArea.removeFromTop(rotarySliderHeight + grpHeight + vSpace));
    Rectangle<int> twoRotSlArea = rotarySlArea;
    Rectangle<int> threeRotSlArea = rotarySlArea;
    twoRotSlArea.removeFromLeft((threeSlWidth / 2) - (twoSlWidth / 2));
    Rectangle<int> twoLabelArea (twoRotSlArea.removeFromTop(grpHeight));
    Rectangle<int> threeLabelArea (threeRotSlArea.removeFromTop(grpHeight));
    
    // labels
    grpMidGain[0].setBounds(twoLabelArea.removeFromLeft(rotarySliderWidth));
    twoLabelArea.removeFromLeft(hSpace);
    grpSideGain[0].setBounds(twoLabelArea.removeFromLeft(rotarySliderWidth));
    
    grpMidGain[1].setBounds(threeLabelArea.removeFromLeft(rotarySliderWidth));
    threeLabelArea.removeFromLeft(hSpace);
    grpSideGain[1].setBounds(threeLabelArea.removeFromLeft(rotarySliderWidth));
    threeLabelArea.removeFromLeft(hSpace);
    grpMidPattern.setBounds(threeLabelArea.removeFromLeft(rotarySliderWidth));
    
    grpPseudoStPattern.setBounds(grpSideGain[1].getBounds());
    grpXyPattern.setBounds(grpSideGain[1].getBounds());
    
    // slider
    twoRotSlArea.removeFromTop(vSpace);
    threeRotSlArea.removeFromTop(vSpace);
    // reference sliders/labels for placing two sliders in the plugin
    slMidGain[0].setBounds(twoRotSlArea.removeFromLeft(rotarySliderWidth));
    twoRotSlArea.removeFromLeft(hSpace);
    slSideGain[0].setBounds(twoRotSlArea.removeFromLeft(rotarySliderWidth));
    
    // reference sliders for placing one or three sliders in the plugin
    slMidGain[1].setBounds(threeRotSlArea.removeFromLeft(rotarySliderWidth));
    threeRotSlArea.removeFromLeft(hSpace);
    slSideGain[1].setBounds(threeRotSlArea.removeFromLeft(rotarySliderWidth));
    threeRotSlArea.removeFromLeft(hSpace);
    slMidPattern.setBounds(threeRotSlArea.removeFromLeft(rotarySliderWidth));
    
    slPseudoStPattern.setBounds(slSideGain[1].getBounds());
    slXyPattern.setBounds(slSideGain[1].getBounds());
    
    // directivity visualiser and meters
    Rectangle<int> dirVisArea (mainArea.removeFromTop(dirVisHeight));
    dirVisArea.removeFromLeft((threeSlWidth / 2) - dirVisHeight + hSpace);
    dirVis[0].setBounds(dirVisArea);
    dirVis[1].setBounds(dirVisArea);
    
    // reference for placing a linear slider below the directivity visualiser
    Rectangle<int> bottomArea (mainArea.removeFromTop(linearSliderHeight + 2 * vSpace + grpHeight));
    bottomArea.removeFromLeft((threeSlWidth / 2) - (twoSlWidth / 2) - hSpace);
    bottomArea.removeFromRight((threeSlWidth / 2) - (twoSlWidth / 2));
    Rectangle<int> bottomLabelArea (bottomArea.removeFromTop(grpHeight));
    grpXyAngle.setBounds(bottomLabelArea);
    slXyAngle.setBounds(bottomArea);
    
    
    slRotation.setBounds(slXyAngle.getBounds());
    grpRotation.setBounds(grpXyAngle.getBounds());
}

void StereoCreatorAudioProcessorEditor::comboBoxChanged(ComboBox *cb)
{
    if (cb == &cbStereoMode)
    {
        switch (cb->getSelectedId())
        {
            case pseudoMsIdx:
                setDirVisAlphaFromSliderValues(&slMidGain[0], 0);
                setDirVisAlphaFromSliderValues(&slSideGain[0], 1);
                
                setSliderVisibility(true, false, false, false, false, false, false);
                
                dirVis[0].setPatternRotation(90.0f);
                dirVis[0].setDirWeight(0.0f);
                dirVis[1].setPatternRotation(90.0f);
                dirVis[1].setDirWeight(1.0f);
                
                break;
            case pseudoStereoIdx:
                setSliderVisibility(false, false, true, false, false, false, false);
                
                dirVis[0].setPatternRotation(- 90.0f);
                dirVis[0].setDirWeight(slPseudoStPattern.getValue());
                dirVis[1].setPatternRotation(90.0f);
                dirVis[1].setDirWeight(slPseudoStPattern.getValue());
                dirVis[0].setPatternAlpha(1.0f);
                dirVis[1].setPatternAlpha(1.0f);
                break;
            case trueMsIdx:
                setDirVisAlphaFromSliderValues(&slMidGain[1], 0);
                setDirVisAlphaFromSliderValues(&slSideGain[1], 1);
                
                setSliderVisibility(false, true, false, true, false, false, false);
                
                dirVis[0].setPatternRotation(0.0f);
                dirVis[0].setDirWeight(slMidPattern.getValue());
                dirVis[1].setPatternRotation(90.0f);
                dirVis[1].setDirWeight(1.0f);
                
                break;
            case trueStereoIdx:
                setSliderVisibility(false, false, false, false, false, true, true);
                
                dirVis[0].setPatternRotation(- slXyAngle.getValue() / 2.0f);
                dirVis[0].setDirWeight(slXyPattern.getValue());
                dirVis[0].setPatternAlpha(1.0f);
                dirVis[1].setPatternRotation(slXyAngle.getValue() / 2.0f);
                dirVis[1].setDirWeight(slXyPattern.getValue());
                dirVis[1].setPatternAlpha(1.0f);
                break;
            case blumleinIdx:
                setSliderVisibility(false, false, false, false, true, false, false);
                
                dirVis[0].setPatternRotation(slRotation.getValue() - 45.0f);
                dirVis[0].setDirWeight(1.0f);
                dirVis[0].setPatternAlpha(1.0f);
                dirVis[1].setPatternRotation(slRotation.getValue() + 45.0f);
                dirVis[1].setDirWeight(1.0f);
                dirVis[1].setPatternAlpha(1.0f);
                break;
            default:
                cbStereoMode.setSelectedId(eStereoMode::trueMsIdx);
                break;
        }
    }
    repaint();
}

void StereoCreatorAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    if (slider == &slMidGain[0] && cbStereoMode.getSelectedId() == pseudoMsIdx)
    {
        setDirVisAlphaFromSliderValues(slider, 0);
    }
    else if (slider == &slMidGain[1] && cbStereoMode.getSelectedId() == trueMsIdx)
    {
        setDirVisAlphaFromSliderValues(slider, 0);
    }
    else if (slider == &slSideGain[0] && cbStereoMode.getSelectedId() == pseudoMsIdx)
    {
        setDirVisAlphaFromSliderValues(slider, 1);
    }
    else if (slider == &slSideGain[1] && cbStereoMode.getSelectedId() == trueMsIdx)
    {
        setDirVisAlphaFromSliderValues(slider, 1);
    }
    else if (slider == &slPseudoStPattern && cbStereoMode.getSelectedId() == pseudoStereoIdx)
    {
        dirVis[0].setDirWeight(slider->getValue());
        dirVis[1].setDirWeight(slider->getValue());
    }
    else if (slider == &slMidPattern && cbStereoMode.getSelectedId() == trueMsIdx)
    {
        dirVis[0].setDirWeight(slider->getValue());
    }
    else if (slider == &slXyPattern && cbStereoMode.getSelectedId() == trueStereoIdx)
    {
        dirVis[0].setDirWeight(slider->getValue());
        dirVis[1].setDirWeight(slider->getValue());
    }
    else if (slider == &slXyAngle && cbStereoMode.getSelectedId() == trueStereoIdx)
    {
        dirVis[0].setPatternRotation(- slXyAngle.getValue() / 2.0f);
        dirVis[1].setPatternRotation(slXyAngle.getValue() / 2.0f);
    }
    else if (slider == &slRotation && cbStereoMode.getSelectedId() == blumleinIdx)
    {
        dirVis[0].setPatternRotation(slRotation.getValue() - 45.0f);
        dirVis[1].setPatternRotation(slRotation.getValue() + 45.0f);
    }
    repaint();
}

void StereoCreatorAudioProcessorEditor::buttonClicked(Button *button)
{
    if (button == &tbAbLayer[0])
    {
        bool isToggled = button->getToggleState();
        if (isToggled < 0.5f)
        {
            processor.setAbLayer(eCurrentActiveLayer::layerB);
            setAbButtonAlphaFromLayerState(eCurrentActiveLayer::layerB);
        }
        comboBoxChanged(&cbStereoMode);
    }
    else if (button == &tbAbLayer[1])
    {
        bool isToggled = button->getToggleState();
        if (isToggled < 0.5f)
        {
            processor.setAbLayer(eCurrentActiveLayer::layerA);
            setAbButtonAlphaFromLayerState(eCurrentActiveLayer::layerA);
        }
        comboBoxChanged(&cbStereoMode);
    }
    else if (button == &tbCalcCompGain)
    {
        bool isToggled = button->getToggleState();
        button->setToggleState(!isToggled, NotificationType::dontSendNotification);
    }
}

void StereoCreatorAudioProcessorEditor::setAbButtonAlphaFromLayerState(int layerState)
{
    if (layerState == eCurrentActiveLayer::layerA)
    {
        tbAbLayer[0].setAlpha(1.0f);
        tbAbLayer[1].setAlpha(0.3f);
    }
    else
    {
        tbAbLayer[0].setAlpha(0.3f);
        tbAbLayer[1].setAlpha(1.0f);
    }
}

void StereoCreatorAudioProcessorEditor::setDirVisAlphaFromSliderValues(Slider *slider, int dirVisIdx)
{
    float sliderRange = slider->getMaximum() + std::abs(slider->getMinimum());
    float newAlpha = (slider->getValue() + std::abs(slider->getMinimum())) / sliderRange * 0.75f;
    newAlpha += 0.25f;
    dirVis[dirVisIdx].setPatternAlpha(newAlpha);
    repaint();
}

void StereoCreatorAudioProcessorEditor::timerCallback()
{
    for (int i = 0; i < 4; ++i)
    {
        inputMeter[i].setLevel(processor.inRms[i].get());
    }
    outputMeter[0].setLevel(processor.outRms[0].get());
    outputMeter[1].setLevel(processor.outRms[1].get());
    
    if (processor.getNumInpCh() == 2) // two channel input
    {
        setComboBoxItemsEnabled(true);
        inputMeter[0].setVisible(true);
        inputMeter[1].setVisible(true);
        inputMeter[2].setVisible(false);
        inputMeter[3].setVisible(false);
    }
    else // four channel input
    {
        setComboBoxItemsEnabled(false);
        inputMeter[0].setVisible(true);
        inputMeter[1].setVisible(true);
        inputMeter[2].setVisible(true);
        inputMeter[3].setVisible(true);
    }
    
    tbCalcCompGain.setToggleState(processor.compensationGainCalcOver(), NotificationType::dontSendNotification);
}

void StereoCreatorAudioProcessorEditor::setComboBoxItemsEnabled(bool twoChannelInput)
{
        cbStereoMode.setItemEnabled(eStereoMode::pseudoMsIdx, twoChannelInput);
        cbStereoMode.setItemEnabled(eStereoMode::pseudoStereoIdx, twoChannelInput);
        cbStereoMode.setItemEnabled(eStereoMode::trueMsIdx, !twoChannelInput);
        cbStereoMode.setItemEnabled(eStereoMode::trueStereoIdx, !twoChannelInput);
        cbStereoMode.setItemEnabled(eStereoMode::blumleinIdx, !twoChannelInput);
}

void StereoCreatorAudioProcessorEditor::setSliderVisibility(bool msTwoCh, bool msFourCh, bool width, bool msPattern, bool rotation, bool xyPattern, bool xyAngle)
{
    slMidGain[0].setVisible(msTwoCh);
    slMidGain[0].setEnabled(msTwoCh);
    grpMidGain[0].setVisible(msTwoCh);
    grpMidGain[0].setEnabled(msTwoCh);
    slSideGain[0].setVisible(msTwoCh);
    slSideGain[0].setEnabled(msTwoCh);
    grpSideGain[0].setVisible(msTwoCh);
    grpSideGain[0].setEnabled(msTwoCh);
    slMidGain[1].setVisible(msFourCh);
    slMidGain[1].setEnabled(msFourCh);
    grpMidGain[1].setVisible(msFourCh);
    grpMidGain[1].setEnabled(msFourCh);
    slSideGain[1].setVisible(msFourCh);
    slSideGain[1].setEnabled(msFourCh);
    grpSideGain[1].setVisible(msFourCh);
    grpSideGain[1].setEnabled(msFourCh);
    slPseudoStPattern.setVisible(width);
    slPseudoStPattern.setEnabled(width);
    grpPseudoStPattern.setVisible(width);
    grpPseudoStPattern.setEnabled(width);
    slMidPattern.setVisible(msPattern);
    slMidPattern.setEnabled(msPattern);
    grpMidPattern.setVisible(msPattern);
    grpMidPattern.setEnabled(msPattern);
    
    slXyAngle.setVisible(xyAngle);
    slXyAngle.setEnabled(xyAngle);
    grpXyAngle.setVisible(xyAngle);
    grpXyAngle.setEnabled(xyAngle);
    slXyPattern.setVisible(xyPattern);
    slXyPattern.setEnabled(xyPattern);
    grpXyPattern.setVisible(xyPattern);
    grpXyAngle.setEnabled(xyPattern);
    slRotation.setVisible(rotation);
    slRotation.setEnabled(rotation);
    grpRotation.setVisible(rotation);
    grpRotation.setEnabled(rotation);
    
    slCompensationGain[0].setEnabled(msTwoCh);
    slCompensationGain[0].setVisible(msTwoCh);
    slCompensationGain[1].setEnabled(width);
    slCompensationGain[1].setVisible(width);
    slCompensationGain[2].setEnabled(msFourCh);
    slCompensationGain[2].setVisible(msFourCh);
    slCompensationGain[3].setEnabled(xyPattern);
    slCompensationGain[3].setVisible(xyPattern);
    slCompensationGain[4].setEnabled(rotation);
    slCompensationGain[4].setVisible(rotation);
}


// implement this for AAX automation shortchut
int StereoCreatorAudioProcessorEditor::getControlParameterIndex (Component& control)
{
    if (&control == &slMidGain[0])
        return 1;
    else if (&control == &slMidGain[1])
        return 2;
    else if (&control == &slSideGain[0])
        return 3;
    else if (&control == &slSideGain[1])
        return 4;
    else if (&control == &slPseudoStPattern)
        return 5;
    else if (&control == &slMidPattern)
        return 6;
    else if (&control == &slXyPattern)
        return 7;
    else if (&control == &slXyAngle)
        return 8;
    else if (&control == &slRotation)
        return 9;
    else if (&control == &tbChSwitch)
        return 10;
    
    return -1;
}
