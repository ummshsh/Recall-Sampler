#pragma once
#include <JuceHeader.h>


struct ColourPalette
{
    // Colours for the main editor background
    juce::Colour editorBackground{ juce::Colours::white };

    // Colours for the FlashbackVisualiser
    juce::Colour visBackground{ juce::Colour::fromRGB(216, 219, 226) };
    juce::Colour visWaveform{ juce::Colour::fromRGB(55, 63, 81) };
    juce::Colour visCursor{ juce::Colour::fromRGB(88, 164, 176) };
    juce::Colour visSelection{ juce::Colours::black.withAlpha(0.3f) };

    // Colours for the DraggableNumberBox
    juce::Colour numBoxBackground{ juce::Colour::fromRGBA(91, 192, 235,0.5f) };
    juce::Colour numBoxBorder{ juce::Colours::transparentWhite };
    juce::Colour numBoxText{ juce::Colour::fromRGB(88, 164, 176)};

    // Colours for the Freeze Button
    juce::Colour freezeButtonOn{ juce::Colour::fromRGB(88, 164, 176) };
    juce::Colour freezeButtonOff{ juce::Colours::grey };
};
