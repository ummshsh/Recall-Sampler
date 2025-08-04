#pragma once

#include <JuceHeader.h>
#include "ColourPalette.cpp"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel(const ColourPalette& p) : palette(p) {}

    void drawDrawableButton(juce::Graphics& g, juce::DrawableButton& button,
        bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        const float cornerRadius = 8.0f;

        // Draw the background/border based on the toggle state
        if (button.getToggleState())
        {
            // "On" state: Filled blue background
            g.setColour(palette.freezeButtonOn);
            //g.fillRoundedRectangle(bounds, cornerRadius);
        }
        else
        {
            // "Off" state: White fill with a grey border
            g.setColour(juce::Colours::white);
            //g.fillRoundedRectangle(bounds, cornerRadius);
            g.setColour(palette.controlBorder);
            //g.drawRoundedRectangle(bounds, cornerRadius, 1.5f);
        }

        // Draw the icon itself on top
        auto* icon = button.getCurrentImage();
        if (icon != nullptr)
        {
            // Reduce the icon area slightly to give it padding
            auto iconArea = bounds.reduced(button.getBounds().getWidth() * 0.25f);
            icon->drawWithin(g, iconArea, juce::RectanglePlacement::centred, 1.0f);
        }
    }

private:
    const ColourPalette& palette;
};