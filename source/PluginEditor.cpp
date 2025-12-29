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

#include "PluginEditor.h"
#include "../resources/customComponents/ImgPaths.h"
#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

#define AA_SUPPORT_URL "https://austrian.audio/support-downloads/"

//==============================================================================
StereoCreatorAudioProcessorEditor::StereoCreatorAudioProcessorEditor (
    StereoCreatorAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p), stereoCreatorProcessor (p), valueTreeState (vts)
{
    using namespace juce;

    setSize (EDITOR_WIDTH, EDITOR_HEIGHT);

    addAndMakeVisible (sharedTooltipWindow);

    addAndMakeVisible (logo);

    addAndMakeVisible (abButton);

    sharedTooltipWindow.setMillisecondsBeforeTipAppears (500);

    // loading image data
    arrayImage4Ch = juce::ImageCache::getFromMemory (arrayPng4Ch, arrayPng4ChSize);
    arrayImage2Ch = juce::ImageCache::getFromMemory (arrayPng2Ch, arrayPng2ChSize);

    bCardPath.loadPathFromData (bCardData, sizeof (bCardData));
    cardPath.loadPathFromData (cardData, sizeof (cardData));
    sCardPath.loadPathFromData (sCardData, sizeof (sCardData));
    hCardPath.loadPathFromData (hCardData, sizeof (hCardData));
    eightPath.loadPathFromData (eightData, sizeof (eightData));
    omniPath.loadPathFromData (omniData, sizeof (omniData));

#ifdef AA_SHOW_LOGO
    aaLogoBgPath.loadPathFromData (aaLogoData, sizeof (aaLogoData));
#endif

    // combo box
    addAndMakeVisible (setupComponent);
    cbAttStereoMode = std::make_unique<ComboBoxAttachment> (valueTreeState,
                                                            "stereoMode",
                                                            setupComponent.stereoMode);
    setupComponent.stereoMode.addListener (this);
    tbAttChSwitch = std::make_unique<ButtonAttachment> (valueTreeState,
                                                        "channelSwitch",
                                                        setupComponent.lrSwapButton);
    setupComponent.stereoMode.setSelectedId (stereoCreatorProcessor.getStereoModeIdx());

    // help tooltip
    addAndMakeVisible (&helpToolTip);
    helpToolTip.setText ("help", juce::NotificationType::dontSendNotification);
    helpToolTip.setTextColour (juce::Colours::white.withAlpha (0.5f));
    helpToolTip.setInterceptsMouseClicks (true, false); // Enable mouse clicks
    helpToolTip.addMouseListener (this, false); // Listen for clicks

    for (int i = 0; i < 5; i++)
    {
        auto& s = compensationGainComponent.sliders[i];
        auto& sa = slAttCompensationGain[i];

        sa = std::make_unique<ReverseSlider::SliderAttachment> (valueTreeState,
                                                                "compensationGain" + String (i + 1),
                                                                s);
        s.addListener (this);
    }

    // buttons

    compensationGainComponent.button.addListener (this);
    compensationGainComponent.button.setToggleState (false, NotificationType::dontSendNotification);
    tbAttCalcCompGain = std::make_unique<ButtonAttachment> (valueTreeState,
                                                            "calcCompGain",
                                                            compensationGainComponent.button);

    const int layerState = stereoCreatorProcessor.getAbLayerState();

    abButton[0].addListener (this);
    abButton[0].setToggleState (layerState == eCurrentActiveLayer::layerA,
                                NotificationType::dontSendNotification);

    abButton[1].addListener (this);
    abButton[1].setToggleState (layerState == eCurrentActiveLayer::layerB,
                                NotificationType::dontSendNotification);

    for (size_t i = 0; i < 2; ++i)
    {
        auto& midGain = grpMidGain[i];
        auto& mSlider = midGain.slider;
        addAndMakeVisible (midGain);
        midGain.setText ("Mid Gain");

        slAttMidGain[i] = std::make_unique<ReverseSlider::SliderAttachment> (valueTreeState,
                                                                             "msMidGain",
                                                                             mSlider);

        mSlider.setTextValueSuffix (" dB");
        mSlider.setColour (Slider::rotarySliderOutlineColourId,
                           AAGuiComponents::Colours::polarVisualizerGreenDark);
        mSlider.addListener (this);

        auto& sideGain = grpSideGain[i];
        auto& sSlider = sideGain.slider;
        addAndMakeVisible (sideGain);
        sideGain.setText ("Side Gain");

        slAttSideGain[i] = std::make_unique<ReverseSlider::SliderAttachment> (valueTreeState,
                                                                              "msSideGain",
                                                                              sSlider);

        sSlider.setTextValueSuffix (" dB");
        sSlider.setColour (Slider::rotarySliderOutlineColourId,
                           AAGuiComponents::Colours::polarVisualizerRed);
        sSlider.addListener (this);
    }

    addAndMakeVisible (&grpPseudoStPattern);
    grpPseudoStPattern.setText ("Pattern");
    slAttPseudoStPattern =
        std::make_unique<ReverseSlider::SliderAttachment> (valueTreeState,
                                                           "pseudoStPattern",
                                                           grpPseudoStPattern.slider);
    grpPseudoStPattern.slider.setColour (Slider::rotarySliderOutlineColourId,
                                         AAGuiComponents::Colours::polarVisualizerYellow);
    grpPseudoStPattern.slider.addListener (this);
    grpPseudoStPattern.slider.dirStripTop.setPatternPathsAndFactors (bCardPath,
                                                                     cardPath,
                                                                     bCardFact,
                                                                     cardFact);
    grpPseudoStPattern.slider.dirStripBottom.setPatternPathsAndFactors (omniPath,
                                                                        hCardPath,
                                                                        omniFact,
                                                                        hCardFact);

    addAndMakeVisible (&grpMidPattern);
    grpMidPattern.setText ("Mid Pattern");
    slAttMidPattern = std::make_unique<ReverseSlider::SliderAttachment> (valueTreeState,
                                                                         "msMidPattern",
                                                                         grpMidPattern.slider);
    grpMidPattern.slider.setColour (Slider::rotarySliderOutlineColourId,
                                    AAGuiComponents::Colours::polarVisualizerYellow);
    grpMidPattern.slider.addListener (this);
    grpMidPattern.slider.dirStripTop.setPatternPathsAndFactors (bCardPath,
                                                                cardPath,
                                                                bCardFact,
                                                                cardFact);
    grpMidPattern.slider.dirStripBottom.setPatternPathsAndFactors (omniPath,
                                                                   hCardPath,
                                                                   omniFact,
                                                                   hCardFact);

    addAndMakeVisible (&grpXyPattern);
    grpXyPattern.setText ("Pattern");
    slAttXyPattern = std::make_unique<ReverseSlider::SliderAttachment> (valueTreeState,
                                                                        "trueStXyPattern",
                                                                        grpXyPattern.slider);
    grpXyPattern.slider.setColour (Slider::rotarySliderOutlineColourId,
                                   AAGuiComponents::Colours::polarVisualizerYellow);
    grpXyPattern.slider.addListener (this);
    grpXyPattern.slider.dirStripTop.setPatternPathsAndFactors (cardPath,
                                                               sCardPath,
                                                               cardFact,
                                                               sCardFact);
    grpXyPattern.slider.dirStripBottom.setPatternPathsAndFactors (bCardPath,
                                                                  sCardPath,
                                                                  bCardFact,
                                                                  hCardFact);

    addAndMakeVisible (&grpXyAngle);
    grpXyAngle.setText ("Angle");
    grpXyAngle.slider.setTextValueSuffix (CharPointer_UTF8 (R"(°)"));
    grpXyAngle.slider.setColour (Slider::rotarySliderOutlineColourId,
                                 AAGuiComponents::Colours::polarVisualizerRed);
    grpXyAngle.slider.addListener (this);
    slAttXyAngle = std::make_unique<ReverseSlider::SliderAttachment> (valueTreeState,
                                                                      "trueStXyAngle",
                                                                      grpXyAngle.slider);

    addAndMakeVisible (&grpRotation);
    grpRotation.setText ("Rotation");
    grpRotation.slider.setTextValueSuffix (CharPointer_UTF8 (R"(°)"));
    grpRotation.slider.setColour (Slider::rotarySliderOutlineColourId,
                                  AAGuiComponents::Colours::polarVisualizerRed);
    grpRotation.slider.addListener (this);
    slAttRotation = std::make_unique<ReverseSlider::SliderAttachment> (valueTreeState,
                                                                       "blumleinRot",
                                                                       grpRotation.slider);

    addAndMakeVisible (&compensationGainComponent);

    addAndMakeVisible (&levelMeters);

    // directivity visualiser
    addAndMakeVisible (&dirVis[0]);
    dirVis[0].setDirWeight (0.5f);
    dirVis[0].setColour (AAGuiComponents::Colours::polarVisualizerGreenDark);

    addAndMakeVisible (&dirVis[1]);
    dirVis[1].setDirWeight (-0.5f);
    dirVis[1].setColour (AAGuiComponents::Colours::polarVisualizerRed);
    dirVis[1].shouldDrawGridLines (false);

    comboBoxChanged (&setupComponent.stereoMode);

    startTimer (80);
}

