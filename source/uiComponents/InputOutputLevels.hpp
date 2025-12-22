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
#include "../../resources/customComponents/LevelMeter.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{
struct InputOutputLevels : public GroupComponent
{
    InputOutputLevels()
    {
        setText ("Input - Output Levels");

        for (size_t i = 0; i < 4; ++i)
        {
            addAndMakeVisible (inputMeter[i]);
            inputMeter[i].setLabelText (inMeterLabelText[i]);
            inputMeter[i].setColour (Colours::polarVisualizerRed);
        }

        for (size_t i = 0; i < 2; ++i)
        {
            addAndMakeVisible (outputMeter[i]);
            outputMeter[i].setLabelText (outMeterLabelText[i]);
            outputMeter[i].setColour (Colours::polarVisualizerRed);
        }
    }

    void setQuadChannelInputMode (bool isQuad)
    {
        if (isQuad)
        {
            inputMeter[2].setVisible (true);
            inputMeter[2].setEnabled (true);
            inputMeter[3].setVisible (true);
            inputMeter[3].setEnabled (true);
        }
        else
        {
            inputMeter[2].setVisible (false);
            inputMeter[2].setEnabled (false);
            inputMeter[3].setVisible (false);
            inputMeter[3].setEnabled (false);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (10);
        constexpr auto meterWidth = 12;
        constexpr auto meterSpacing = 10;

        area.removeFromTop (textMarginTop);

        for (auto& m : inputMeter)
        {
            m.setBounds (area.removeFromLeft (meterWidth));
            area.removeFromLeft (meterSpacing);
        }

        outputMeter[1].setBounds (area.removeFromRight (meterWidth));
        area.removeFromRight (meterSpacing);
        outputMeter[0].setBounds (area.removeFromRight (meterWidth));
    }

    LevelMeter inputMeter[4];
    LevelMeter outputMeter[2];

private:
    static constexpr std::array<std::string, 4> inMeterLabelText = { "L", "R", "F", "B" };
    static constexpr std::array<std::string, 2> outMeterLabelText = { "L", "R" };
};
} // namespace AAGuiComponents
