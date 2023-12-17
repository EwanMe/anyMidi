/**
 *
 *  @file      Main.cpp
 *  @brief     Start-up code for JUCE application.
 *  @author    Hallvard Jensen
 *  @date      13 Feb 2021
 *  @copyright Hallvard Jensen, 2021. All right reserved.
 *
 */

#include <JuceHeader.h>

#include "./core/AudioProcessor.h"
#include "./ui/CustomLookAndFeel.h"
#include "./ui/MainComponent.h"
#include "./util/Globals.h"

class AnyMidiStandaloneApplication : public juce::JUCEApplication {
public:
    AnyMidiStandaloneApplication() = default;

    const juce::String getApplicationName() override {
        return ProjectInfo::projectName;
    }
    const juce::String getApplicationVersion() override {
        return ProjectInfo::versionString;
    }

    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise([[maybe_unused]] const juce::String &commandLine) override {
        const juce::ValueTree audioProcNode(anyMidi::AUDIO_PROC_ID);
        tree_.addChild(audioProcNode, -1, nullptr);

        const juce::ValueTree guiNode{anyMidi::GUI_ID};
        tree_.addChild(guiNode, -1, nullptr);

        audioProcessor_ = std::make_unique<anyMidi::AudioProcessor>(
            anyMidi::defaultSampleRate, tree_);
        mainWindow_ = std::make_shared<MainWindow>(getApplicationName(),
                                                   &layout_, guiNode);
        tray_ = std::make_unique<anyMidi::TrayIcon>(mainWindow_.get());
    }

    void shutdown() override { mainWindow_ = nullptr; }

    void systemRequestedQuit() override { quit(); }

    void anotherInstanceStarted(const juce::String &commandLine) override {
        // When another instance of the app is launched while this one is
        // running, this method is invoked, and the commandLine parameter tells
        // you what the other instance's command-line arguments were.
    }

    /*
        This class implements the desktop window that contains an instance of
        the MainComponent class.
    */
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(const juce::String &name, anyMidi::CustomLookAndFeel *layout,
                   const juce::ValueTree &v)
            : DocumentWindow(
                  name,
                  layout->findColour(juce::DocumentWindow::backgroundColourId),
                  DocumentWindow::minimiseButton |
                      DocumentWindow::closeButton) {
            setUsingNativeTitleBar(false);
            setTitleBarTextCentred(false);
            
            // JUCE handles pointer safety
            setContentOwned(new anyMidi::MainComponent(v), true); // NOLINT
            
            setResizable(false, false);
            centreWithSize(getWidth(), getHeight());
            setLookAndFeel(layout);

            // Draw task bar icon
            const juce::Image largeIcon = juce::ImageCache::getFromMemory(
                BinaryData::anyMidiLogo_png, BinaryData::anyMidiLogo_pngSize);
            const juce::Graphics gL{largeIcon};
            gL.drawImage(largeIcon,
                         juce::Rectangle<float>(LargeIconSize, LargeIconSize));
            getPeer()->setIcon(largeIcon);

            // Draw window title header icon
            const juce::Image smallIcon = juce::ImageCache::getFromMemory(
                BinaryData::anyMidiLogoSmall_png,
                BinaryData::anyMidiLogoSmall_pngSize);
            const juce::Graphics gS{smallIcon};
            gS.drawImage(smallIcon,
                         juce::Rectangle<float>(SmallIconSize, SmallIconSize));
            setIcon(smallIcon);

            setVisible(true);
        }

        ~MainWindow() override { setLookAndFeel(nullptr); }

        void closeButtonPressed() override {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        void minimiseButtonPressed() override {
            // The application is completely removed from the desktop and task
            // bar, leaving only the system tray icon accessible.
            setVisible(false);
        }

        // int getDesktopWindowStyleFlags() const override
        //{
        //     int styleFlags = ResizableWindow::getDesktopWindowStyleFlags();

        //    // Removes application from taskbar.
        //    return styleFlags ^
        //    juce::ComponentPeer::StyleFlags::windowAppearsOnTaskbar;
        //}

    private:
        static const int SmallIconSize{16};
        static const int LargeIconSize{470};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    anyMidi::CustomLookAndFeel layout_;

    std::unique_ptr<anyMidi::AudioProcessor> audioProcessor_;
    std::shared_ptr<MainWindow> mainWindow_;
    std::unique_ptr<anyMidi::TrayIcon> tray_;
    juce::ValueTree tree_{anyMidi::ROOT_ID};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnyMidiStandaloneApplication)
};

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(AnyMidiStandaloneApplication)