StereoCreatorAudioProcessorEditor::~StereoCreatorAudioProcessorEditor()
{
    for (size_t i = 0; i < 5; ++i)
    {
        compensationGainComponent.sliders[i].removeListener (this);
        slAttCompensationGain[i] = nullptr;
    }

    for (size_t i = 0; i < 2; ++i)
    {
        grpMidGain[i].slider.removeListener (this);
        slAttMidGain[i] = nullptr;
        grpSideGain[i].slider.removeListener (this);
        slAttSideGain[i] = nullptr;
    }

    grpXyAngle.slider.removeListener (this);
    slAttXyAngle = nullptr;
    grpRotation.slider.removeListener (this);
    slAttRotation = nullptr;

    grpPseudoStPattern.slider.removeListener (this);
    slAttPseudoStPattern = nullptr;

    grpMidPattern.slider.removeListener (this);
    slAttMidPattern = nullptr;

    grpXyPattern.slider.removeListener (this);
    slAttXyPattern = nullptr;
}

void StereoCreatorAudioProcessorEditor::mouseUp (const juce::MouseEvent& event)
{
    if (event.eventComponent == &helpToolTip)
    {
        if (! juce::URL (AA_SUPPORT_URL).launchInDefaultBrowser())
        {
            DBG ("Failed to open URL!");
        }
    }
}

