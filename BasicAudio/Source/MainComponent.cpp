// Music 256a / CS 476a | fall 2016
// CCRMA, Stanford University
//
// Author: Romain Michon (rmichonATccrmaDOTstanfordDOTedu)
// Description: Simple JUCE sine wave synthesizer

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Sine.h"
#include "Smooth.h"

class MainContentComponent :
    public AudioAppComponent,
    private Slider::Listener,
    private ToggleButton::Listener
{
public:
    MainContentComponent() : ratio (1.0), onOff (0), samplingRate(0.0)
    {
        // add synth header
        
        addAndMakeVisible(mainLabel);
        mainLabel.setText("Graham's Additive Synth", dontSendNotification);
        mainLabel.setColour(Label::ColourIds::textColourId, Colour::fromRGB(193, 193, 217));
        
        // configure gain sliders for the 4 sine waves
        
        for (int i=0; i<4; i++) {
            addAndMakeVisible(gainSliders[i]);
            gainSliders[i].setRange(0.0, 1.0);
            gainSliders[i].setValue(0.5);
            gainSliders[i].setSliderStyle(Slider::SliderStyle::LinearBarVertical);
            gainSliders[i].setColour(Slider::ColourIds::textBoxTextColourId, Colours::darkgrey);
            gainSliders[i].setColour(Slider::ColourIds::backgroundColourId, Colours::grey);
            gainSliders[i].setTextBoxIsEditable(false);
            gainSliders[i].addListener(this);
        }
        // configuring frequency slider and adding it to the main window
        addAndMakeVisible (frequencySlider);
        frequencySlider.setRange (50.0, 5000.0);
        frequencySlider.setSkewFactorFromMidPoint (500.0);
        frequencySlider.setValue(100); // will also set the default frequency of the sine osc
        frequencySlider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        frequencySlider.setColour(Slider::ColourIds::rotarySliderFillColourId, Colour::fromRGB(193, 193, 217));
        frequencySlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);

        frequencySlider.addListener (this);
        
        // configuring frequency label box and adding it to the main window
        addAndMakeVisible(frequencyLabel);
        frequencyLabel.setText ("Freq", dontSendNotification);
        frequencyLabel.setColour(Label::ColourIds::textColourId, Colour::fromRGB(193, 193, 217));
        frequencyLabel.attachToComponent (&frequencySlider, true);
        
        // configure ratio slider and add it to main window
        addAndMakeVisible (ratioSlider);
        ratioSlider.setRange (1.0, 2.0);
        ratioSlider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        ratioSlider.setValue(1);
        ratioSlider.setColour(Slider::ColourIds::rotarySliderFillColourId, Colour::fromRGB(193, 193, 217));
        ratioSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
        ratioSlider.addListener (this);
        
        addAndMakeVisible(ratioLabel);
        ratioLabel.setText ("Ratio", dontSendNotification);
        ratioLabel.setColour(Label::ColourIds::textColourId, Colour::fromRGB(193, 193, 217));
        ratioLabel.attachToComponent (&ratioSlider, true);
        
        // configure gain slider and add it to main window
        
        addAndMakeVisible(masterGainSlider);
        masterGainSlider.setRange(0.0, 1.0);
        masterGainSlider.setValue(0.5);
        masterGainSlider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        masterGainSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
        masterGainSlider.setColour(Slider::ColourIds::rotarySliderFillColourId, Colour::fromRGB(193, 193, 217));
        masterGainSlider.addListener(this);
        
        addAndMakeVisible(gainLabel);
        gainLabel.setText("Gain", dontSendNotification);
        gainLabel.setColour(Label::ColourIds::textColourId, Colour::fromRGB(193, 193, 217));
        gainLabel.attachToComponent (&masterGainSlider, true);
        
        // configure pan slider and add it to main window
        
        addAndMakeVisible(panSlider);
        panSlider.setRange(0.0, 1.0);
        panSlider.setValue(0.5);
        panSlider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        panSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
        panSlider.setColour(Slider::ColourIds::rotarySliderFillColourId, Colour::fromRGB(193, 193, 217));
        panSlider.addListener(this);
        
        addAndMakeVisible(panLabel);
        panLabel.setText("Pan", dontSendNotification);
        panLabel.setColour(Label::ColourIds::textColourId, Colour::fromRGB(193, 193, 217));
        panLabel.attachToComponent (&panSlider, true);
        
        // configuring on/off button and adding it to the main window
        addAndMakeVisible(onOffButton);
        onOffButton.addListener(this);
        
        
        // configuring on/off label and adding it to the main window
        addAndMakeVisible(onOffLabel);
        onOffLabel.setText ("On/Off", dontSendNotification);
        onOffLabel.setColour(Label::ColourIds::textColourId, Colour::fromRGB(193, 193, 217));
        onOffLabel.attachToComponent (&onOffButton, true);
        
        setSize (550, 300);
        nChans = 2;
        setAudioChannels (0, nChans); // no inputs, one output
    }
    
    ~MainContentComponent()
    {
        shutdownAudio();
    }
    
    void resized() override
    {
        // placing the UI elements in the main window
        // getWidth has to be used in case the window is resized by the user
        mainLabel.setBounds((getWidth()/2) - 70, 15, getWidth(), 20);
        onOffButton.setBounds (75, 60, 30, 20);
        masterGainSlider.setBounds(65, 95, 40, 40);
        frequencySlider.setBounds (65, 145, 40, 40);
        ratioSlider.setBounds (65, 195, 40, 40);
        panSlider.setBounds(65, 245, 40, 40);

        for (int i=0; i<4; i++) {
            gainSliders[i].setBounds(140 + (i*100), 60, 80, 220);
        }
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        if (samplingRate > 0.0){
            if (slider == &frequencySlider){
                double newFreq = frequencySlider.getValue();
                for (int i=0; i<4; i++) {
                    sines[i].setFrequency(newFreq + (newFreq * i * (ratio-1)));
                }
            } else if (slider == &ratioSlider) {
                ratio = ratioSlider.getValue();
                double newFreq = frequencySlider.getValue();
                for (int i=0; i<4; i++) {
                    sines[i].setFrequency(newFreq + (newFreq * i * (ratio-1)));
                }
            } else if (slider == &masterGainSlider) {
                masterGain = masterGainSlider.getValue();
            } else if (slider == &panSlider) {
                pan = panSlider.getValue();
            } else {
                for (int i=0; i<4; i++) {
                    if (slider == &gainSliders[i]) {
                        gains[i] = gainSliders[i].getValue();
                    }
                }
            }
        }
    }
    
    void buttonClicked (Button* button) override
    {
        // turns audio on or off
        if(button == &onOffButton && onOffButton.getToggleState()){
            onOff = 1;
        }
        else{
            onOff = 0;
        }
    }
    
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        samplingRate = sampleRate;
        for (int i=0; i<4; i++) {
            sines[i].setSamplingRate(sampleRate);
        }
        for(int i=0; i<8; i++){
            smooth[i].setSmooth(0.999);
        }
    }
    
    void releaseResources() override
    {
    }
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // getting the audio output buffer to be filled
        float* const buffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        float* const buffer2 = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

        
        // computing one block
        for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            double newSample = 0;
            for (int i=0; i<4; i++) {
                newSample += sines[i].tick() * smooth[i].tick(gains[i]);
            }
            newSample = (newSample / 4)*smooth[5].tick(masterGain)*smooth[4].tick(onOff);
            buffer[sample] = newSample * (1.0-smooth[6].tick(pan));
            buffer2[sample] = newSample * smooth[7].tick(pan);
        }
    }
    
    
private:
    // UI Elements
    Slider frequencySlider, ratioSlider, gainSliders[4], masterGainSlider, panSlider;
    ToggleButton onOffButton;
    
    Label frequencyLabel, onOffLabel, ratioLabel, gainLabel, panLabel, mainLabel;
    
    Sine sines[4]; // the sine wave oscillators
    
    Smooth smooth[8]; // the smoothers for interpolation
    
    // Global Variables
    float gains[4], ratio, pan, masterGain;
    int onOff, samplingRate, nChans;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
