/**
 *
 *  @file      Main.cpp
 *  @brief     Start-up code for JUCE application.
 *  @author    Hallvard Jensen
 *  @date      13 Feb 2021 8:29:21 pm
 *  @copyright © Hallvard Jensen, 2021. All right reserved.
 *
 */


#include <JuceHeader.h>
#include "./ui/MainComponent.h"
#include "./core/AudioProcessor.h"
#include "./util/Globals.h"

class anyMidiStandaloneApplication : public juce::JUCEApplication
{

public:

    anyMidiStandaloneApplication() : tree{anyMidi::ROOT_ID} {}

    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }


    void initialise(const juce::String& commandLine) override
    {
        juce::ValueTree audioProcNode(anyMidi::AUDIO_PROC_ID);
        tree.addChild(audioProcNode, -1, nullptr);
        
        juce::ValueTree guiNode{ anyMidi::GUI_ID };
        tree.addChild(guiNode, -1, nullptr);

        audioProcessor = std::make_unique<anyMidi::AudioProcessor>(tree);
        mainWindow.reset(new MainWindow(getApplicationName(), guiNode));
        tray = std::make_unique<anyMidi::TrayIcon>(mainWindow.get());
    }


    void shutdown() override
    {
        mainWindow = nullptr; // (deletes our window)
    }


    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }


    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }


    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow : public juce::DocumentWindow
    {

    public:

        MainWindow(juce::String name, juce::ValueTree v)
            : DocumentWindow(name,
                juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(juce::DocumentWindow::backgroundColourId),
                DocumentWindow::minimiseButton | DocumentWindow::closeButton)
        {
            setUsingNativeTitleBar(false);
            setTitleBarTextCentred(false);
            setContentOwned(new anyMidi::MainComponent(v), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#else
            setResizable(false, false);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);
        }


        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        void minimiseButtonPressed() override
        {
            this->removeFromDesktop();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */


    private:

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)

    };


private:
    std::unique_ptr<anyMidi::AudioProcessor> audioProcessor;
    std::shared_ptr<MainWindow> mainWindow;
    std::unique_ptr<anyMidi::TrayIcon> tray;
    juce::ValueTree tree;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(anyMidiStandaloneApplication)
};


// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(anyMidiStandaloneApplication)