//==============================================================================
void StereoCreatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;

    g.fillAll (AAGuiComponents::Colours::mainBackground);

    if (stereoCreatorProcessor.getNumInpCh() == 2) // two channel input
    {
        // title.setLineBounds (true, 0, 0, 0); // default line
        g.drawImageWithin (arrayImage2Ch,
                           0,
                           0,
                           arrayImage2Ch.getWidth() / 2,
                           arrayImage2Ch.getHeight() / 2,
                           RectanglePlacement::onlyReduceInSize);

        //        helpToolTip.setTooltip(helpText2Ch);
    }
    else // four channel input
    {
        // title.setLineBounds (false, 0, 33, 101);
        g.drawImageWithin (arrayImage4Ch,
                           4,
                           8,
                           arrayImage4Ch.getWidth() / 2,
                           arrayImage4Ch.getHeight() / 2,
                           RectanglePlacement::onlyReduceInSize);
        //        helpToolTip.setTooltip(helpText4Ch);
    }

#ifdef AA_SHOW_LOGO
    // background logo
    aaLogoBgPath.applyTransform (aaLogoBgPath.getTransformToScaleToFit (0.50f * currWidth,
                                                                        0.25f * currHeight,
                                                                        0.58f * currWidth,
                                                                        0.58f * currWidth,
                                                                        true,
                                                                        Justification::centred));
    g.setColour (Colours::white.withAlpha (0.1f));
    g.strokePath (aaLogoBgPath, PathStrokeType (0.1f));
    g.fillPath (aaLogoBgPath);
