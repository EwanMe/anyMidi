/**
 *
 *  @file      CustomLookAndFeel.cpp
 *  @author    Hallvard Jensen
 *  @date      8 Aug 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#include "CustomLookAndFeel.h"

anyMidi::CustomLookAndFeel::CustomLookAndFeel() {
    setColour(juce::DocumentWindow::ColourIds::textColourId,
              palette_.at(ColorGroup::TextPrimary));
    setColour(juce::DocumentWindow::backgroundColourId,
              palette_.at(ColorGroup::BackgroundPrimary));

    // Tabs
    setColour(juce::TabbedComponent::ColourIds::outlineColourId,
              palette_.at(ColorGroup::BackgroundPrimary));
    setColour(juce::TabbedComponent::ColourIds::backgroundColourId,
              palette_.at(ColorGroup::BackgroundPrimary));

    setColour(juce::TabbedButtonBar::ColourIds::tabOutlineColourId,
              palette_.at(ColorGroup::Outline));
    setColour(juce::TabbedButtonBar::ColourIds::tabTextColourId,
              palette_.at(ColorGroup::TextSecondary));
    setColour(juce::TabbedButtonBar::ColourIds::frontTextColourId,
              palette_.at(ColorGroup::TextPrimary));

    // Label
    setColour(juce::Label::ColourIds::textColourId,
              palette_.at(ColorGroup::TextPrimary));

    // Slider
    setColour(juce::Slider::ColourIds::textBoxTextColourId,
              palette_.at(ColorGroup::Active)); // Text sliders / boxes
    setColour(juce::Slider::ColourIds::trackColourId,
              palette_.at(ColorGroup::Active));
    setColour(juce::Slider::ColourIds::backgroundColourId,
              palette_.at(ColorGroup::BackgroundSecondary));
    setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
              palette_.at(ColorGroup::Outline));
    setColour(juce::Slider::ColourIds::thumbColourId,
              palette_.at(ColorGroup::Active));

    // Button
    setColour(juce::TextButton::ColourIds::textColourOffId,
              palette_.at(ColorGroup::TextPrimary));
    setColour(juce::TextButton::ColourIds::buttonColourId,
              palette_.at(ColorGroup::BackgroundPrimary));
    setColour(juce::TextButton::ColourIds::buttonOnColourId,
              palette_.at(ColorGroup::BackgroundSecondary));

    // Dropdown menu
    setColour(juce::ComboBox::ColourIds::textColourId,
              palette_.at(ColorGroup::TextPrimary));
    setColour(juce::ComboBox::ColourIds::backgroundColourId,
              palette_.at(ColorGroup::BackgroundPrimary));
    setColour(juce::ComboBox::ColourIds::arrowColourId,
              palette_.at(ColorGroup::TextPrimary));
    setColour(juce::ComboBox::ColourIds::outlineColourId,
              palette_.at(ColorGroup::Outline));

    setColour(juce::PopupMenu::ColourIds::backgroundColourId,
              palette_.at(ColorGroup::BackgroundPrimary));
    setColour(juce::PopupMenu::ColourIds::textColourId,
              palette_.at(ColorGroup::TextPrimary));
    setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId,
              palette_.at(ColorGroup::Active));
    setColour(juce::PopupMenu::ColourIds::highlightedTextColourId,
              palette_.at(ColorGroup::BackgroundSecondary));

    // Checkboxes
    setColour(juce::ListBox::ColourIds::backgroundColourId,
              palette_.at(ColorGroup::BackgroundPrimary));
    setColour(juce::ListBox::ColourIds::textColourId,
              palette_.at(ColorGroup::TextPrimary));
    setColour(juce::ListBox::ColourIds::outlineColourId,
              palette_.at(ColorGroup::Outline));

    setColour(juce::ToggleButton::ColourIds::textColourId,
              palette_.at(ColorGroup::TextPrimary));
    setColour(juce::ToggleButton::ColourIds::tickColourId,
              palette_.at(ColorGroup::TextPrimary));
    setColour(juce::ToggleButton::ColourIds::tickDisabledColourId,
              palette_.at(ColorGroup::Outline));

    // Text box
    setColour(juce::TextEditor::ColourIds::backgroundColourId,
              palette_.at(ColorGroup::BackgroundPrimary));
    setColour(juce::TextEditor::ColourIds::outlineColourId,
              palette_.at(ColorGroup::Outline));
    setColour(juce::TextEditor::ColourIds::highlightColourId,
              palette_.at(ColorGroup::Active));
    setColour(juce::TextEditor::ColourIds::highlightedTextColourId,
              palette_.at(ColorGroup::BackgroundPrimary));
    setColour(juce::TextEditor::ColourIds::textColourId,
              palette_.at(ColorGroup::TextPrimary));

    // Caret
    setColour(juce::CaretComponent::ColourIds::caretColourId,
              palette_.at(ColorGroup::Active));

    // Popup menu
    setColour(juce::PopupMenu::ColourIds::backgroundColourId,
              palette_.at(ColorGroup::BackgroundPrimary));
    setColour(juce::PopupMenu::ColourIds::textColourId,
              palette_.at(ColorGroup::TextPrimary));
    setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId,
              palette_.at(ColorGroup::Active));
    setColour(juce::PopupMenu::ColourIds::highlightedTextColourId,
              palette_.at(ColorGroup::BackgroundPrimary));
    setColour(juce::PopupMenu::ColourIds::headerTextColourId,
              juce::Colours::red);
}