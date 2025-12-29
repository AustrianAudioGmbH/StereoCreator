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
#include "../../resources/customComponents/ToggleButton.hpp"
#include "../../resources/lookAndFeel/BinaryFonts.h"

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
        using namespace juce;

        auto fontOptions =
            FontOptions (Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansRegular_ttf,
                                                            BinaryFonts::NunitoSansRegular_ttfSize))
                .withHeight (17.0f);

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

        addAndMakeVisible (lrSwapLabel);
        lrSwapLabel.setText ("L/R channel swap", NotificationType::dontSendNotification);
        lrSwapLabel.setFont (fontOptions);

        addAndMakeVisible (lrSwapButton);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (10);
        area.removeFromTop (textMarginTop);
        stereoMode.setBounds (area.removeFromTop (20));
        area.removeFromTop (10);
        lrSwapLabel.setBounds (area.removeFromLeft (area.getWidth() - 35));
        lrSwapButton.setBounds (area.removeFromRight (30).withSizeKeepingCentre (30, 20));
    }

    ComboBox stereoMode;
    juce::Label lrSwapLabel;
    ToggleButton lrSwapButton;
};
} // namespace AAGuiComponents
