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

#include "../../resources/customComponents/ComboBox.hpp"
#include "../../resources/customComponents/GroupComponent.hpp"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{
struct Setup : public GroupComponent
{
    enum StereoMode
    {
        pseudoMsIdx = 1,
        pseudoStereoIdx = 2,
        trueMsIdx = 3,
        trueStereoIdx = 4,
        blumleinIdx = 5
    };

    Setup()
    {
        setText ("Setup");

        addAndMakeVisible (stereoMode);
        stereoMode.addItemList (
            {
                "pseudo-m/s",
                "pseudo-stereo",
                "true-m/s",
                "true-stereo",
                "blumlein",
            },
            1);
        stereoMode.setEditableText (false);

        addAndMakeVisible (lrSwapButton);
        // TODO: restyle the L/R button to be consistent with PD3
        lrSwapButton.setButtonText ("L/R channel swap");
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (10);
        area.removeFromTop (textMarginTop);
        stereoMode.setBounds (area.removeFromTop (20));
        area.removeFromTop (10);
        lrSwapButton.setBounds (area.removeFromTop (20));
    }

    ComboBox stereoMode;
    juce::ToggleButton lrSwapButton;
};
} // namespace AAGuiComponents