#endif
}

void StereoCreatorAudioProcessorEditor::resized()
{
    using namespace juce;

    constexpr int leftRightMargin = 20;
    constexpr int headerHeight = 50;
    constexpr int topMargin = 10;
    constexpr int rotarySliderWidth = 80;
    constexpr int grpHeight = 100;
    constexpr int comboBoxWidth = 150;
    constexpr int sideAreaWidth = comboBoxWidth;
    constexpr int arrayWidth = 180;
    constexpr int dirVisHeight = 150;

    constexpr int vSpace = 10;
    constexpr int hSpace = 10;

    constexpr int threeSlWidth = 3 * rotarySliderWidth + 2 * hSpace;
    constexpr int twoSlWidth = 2 * rotarySliderWidth + hSpace;

    Rectangle<int> area (getLocalBounds());
    area.reduce (leftRightMargin, topMargin);

    // header and footer
    helpToolTip.setBounds (5, getHeight() - 30, 40, 25);

    Rectangle<int> headerArea = area.removeFromTop (headerHeight);

    logo.setBounds (headerArea.removeFromLeft (140));
    headerArea.removeFromLeft (100);

    abButton.setBounds (headerArea.removeFromLeft (160));

    area.removeFromLeft (arrayWidth);

    //--------------- SIDE AREA ----------------
    Rectangle<int> sideArea (area.removeFromLeft (sideAreaWidth));
    setupComponent.setBounds (sideArea.removeFromTop (90));
    sideArea.removeFromTop (vSpace);

    compensationGainComponent.setBounds (sideArea.removeFromTop (grpHeight));
    sideArea.removeFromTop (vSpace);

    levelMeters.setBounds (sideArea);

    //--------------- MAIN AREA ----------------
    Rectangle<int> mainArea (area.removeFromRight (area.getWidth() - hSpace));

    Rectangle<int> rotarySlArea (mainArea.removeFromTop (grpHeight));
    Rectangle<int> twoRotSlArea = rotarySlArea;
    Rectangle<int> threeRotSlArea = rotarySlArea;
    twoRotSlArea.removeFromLeft ((threeSlWidth / 2) - (twoSlWidth / 2));
    Rectangle<int> twoLabelArea (twoRotSlArea.removeFromTop (grpHeight));
    Rectangle<int> threeLabelArea (threeRotSlArea.removeFromTop (grpHeight));

    // labels
    grpMidGain[0].setBounds (twoLabelArea.removeFromLeft (rotarySliderWidth));

    twoLabelArea.removeFromLeft (hSpace);
    grpSideGain[0].setBounds (twoLabelArea.removeFromLeft (rotarySliderWidth));

    grpMidGain[1].setBounds (threeLabelArea.removeFromLeft (rotarySliderWidth));
    threeLabelArea.removeFromLeft (hSpace);

    grpSideGain[1].setBounds (threeLabelArea.removeFromLeft (rotarySliderWidth));
    threeLabelArea.removeFromLeft (hSpace);

    grpMidPattern.setBounds (threeLabelArea.removeFromLeft (rotarySliderWidth));

    grpPseudoStPattern.setBounds (grpSideGain[1].getBounds());

    grpXyPattern.setBounds (grpMidGain[0].getBounds());
    grpXyAngle.setBounds (grpSideGain[0].getBounds());

    grpRotation.setBounds (grpPseudoStPattern.getBounds());

    // directivity visualiser and meters
    Rectangle<int> dirVisArea (mainArea.removeFromTop (dirVisHeight));
    dirVisArea.removeFromLeft ((threeSlWidth / 2) - dirVisHeight + hSpace);
    dirVis[0].setBounds (dirVisArea);
    dirVis[1].setBounds (dirVisArea);
}

