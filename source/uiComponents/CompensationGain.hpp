
/*
 ==============================================================================
 Author: Sebastian Grill
 
 Copyright (c) 2025 - Austrian Audio GmbH
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

#include "../../resources/customComponents/Colours.hpp"
#include "../../resources/customComponents/GroupComponent.hpp"
#include "../../resources/customComponents/TextButton.hpp"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{

struct CompensationGain : public GroupComponent
{
    CompensationGain()
    {
        using namespace juce;

        setText ("Compensation Gain");
        for (auto& s : sliders)
        {
            addAndMakeVisible (s);
            s.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
            s.setColour (Slider::rotarySliderOutlineColourId, Colours::polarVisualizerRed);
            s.setTextValueSuffix (" dB");
            s.setTextBoxStyle (Slider::TextBoxBelow, false, 60, 15);
        }
        addAndMakeVisible (button);
        button.setButtonText ("Calculate");
    }

    void enableSlider (int slider)
    {
        for (auto& s : sliders)
        {
            s.setEnabled (false);
            s.setVisible (false);
        }

        sliders[slider].setEnabled (true);
        sliders[slider].setVisible (true);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (10);
        area.removeFromTop (textMarginTop);
        auto sliderArea = area.removeFromLeft (60);
        for (auto& s : sliders)
            s.setBounds (sliderArea);

        area.reduce (0, 15);
        button.setBounds (area.removeFromRight (area.getWidth() - 10));
    }

    juce::Slider sliders[5];
    TextButton<ButtonColor::gray> button;
};
} // namespace AAGuiComponents
