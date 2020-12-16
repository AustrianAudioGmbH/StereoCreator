/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

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
    
    arrayImage = ImageCache::getFromMemory (arrayPng, arrayPngSize);
    aaLogoBgPath.loadPathFromData (aaLogoData, sizeof (aaLogoData));
    
    // colours
    colours[0] = Colour(0xFDBA4949);
    colours[1] = Colour(0xFD49BA64);
    colours[2] = Colour(0xFDBAAF49);
    
    
    // combo box
    addAndMakeVisible(cbStereoMode);
    cbAttStereoMode.reset(new ComboBoxAttachment (valueTreeState, "stereoMode", cbStereoMode));
    cbStereoMode.addItem("m/s", eStereoMode::msIdx);
    cbStereoMode.addItem("pass through", eStereoMode::passThroughIdx);
//    cbStereoMode.setItemEnabled(eStereoMode::passThroughIdx, false);
    cbStereoMode.setEditableText(false);
    cbStereoMode.setJustificationType(Justification::centred);
    cbStereoMode.setSelectedId(processor.getStereoModeIdx());
    cbStereoMode.addListener(this);
    
    // sliders
    addAndMakeVisible(&slMidGain[0]);
    slAttMidGain[0].reset(new ReverseSlider::SliderAttachment (valueTreeState, "msMidGain", slMidGain[0]));
    slMidGain[0].setSliderStyle (Slider::Rotary);
    slMidGain[0].setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slMidGain[0].setTextValueSuffix(" dB");
    slMidGain[0].setColour (Slider::rotarySliderOutlineColourId, colours[0]);
    slMidGain[0].addListener (this);
    
    addAndMakeVisible(&slMidGain[1]);
    slAttMidGain[1].reset(new ReverseSlider::SliderAttachment (valueTreeState, "msMidGain", slMidGain[1]));
    slMidGain[1].setSliderStyle (Slider::Rotary);
    slMidGain[1].setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slMidGain[1].setTextValueSuffix(" dB");
    slMidGain[1].setColour (Slider::rotarySliderOutlineColourId, colours[0]);
    slMidGain[1].addListener (this);
    
    addAndMakeVisible(&slSideGain[0]);
    slAttSideGain[0].reset(new ReverseSlider::SliderAttachment (valueTreeState, "msSideGain", slSideGain[0]));
    slSideGain[0].setSliderStyle (Slider::Rotary);
    slSideGain[0].setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slSideGain[0].setTextValueSuffix(" dB");
    slSideGain[0].setColour (Slider::rotarySliderOutlineColourId, colours[1]);
    slSideGain[0].addListener (this);
    
    addAndMakeVisible(&slSideGain[1]);
    slAttSideGain[1].reset(new ReverseSlider::SliderAttachment (valueTreeState, "msSideGain", slSideGain[1]));
    slSideGain[1].setSliderStyle (Slider::Rotary);
    slSideGain[1].setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slSideGain[1].setTextValueSuffix(" dB");
    slSideGain[1].setColour (Slider::rotarySliderOutlineColourId, colours[1]);
    slSideGain[1].addListener (this);
    
    addAndMakeVisible(&slWidth);
    slAttWidth.reset(new ReverseSlider::SliderAttachment (valueTreeState, "lrWidth", slWidth));
    slWidth.setSliderStyle(Slider::Rotary);
    slWidth.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slWidth.setTextValueSuffix(" %");
    slWidth.setColour(Slider::rotarySliderOutlineColourId, colours[2]);
    slWidth.addListener(this);
    
    addAndMakeVisible(&slMidPattern);
    slAttMidPattern.reset(new ReverseSlider::SliderAttachment (valueTreeState, "msMidPattern", slMidPattern));
    slMidPattern.setSliderStyle(Slider::Rotary);
    slMidPattern.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
    slMidPattern.setColour(Slider::rotarySliderOutlineColourId, colours[2]);
    slMidPattern.addListener(this);
    
    // buttons
    addAndMakeVisible(&tbChSwitch);
    tbAttChSwitch.reset(new ButtonAttachment (valueTreeState, "channelSwitch", tbChSwitch));
    tbChSwitch.setButtonText("channel switch");
    
    addAndMakeVisible(&tbAutoLevels);
    tbAttAutoLevels.reset(new ButtonAttachment (valueTreeState, "autoLevelMode", tbAutoLevels));
    tbAutoLevels.setButtonText("auto levels");
    
    // group components and labels
    addAndMakeVisible(&grpStereoMode);
    grpStereoMode.setText("choose setup");
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
    
    addAndMakeVisible(&grpWidth);
    grpWidth.setText("width");
    grpWidth.setTextLabelPosition(Justification::centred);
    
    addAndMakeVisible(&grpMidPattern);
    grpMidPattern.setText("mid pattern");
    grpMidPattern.setTextLabelPosition(Justification::centred);

    
    // directivity visualiser
    addAndMakeVisible(&dirVis[0]);
    dirVis[0].setDirWeight(0.5f);
    dirVis[0].setColour(colours[0]);
    
    addAndMakeVisible(&dirVis[1]);
    dirVis[1].setDirWeight(-0.5f);
    dirVis[1].setColour(colours[1]);
    
    
    startTimer(30);
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
    
    
    g.drawImage(arrayImage, -25, 55, arrayImageArea.getWidth() + 80, currHeight - 50, 0, 0, arrayImage.getWidth(), arrayImage.getHeight());
    
    // background logo
    aaLogoBgPath.applyTransform (aaLogoBgPath.getTransformToScaleToFit (0.4f * currWidth, 0.25f * currHeight,
                                                                        0.7f * currWidth, 0.7f * currWidth, true, Justification::centred));
    g.setColour (Colours::white.withAlpha(0.1f));
    g.strokePath (aaLogoBgPath, PathStrokeType (0.1f));
    g.fillPath (aaLogoBgPath);
    
    if (processor.getNumInpCh() == 2)
    {
        switch (processor.getStereoModeIdx())
        {
            case msIdx:
                tbAutoLevels.setVisible(true);
                
                slMidGain[0].setVisible(true);
                grpMidGain[0].setVisible(true);
                slSideGain[0].setVisible(true);
                grpSideGain[0].setVisible(true);
                slMidGain[1].setVisible(false);
                grpMidGain[1].setVisible(false);
                slSideGain[1].setVisible(false);
                grpSideGain[1].setVisible(false);
                slWidth.setVisible(false);
                grpWidth.setVisible(false);
                slMidPattern.setVisible(false);
                grpMidPattern.setVisible(false);
           
                dirVis[0].setPatternRotation(90.0f);
                dirVis[0].setDirWeight(0.0f);
                dirVis[1].setPatternRotation(90.0f);
                dirVis[1].setDirWeight(1.0f);
                break;
            case passThroughIdx:
                
                dirVis[0].setPatternRotation(90.0f);
                dirVis[0].setDirWeight(- slWidth.getValue() / (2.0f * slWidth.getMaximum()));
                dirVis[1].setPatternRotation(90.0f);
                dirVis[1].setDirWeight(slWidth.getValue() / (2.0f * slWidth.getMaximum()));
//                setPolarPatterns(slWidth.getValue() / (2.0f * slWidth.getMaximum()));
                
                tbAutoLevels.setVisible(false);
                
                slMidGain[0].setVisible(false);
                grpMidGain[0].setVisible(false);
                slSideGain[0].setVisible(false);
                grpSideGain[0].setVisible(false);
                slMidGain[1].setVisible(false);
                grpMidGain[1].setVisible(false);
                slSideGain[1].setVisible(false);
                grpSideGain[1].setVisible(false);
                slWidth.setVisible(true);
                grpWidth.setVisible(true);
                slMidPattern.setVisible(false);
                grpMidPattern.setVisible(false);
                break;
                
            default:
                break;
        }
    }
    else
    {
        switch (processor.getStereoModeIdx())
        {
            case msIdx:
                tbAutoLevels.setVisible(true);
                
                slMidGain[0].setVisible(false);
                grpMidGain[0].setVisible(false);
                slSideGain[0].setVisible(false);
                grpSideGain[0].setVisible(false);
                slMidGain[1].setVisible(true);
                grpMidGain[1].setVisible(true);
                slSideGain[1].setVisible(true);
                grpSideGain[1].setVisible(true);
                slWidth.setVisible(false);
                grpWidth.setVisible(false);
                slMidPattern.setVisible(true);
                grpMidPattern.setVisible(true);
                
                dirVis[0].setPatternRotation(0.0f);
                dirVis[0].setDirWeight(- (slMidPattern.getValue() - 1.0f) / 2);
                dirVis[1].setPatternRotation(90.0f);
                dirVis[1].setDirWeight(1.0f);
                break;
            case passThroughIdx:
                dirVis[0].setPatternRotation(90.0f);
                dirVis[0].setDirWeight(- slWidth.getValue() / (2.0f * slWidth.getMaximum()));
                dirVis[1].setPatternRotation(90.0f);
                dirVis[1].setDirWeight(slWidth.getValue() / (2.0f * slWidth.getMaximum()));
                
                tbAutoLevels.setVisible(false);
                
                slMidGain[1].setVisible(false);
                grpMidGain[1].setVisible(false);
                slSideGain[1].setVisible(false);
                grpSideGain[1].setVisible(false);
                slWidth.setVisible(true);
                grpWidth.setVisible(true);
                slMidPattern.setVisible(false);
                grpMidPattern.setVisible(false);
                break;
                
            default:
                break;
        }
    }
    
}

