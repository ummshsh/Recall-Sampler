#pragma once
#include <JuceHeader.h>

struct ColourPalette
{
    // A single, unified background colour for the entire plugin
    juce::Colour appBackground{ juce::Colour::fromRGB(237, 245, 255) };

    // Colours for the FlashbackVisualiser
    juce::Colour visBackground{ juce::Colour::fromRGB(227, 235, 255) };
    juce::Colour visWaveformBody{ juce::Colour::fromRGB(67, 146, 241) };
    juce::Colour visWaveformOutline{ juce::Colour::fromRGB(67, 118, 224) };
    juce::Colour visCursor{ juce::Colour::fromRGB(67, 118, 224) };
    juce::Colour visSelection{ juce::Colour::fromString("#FF4299e1").withAlpha(0.4f) };

    juce::Colour controlText{ juce::Colour::fromRGB(67, 118, 224)};

    // Colours for the Freeze Button
    juce::Colour freezeButtonOn{ juce::Colour::fromRGB(245, 93, 62)};
    juce::Colour freezeButtonOff{ juce::Colour::fromString("#FF718096") };

    juce::Colour controlBorder{ juce::Colour::fromRGB(67, 118, 224)};
};