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

class anyMidiStandaloneApplication : public juce::JUCEApplication {
public:
    anyMidiStandaloneApplication() : tree{anyMidi::ROOT_ID} {}

    const juce::String getApplicationName() override {
        return ProjectInfo::projectName;
    }
    const juce::String getApplicationVersion() override {
        return ProjectInfo::versionString;
    }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String &commandLine) override {
        juce::ValueTree audioProcNode(anyMidi::AUDIO_PROC_ID);
        tree.addChild(audioProcNode, -1, nullptr);

        juce::ValueTree guiNode{anyMidi::GUI_ID};
        tree.addChild(guiNode, -1, nullptr);

        audioProcessor = std::make_unique<anyMidi::AudioProcessor>(tree);
        mainWindow.reset(
            new MainWindow(getApplicationName(), &layout, guiNode));
        tray = std::make_unique<anyMidi::TrayIcon>(mainWindow.get());
    }

    void shutdown() override { mainWindow = nullptr; }

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
        MainWindow(juce::String name, anyMidi::CustomLookaAndFeel *layout,
                   juce::ValueTree v)
            : DocumentWindow(
                  name,
                  layout->findColour(juce::DocumentWindow::backgroundColourId),
                  DocumentWindow::minimiseButton |
                      DocumentWindow::closeButton) {
            setUsingNativeTitleBar(false);
            setTitleBarTextCentred(false);
            setContentOwned(new anyMidi::MainComponent(v), true);
#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#else
            setResizable(false, false);
            centreWithSize(getWidth(), getHeight());
#endif

            setLookAndFeel(layout);

            // Draw task bar icon
            juce::Image largeIcon = juce::ImageCache::getFromMemory(
                BinaryData::anyMidiLogo_png, BinaryData::anyMidiLogo_pngSize);
            juce::Graphics gL{largeIcon};
            gL.drawImage(largeIcon, juce::Rectangle<float>(470, 470));
            getPeer()->setIcon(largeIcon);

            // Draw window title header icon
            juce::Image smallIcon = juce::ImageCache::getFromMemory(
                BinaryData::anyMidiLogoSmall_png,
                BinaryData::anyMidiLogoSmall_pngSize);
            juce::Graphics gS{smallIcon};
            gS.drawImage(smallIcon, juce::Rectangle<float>(16, 16));
            setIcon(smallIcon);

            setVisible(true);
        }

        ~MainWindow() { setLookAndFeel(nullptr); }

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
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    anyMidi::CustomLookaAndFeel layout;

    std::unique_ptr<anyMidi::AudioProcessor> audioProcessor;
    std::shared_ptr<MainWindow> mainWindow;
    std::unique_ptr<anyMidi::TrayIcon> tray;
    juce::ValueTree tree;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(anyMidiStandaloneApplication)
};

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(anyMidiStandaloneApplication)