void StereoCreatorAudioProcessorEditor::comboBoxChanged (juce::ComboBox* cb)
{
    using StereoModes = AAGuiComponents::Setup::StereoMode;

    if (cb == &setupComponent.stereoMode)
    {
        switch (cb->getSelectedId())
        {
            case StereoModes::pseudoMsIdx:
                setDirVisAlphaFromSliderValues (&grpMidGain[0].slider, 0);
                setDirVisAlphaFromSliderValues (&grpSideGain[0].slider, 1);

                setSliderVisibility (true, false, false, false, false, false, false);

                dirVis[0].setPatternRotation (90.0f);
                dirVis[0].setDirWeight (0.0f);
                dirVis[1].setPatternRotation (90.0f);
                dirVis[1].setDirWeight (1.0f);

                break;
            case StereoModes::pseudoStereoIdx:
                setSliderVisibility (false, false, true, false, false, false, false);

                dirVis[0].setPatternRotation (-90.0f);
                dirVis[0].setDirWeight (static_cast<float> (grpPseudoStPattern.slider.getValue()));
                dirVis[1].setPatternRotation (90.0f);
                dirVis[1].setDirWeight (static_cast<float> (grpPseudoStPattern.slider.getValue()));
                dirVis[0].setPatternAlpha (1.0f);
                dirVis[1].setPatternAlpha (1.0f);
                break;
            case StereoModes::trueMsIdx:
                setDirVisAlphaFromSliderValues (&grpMidGain[1].slider, 0);
                setDirVisAlphaFromSliderValues (&grpSideGain[1].slider, 1);

                setSliderVisibility (false, true, false, true, false, false, false);

                dirVis[0].setPatternRotation (0.0f);
                dirVis[0].setDirWeight (static_cast<float> (grpMidPattern.slider.getValue()));
                dirVis[1].setPatternRotation (90.0f);
                dirVis[1].setDirWeight (1.0f);

                break;
            case StereoModes::trueStereoIdx:
                setSliderVisibility (false, false, false, false, false, true, true);

                dirVis[0].setPatternRotation (-static_cast<float> (grpXyAngle.slider.getValue())
                                              / 2.0f);
                dirVis[0].setDirWeight (static_cast<float> (grpXyPattern.slider.getValue()));
                dirVis[0].setPatternAlpha (1.0f);
                dirVis[1].setPatternRotation (static_cast<float> (grpXyAngle.slider.getValue())
                                              / 2.0f);
                dirVis[1].setDirWeight (static_cast<float> (grpXyPattern.slider.getValue()));
                dirVis[1].setPatternAlpha (1.0f);
                break;
            case StereoModes::blumleinIdx:
                setSliderVisibility (false, false, false, false, true, false, false);

                dirVis[0].setPatternRotation (static_cast<float> (grpRotation.slider.getValue())
                                              - 45.0f);
                dirVis[0].setDirWeight (1.0f);
                dirVis[0].setPatternAlpha (1.0f);
                dirVis[1].setPatternRotation (static_cast<float> (grpRotation.slider.getValue())
                                              + 45.0f);
                dirVis[1].setDirWeight (1.0f);
                dirVis[1].setPatternAlpha (1.0f);
                break;
            default:
                //                cbStereoMode.setSelectedId(eStereoMode::trueMsIdx);
                break;
        }
    }
    repaint();
}

void StereoCreatorAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    using StereoMode = AAGuiComponents::Setup::StereoMode;

    if (slider == &grpMidGain[0].slider
        && setupComponent.stereoMode.getSelectedId() == StereoMode::pseudoMsIdx)
    {
        setDirVisAlphaFromSliderValues (slider, 0);
    }
    else if (slider == &grpMidGain[1].slider
             && setupComponent.stereoMode.getSelectedId() == StereoMode::trueMsIdx)
    {
        setDirVisAlphaFromSliderValues (slider, 0);
    }
    else if (slider == &grpSideGain[0].slider
             && setupComponent.stereoMode.getSelectedId() == StereoMode::pseudoMsIdx)
    {
        setDirVisAlphaFromSliderValues (slider, 1);
    }
    else if (slider == &grpSideGain[1].slider
             && setupComponent.stereoMode.getSelectedId() == StereoMode::trueMsIdx)
    {
        setDirVisAlphaFromSliderValues (slider, 1);
    }
    else if (slider == &grpPseudoStPattern.slider
             && setupComponent.stereoMode.getSelectedId() == StereoMode::pseudoStereoIdx)
    {
        dirVis[0].setDirWeight (static_cast<float> (slider->getValue()));
        dirVis[1].setDirWeight (static_cast<float> (slider->getValue()));
    }
    else if (slider == &grpMidPattern.slider
             && setupComponent.stereoMode.getSelectedId() == StereoMode::trueMsIdx)
    {
        dirVis[0].setDirWeight (static_cast<float> (slider->getValue()));
    }
    else if (slider == &grpXyPattern.slider
             && setupComponent.stereoMode.getSelectedId() == StereoMode::trueStereoIdx)
    {
        dirVis[0].setDirWeight (static_cast<float> (slider->getValue()));
        dirVis[1].setDirWeight (static_cast<float> (slider->getValue()));
    }
    else if (slider == &grpXyAngle.slider
             && setupComponent.stereoMode.getSelectedId() == StereoMode::trueStereoIdx)
    {
        dirVis[0].setPatternRotation (static_cast<float> (-grpXyAngle.slider.getValue() / 2.0f));
        dirVis[1].setPatternRotation (static_cast<float> (grpXyAngle.slider.getValue() / 2.0f));
    }
    else if (slider == &grpRotation.slider
             && setupComponent.stereoMode.getSelectedId() == StereoMode::blumleinIdx)
    {
        dirVis[0].setPatternRotation (static_cast<float> (grpRotation.slider.getValue() - 45.0f));
        dirVis[1].setPatternRotation (static_cast<float> (grpRotation.slider.getValue() + 45.0f));
    }
    repaint();
}

void StereoCreatorAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    using namespace juce;

    if (button == &abButton[0])
    {
        if (! button->getToggleState())
            stereoCreatorProcessor.setAbLayer (eCurrentActiveLayer::layerB);

        comboBoxChanged (&setupComponent.stereoMode);
    }
    else if (button == &abButton[1])
    {
        if (! button->getToggleState())
            stereoCreatorProcessor.setAbLayer (eCurrentActiveLayer::layerA);

        comboBoxChanged (&setupComponent.stereoMode);
    }
    if (button == &compensationGainComponent.button)
        button->setToggleState (! button->getToggleState(), NotificationType::dontSendNotification);
}

void StereoCreatorAudioProcessorEditor::setDirVisAlphaFromSliderValues (juce::Slider* slider,
                                                                        int dirVisIdx)
{
    const auto sliderRange =
        static_cast<float> (slider->getMaximum() + std::abs (slider->getMinimum()));
    auto newAlpha = static_cast<float> (slider->getValue() + std::abs (slider->getMinimum()))
                    / sliderRange * 0.75f;
    newAlpha += 0.25f;
    dirVis[dirVisIdx].setPatternAlpha (newAlpha);
    repaint();
}

void StereoCreatorAudioProcessorEditor::timerCallback()
{
    for (int i = 0; i < 4; ++i)
        levelMeters.inputMeter[i].setLevel (
            stereoCreatorProcessor.inRms[i].load (std::memory_order_relaxed));

    for (int i = 0; i < 2; ++i)
        levelMeters.outputMeter[i].setLevel (
            stereoCreatorProcessor.outRms[i].load (std::memory_order_relaxed));

    if (stereoCreatorProcessor.getNumInpCh() == 2) // two channel input
    {
        setComboBoxItemsEnabled (true);
        levelMeters.setQuadChannelInputMode (false);
    }
    else // four channel input
    {
        setComboBoxItemsEnabled (false);
        levelMeters.setQuadChannelInputMode (true);
    }

    compensationGainComponent.button.setToggleState (
        stereoCreatorProcessor.compensationGainCalcOver(),
        juce::NotificationType::dontSendNotification);
}

