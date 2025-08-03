#pragma once
#include <JuceHeader.h>

struct ColourPalette
{
    // A single, unified background colour for the entire plugin
    juce::Colour appBackground{ juce::Colour::fromRGB(241,245,249) };

    // Colours for the FlashbackVisualiser
    juce::Colour visBackground{ juce::Colour::fromString("#FFE2E8F0") };
    juce::Colour visWaveformBody{ juce::Colour::fromString("#FF8d99ab") };
    juce::Colour visWaveformOutline{ juce::Colour::fromString("#FF718096") };
    juce::Colour visCursor{ juce::Colour::fromString("#FF4299e1") };
    juce::Colour visSelection{ juce::Colour::fromString("#FF4299e1").withAlpha(0.4f) };

    juce::Colour controlText{ juce::Colour::fromString("#FF2d3748") };

    // Colours for the Freeze Button
    juce::Colour freezeButtonOn{ juce::Colour::fromString("#FF1C67FC") };
    juce::Colour freezeButtonOff{ juce::Colour::fromString("#FF718096") };

    juce::Colour controlBorder{ juce::Colour::fromString("#FFCBD5E0") };
};