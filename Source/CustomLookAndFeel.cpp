/*
  ==============================================================================

    CustomLookAndFeel.cpp
    Created: 8 Aug 2021 3:09:40pm
    Author:  Hallvard Jensen

  ==============================================================================
*/

#include "CustomLookAndFeel.h"

using namespace anyMidi;

CustomLookaAndFeel::CustomLookaAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::yellow); // remove
    
    // TABS
    setColour(juce::TabbedComponent::outlineColourId, dimWhite);
    setColour(juce::TabbedComponent::backgroundColourId, black);

    setColour(juce::TabbedButtonBar::tabOutlineColourId, dimWhite);
    setColour(juce::TabbedButtonBar::tabTextColourId, dimWhite);
    setColour(juce::TabbedButtonBar::frontTextColourId, white);
    setColour(juce::TabbedButtonBar::frontOutlineColourId, white);

    // LABEL
    setColour(juce::Label::textColourId, white);

    // SLIDER
    setColour(juce::Slider::textBoxTextColourId, turquoise);
    setColour(juce::Slider::trackColourId, turquoise);
    setColour(juce::Slider::textBoxBackgroundColourId, gray);

    // BUTTON
    setColour(juce::TextButton::textColourOnId, turquoise);
    setColour(juce::TextButton::textColourOffId, white);
    setColour(juce::TextButton::buttonColourId, gray);
    setColour(juce::TextButton::buttonOnColourId, gold);

    // DROPDOWN MENU
    setColour(juce::ComboBox::textColourId, white);
    setColour(juce::ComboBox::backgroundColourId, gray);
    //setColour(juce::ComboBox::buttonColourId, gray);
    setColour(juce::ComboBox::arrowColourId, turquoise);

    setColour(juce::PopupMenu::backgroundColourId, gray);

    // Checkboxes
    setColour(juce::ListBox::backgroundColourId, gray);

    // TEXT BOX
    setColour(juce::TextEditor::textColourId, white);
    setColour(juce::TextEditor::backgroundColourId, gray);
}