void StereoCreatorAudioProcessorEditor::resized()
{

    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    const int topMargin = 10;
    const int rotarySliderHeight = 70;
    const int rotarySliderWidth = 80;
    const int grpHeight = 20;
    const int comboBoxWidth = 140;
    const int comboBoxHeight = 20;
    const int toggleBtHeight = 20;
    const int sideAreaWidth = comboBoxWidth;
    const int arrayWidth = 100;
    
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
    sideArea.removeFromTop(vSpace);
    tbAutoLevels.setBounds(sideArea.removeFromTop(toggleBtHeight));
    
    //--------------- MAIN AREA ----------------
    Rectangle<int> mainArea (area.removeFromRight(area.getWidth() - hSpace));
    Rectangle<int> mainArea4Ch = mainArea;
    
    Rectangle<int> rotarySlArea2Ch (mainArea.removeFromTop(rotarySliderHeight + grpHeight + vSpace));
    Rectangle<int> slLabelArea2Ch (rotarySlArea2Ch.removeFromTop(grpHeight));
    
    Rectangle<int> rotarySlArea4Ch (mainArea4Ch.removeFromTop(rotarySliderHeight + grpHeight + vSpace));
    Rectangle<int> slLabelArea4Ch (rotarySlArea4Ch.removeFromTop(grpHeight));
    
    slLabelArea2Ch.removeFromLeft((threeSlWidth / 2) - (twoSlWidth / 2));
    grpMidGain[0].setBounds(slLabelArea2Ch.removeFromLeft(rotarySliderWidth));
    slLabelArea2Ch.removeFromLeft(hSpace);
    grpSideGain[0].setBounds(slLabelArea2Ch.removeFromLeft(rotarySliderWidth));
    
    
    grpMidGain[1].setBounds (slLabelArea4Ch.removeFromLeft(rotarySliderWidth));
    slLabelArea4Ch.removeFromLeft(hSpace);
    grpSideGain[1].setBounds(slLabelArea4Ch.removeFromLeft(rotarySliderWidth));
    slLabelArea4Ch.removeFromLeft(hSpace);
    grpMidPattern.setBounds(slLabelArea4Ch.removeFromLeft(rotarySliderWidth));
    
    grpWidth.setBounds(grpSideGain[1].getBounds());
    
    rotarySlArea2Ch.removeFromTop(vSpace);
    rotarySlArea2Ch.removeFromLeft((threeSlWidth / 2) - (twoSlWidth / 2));
    slMidGain[0].setBounds(rotarySlArea2Ch.removeFromLeft(rotarySliderWidth));
    rotarySlArea2Ch.removeFromLeft(hSpace);
    slSideGain[0].setBounds(rotarySlArea2Ch.removeFromLeft(rotarySliderWidth));
    
    rotarySlArea4Ch.removeFromTop(vSpace);
    slMidGain[1].setBounds (rotarySlArea4Ch.removeFromLeft(rotarySliderWidth));
    rotarySlArea4Ch.removeFromLeft(hSpace);
    slSideGain[1].setBounds(rotarySlArea4Ch.removeFromLeft(rotarySliderWidth));
    rotarySlArea4Ch.removeFromLeft(hSpace);
    slMidPattern.setBounds(rotarySlArea4Ch.removeFromLeft(rotarySliderWidth));
    
    slWidth.setBounds(slSideGain[1].getBounds());

    // directivity visualiser
    Rectangle<int> dirVisArea (mainArea4Ch.removeFromLeft(threeSlWidth));
    dirVis[0].setBounds(dirVisArea);
    dirVis[1].setBounds(dirVisArea);
    
    
    
    
}

void StereoCreatorAudioProcessorEditor::comboBoxChanged(ComboBox *cb)
{
    repaint();
}

void StereoCreatorAudioProcessorEditor::sliderValueChanged(Slider *slider)
{

}

void StereoCreatorAudioProcessorEditor::timerCallback()
{
//    if (processor.wrongBusConfiguration.get())
//    {
//        title.setAlertMessage(wrongBusConfigMessageShort, wrongBusConfigMessageLong);
//        title.showAlertSymbol(true);
//        return;
//    }
//
//    title.showAlertSymbol(false);
}