void StereoCreatorAudioProcessorEditor::setComboBoxItemsEnabled (bool twoChannelInput)
{
    using StereoMode = AAGuiComponents::Setup::StereoMode;

    setupComponent.stereoMode.setItemEnabled (StereoMode::pseudoMsIdx, twoChannelInput);
    setupComponent.stereoMode.setItemEnabled (StereoMode::pseudoStereoIdx, twoChannelInput);
    setupComponent.stereoMode.setItemEnabled (StereoMode::trueMsIdx, ! twoChannelInput);
    setupComponent.stereoMode.setItemEnabled (StereoMode::trueStereoIdx, ! twoChannelInput);
    setupComponent.stereoMode.setItemEnabled (StereoMode::blumleinIdx, ! twoChannelInput);
}

void StereoCreatorAudioProcessorEditor::setSliderVisibility (bool msTwoCh,
                                                             bool msFourCh,
                                                             bool width,
                                                             bool msPattern,
                                                             bool rotation,
                                                             bool xyPattern,
                                                             bool xyAngle)
{
    grpMidGain[0].setVisible (msTwoCh);
    grpMidGain[0].setEnabled (msTwoCh);
    grpSideGain[0].setVisible (msTwoCh);
    grpSideGain[0].setEnabled (msTwoCh);
    grpMidGain[1].setVisible (msFourCh);
    grpMidGain[1].setEnabled (msFourCh);
    grpSideGain[1].setVisible (msFourCh);
    grpSideGain[1].setEnabled (msFourCh);
    grpPseudoStPattern.setVisible (width);
    grpPseudoStPattern.setEnabled (width);
    grpMidPattern.setVisible (msPattern);
    grpMidPattern.setEnabled (msPattern);

    grpXyAngle.setVisible (xyAngle);
    grpXyAngle.setEnabled (xyAngle);
    grpXyPattern.setVisible (xyPattern);
    grpXyPattern.setEnabled (xyPattern);
    grpRotation.setVisible (rotation);
    grpRotation.setEnabled (rotation);

    compensationGainComponent.sliders[0].setEnabled (msTwoCh);
    compensationGainComponent.sliders[0].setVisible (msTwoCh);
    compensationGainComponent.sliders[1].setEnabled (width);
    compensationGainComponent.sliders[1].setVisible (width);
    compensationGainComponent.sliders[2].setEnabled (msFourCh);
    compensationGainComponent.sliders[2].setVisible (msFourCh);
    compensationGainComponent.sliders[3].setEnabled (xyPattern);
    compensationGainComponent.sliders[3].setVisible (xyPattern);
    compensationGainComponent.sliders[4].setEnabled (rotation);
    compensationGainComponent.sliders[4].setVisible (rotation);
}

// implement this for AAX automation shortchut
int StereoCreatorAudioProcessorEditor::getControlParameterIndex (Component& control)
{
    if (&control == &grpMidGain[0].slider)
        return 1;
    else if (&control == &grpMidGain[1].slider)
        return 2;
    else if (&control == &grpSideGain[0].slider)
        return 3;
    else if (&control == &grpSideGain[1].slider)
        return 4;
    else if (&control == &grpPseudoStPattern.slider)
        return 5;
    else if (&control == &grpMidPattern.slider)
        return 6;
    else if (&control == &grpXyPattern.slider)
        return 7;
    else if (&control == &grpXyAngle.slider)
        return 8;
    else if (&control == &grpRotation.slider)
        return 9;
    else if (&control == &setupComponent.lrSwapButton)
        return 10;
    else if (&control == &compensationGainComponent.sliders[0])
        return 11;
    else if (&control == &compensationGainComponent.sliders[1])
        return 12;
    else if (&control == &compensationGainComponent.sliders[2])
        return 13;
    else if (&control == &compensationGainComponent.sliders[3])
        return 14;
    else if (&control == &compensationGainComponent.sliders[4])
        return 15;

    return -1;
}
