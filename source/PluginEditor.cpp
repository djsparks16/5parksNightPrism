#include "PluginEditor.h"


namespace
{
    using namespace juce;
    Colour bg()        { return Colour(0xff060810); }
    Colour panel()     { return Colour(0xff0d1320); }
    Colour panel2()    { return Colour(0xff121a2d); }
    Colour edge()      { return Colour(0x55b8f8ff); }
    Colour accent()    { return Colour(0xff3ae7ff); }
    Colour accent2()   { return Colour(0xff9f5cff); }
    Colour accent3()   { return Colour(0xffff6bb8); }
    Colour accent4()   { return Colour(0xff53ffc4); }
    Colour textHi()    { return Colours::white.withAlpha(0.96f); }
    Colour textLo()    { return Colours::white.withAlpha(0.68f); }

    void drawCard(Graphics& g, Rectangle<float> r, const String& title)
    {
        auto outer = r.expanded(2.0f);
        ColourGradient glow(accent().withAlpha(0.10f), outer.getTopLeft(), accent2().withAlpha(0.06f), outer.getBottomRight(), false);
        glow.addColour(0.65, accent3().withAlpha(0.05f));
        g.setGradientFill(glow);
        g.fillRoundedRectangle(outer, 18.0f);

        ColourGradient shell(panel(), r.getTopLeft(), panel2(), r.getBottomRight(), false);
        shell.addColour(0.28, Colour(0xff162238));
        shell.addColour(0.70, Colour(0xff0b1020));
        g.setGradientFill(shell);
        g.fillRoundedRectangle(r, 16.0f);

        auto sheen = r.reduced(1.5f).removeFromTop(42.0f);
        ColourGradient strip(Colour(0x303ae7ff), sheen.getTopLeft(), Colour(0x109f5cff), sheen.getTopRight(), false);
        strip.addColour(0.70, Colour(0x10ff6bb8));
        g.setGradientFill(strip);
        g.fillRoundedRectangle(sheen, 14.0f);

        g.setColour(Colours::white.withAlpha(0.035f));
        for (int y = 0; y < (int) r.getHeight(); y += 12)
            g.drawHorizontalLine((int) r.getY() + y, r.getX() + 10.0f, r.getRight() - 10.0f);

        ColourGradient border(accent().withAlpha(0.80f), r.getTopLeft(), accent2().withAlpha(0.45f), r.getBottomRight(), false);
        border.addColour(0.65, accent4().withAlpha(0.35f));
        g.setGradientFill(border);
        g.drawRoundedRectangle(r, 16.0f, 1.2f);

        g.setColour(textHi());
        g.setFont(FontOptions(14.5f, Font::bold));
        auto titleArea = juce::Rectangle<int>(juce::roundToInt(r.getX() + 12.0f), juce::roundToInt(r.getY() + 9.0f), juce::roundToInt(r.getWidth() - 24.0f), 20);
        g.drawText(title, titleArea, Justification::left);
    }

    StringArray octaveItems() { return { "-4","-3","-2","-1","0","1","2","3","4" }; }

    StringArray semiItems()   { return { "-12","-11","-10","-9","-8","-7","-6","-5","-4","-3","-2","-1","0","1","2","3","4","5","6","7","8","9","10","11","12" }; }
    StringArray unisonItems() { return { "1","2","3","4","5" }; }
    StringArray subWaveItems(){ return { "Sine","Tri","Saw","Square" }; }
    StringArray filterItems() { return { "LP", "BP", "HP", "NT", "PK" }; }
    StringArray bassModeItems(){ return { "Init", "Reese", "Womp", "Screech", "Warhorn", "Whoop", "Donk", "Tearout", "Laser", "Neuro", "Horn Stab", "Vapor" }; }
    StringArray noiseTypeItems(){ return { "White", "Air", "Metal", "Crunch", "Vowel", "Laser", "Horn", "Vinyl" }; }
    StringArray playItems()   { return { "Mono", "Legato", "Poly" }; }
    StringArray polyItems()   { return { "1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16" }; }
    StringArray modSources()  { return { "None", "ENV2", "ENV3", "LFO1", "LFO2", "LFO3", "LFO4", "Velocity" }; }
    StringArray modDests()    { return { "None", "Cutoff", "OSC A Pos", "OSC B Pos", "Pitch", "Level", "Pan", "Dist", "FX" }; }
}

void ScopeDisplay::timerCallback() { processor.copyScopeData(data); repaint(); }
void SpectrumDisplay::timerCallback() { processor.copyAnalyzerData(data); repaint(); }
void LfoDisplay::timerCallback() { processor.copyLfoShape(data); repaint(); }
void MatrixDisplay::timerCallback() { summary = processor.getMatrixSummary(); repaint(); }


void ScopeDisplay::paint(Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(6.0f);
    ColourGradient bgFill(Colour(0xff08101a), r.getTopLeft(), Colour(0xff10192b), r.getBottomRight(), false);
    bgFill.addColour(0.55, Colour(0xff0f1730));
    g.setGradientFill(bgFill);
    g.fillRoundedRectangle(r, 10.0f);

    g.setColour(Colours::white.withAlpha(0.06f));
    for (int i = 0; i <= 8; ++i)
    {
        const float y = r.getY() + r.getHeight() * (float) i / 8.0f;
        g.drawHorizontalLine((int) y, r.getX(), r.getRight());
    }

    Path line, fill;
    line.startNewSubPath(r.getX(), r.getCentreY());
    fill.startNewSubPath(r.getX(), r.getBottom());
    for (size_t i = 0; i < data.size(); ++i)
    {
        const float x = r.getX() + r.getWidth() * (float) i / (float) (data.size() - 1);
        const float y = r.getCentreY() - data[i] * r.getHeight() * 0.42f;
        line.lineTo(x, y);
        fill.lineTo(x, y);
    }
    fill.lineTo(r.getRight(), r.getBottom());
    fill.closeSubPath();

    ColourGradient area(accent().withAlpha(0.24f), r.getTopLeft(), accent2().withAlpha(0.06f), r.getBottomLeft(), false);
    area.addColour(0.7, accent4().withAlpha(0.04f));
    g.setGradientFill(area);
    g.fillPath(fill);

    g.setColour(accent().withAlpha(0.28f));
    g.strokePath(line, PathStrokeType(6.0f));
    g.setColour(accent());
    g.strokePath(line, PathStrokeType(2.2f));
    g.setColour(Colours::white.withAlpha(0.18f));
    g.drawFittedText("3D WAVE", r.toNearestInt().removeFromTop(18), Justification::centredRight, 1);
}



void SpectrumDisplay::paint(Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(6.0f);
    g.setColour(Colour(0xff0a1020));
    g.fillRoundedRectangle(r, 10.0f);

    Path ridge;
    ridge.startNewSubPath(r.getX(), r.getBottom());
    for (size_t i = 0; i < data.size(); ++i)
    {
        const float x = r.getX() + r.getWidth() * (float) i / (float) (data.size() - 1);
        const float norm = jlimit(0.0f, 1.0f, (data[i] + 100.0f) / 100.0f);
        const float y = r.getBottom() - norm * r.getHeight();
        ridge.lineTo(x, y);
    }
    ridge.lineTo(r.getRight(), r.getBottom());
    ridge.closeSubPath();

    ColourGradient fill(accent3().withAlpha(0.22f), r.getBottomLeft(), accent2().withAlpha(0.05f), r.getTopRight(), false);
    fill.addColour(0.55, accent().withAlpha(0.14f));
    g.setGradientFill(fill);
    g.fillPath(ridge);

    g.setColour(accent3().withAlpha(0.35f));
    for (size_t i = 0; i < data.size(); ++i)
    {
        const float x = r.getX() + r.getWidth() * (float) i / (float) data.size();
        const float w = r.getWidth() / (float) data.size();
        const float norm = jlimit(0.0f, 1.0f, (data[i] + 100.0f) / 100.0f);
        g.fillRoundedRectangle(x, r.getBottom() - norm * r.getHeight(), w - 1.0f, norm * r.getHeight(), 1.0f);
    }

    g.setColour(textLo());
    g.drawFittedText("SPECTRUM", r.toNearestInt().removeFromTop(18), Justification::centredRight, 1);
}



void LfoDisplay::paint(Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(6.0f);
    g.setColour(Colour(0xff091120));
    g.fillRoundedRectangle(r, 10.0f);
    g.setColour(Colours::white.withAlpha(0.08f));
    g.drawLine(r.getX(), r.getCentreY(), r.getRight(), r.getCentreY(), 1.0f);

    Path p;
    p.startNewSubPath(r.getX(), r.getCentreY());
    for (size_t i = 0; i < data.size(); ++i)
    {
        const float x = r.getX() + r.getWidth() * (float) i / (float) (data.size() - 1);
        const float y = r.getCentreY() - data[i] * r.getHeight() * 0.40f;
        p.lineTo(x, y);
    }

    g.setColour(accent4().withAlpha(0.25f));
    g.strokePath(p, PathStrokeType(6.0f));
    g.setColour(accent4());
    g.strokePath(p, PathStrokeType(2.0f));
    g.setColour(textLo());
    g.drawFittedText("MOTION", r.toNearestInt().removeFromTop(18), Justification::centredRight, 1);
}



void MatrixDisplay::paint(Graphics& g)
{
    auto r = getLocalBounds().reduced(8).toFloat();
    g.setColour(Colour(0xff09101b));
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(accent2().withAlpha(0.25f));
    g.drawRoundedRectangle(r, 8.0f, 1.0f);
    g.setColour(textLo());
    g.setFont(FontOptions(11.0f));
    g.drawFittedText(summary, r.toNearestInt(), Justification::topLeft, 5);
}


SerumKnob::SerumKnob()
{
    addAndMakeVisible(slider);
    addAndMakeVisible(title);
    slider.setSliderStyle(Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(Slider::TextBoxBelow, false, 56, 16);
    slider.setColour(Slider::rotarySliderFillColourId, accent());
    slider.setColour(Slider::thumbColourId, accent2());
    slider.setColour(Slider::textBoxTextColourId, textHi());
    slider.setColour(Slider::textBoxOutlineColourId, Colours::transparentBlack);
    slider.setColour(Slider::textBoxBackgroundColourId, Colours::black.withAlpha(0.22f));
    title.setJustificationType(Justification::centred);
    title.setColour(Label::textColourId, textLo());
    title.setFont(FontOptions(11.0f));
}

void SerumKnob::resized()
{
    auto r = getLocalBounds();
    title.setBounds(r.removeFromTop(18));
    slider.setBounds(r);
}

FiveParksVST3AudioProcessorEditor::FiveParksVST3AudioProcessorEditor(FiveParksVST3AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), scope(p), spectrum(p), lfoShape(p), matrixView(p)
{
    setSize(1880, 1040);

    logo.setText("5parks VST3 · NIGHT PRISM", dontSendNotification);
    logo.setColour(juce::Label::textColourId, textHi());
    logo.setFont(juce::FontOptions(26.0f, juce::Font::bold));
    subtitle.setText("dark neon wavetable bass lab · serum-style layout · bigger displays · smoother tones · dnb bass design", dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, textLo());
    subtitle.setFont(juce::FontOptions(13.0f));
    addAndMakeVisible(logo);
    addAndMakeVisible(subtitle);

    for (auto* c : { &oscABox, &oscBBox, &subBox, &noiseBox, &filterBox, &bassBox, &envBox, &lfoBox, &matrixBox, &fxBox, &perfBox, &displayBox })
        addAndMakeVisible(*c);

    displayBox.addAndMakeVisible(scope);
    displayBox.addAndMakeVisible(spectrum);
    displayBox.addAndMakeVisible(lfoShape);
    matrixBox.addAndMakeVisible(matrixView);

    oscABox.addAndMakeVisible(oscAOct); oscABox.addAndMakeVisible(oscASemi); oscABox.addAndMakeVisible(oscAUnison);
    oscBBox.addAndMakeVisible(oscBOct); oscBBox.addAndMakeVisible(oscBSemi); oscBBox.addAndMakeVisible(oscBUnison);
    for (auto* cb : { &oscAOct,&oscASemi,&oscAUnison,&oscBOct,&oscBSemi,&oscBUnison,&subWave,&noiseType,&filterMode,&bassMode,&playMode,&polyphony,&mod1Src,&mod1Dst,&mod2Src,&mod2Dst,&mod3Src,&mod3Dst,&mod4Src,&mod4Dst,&mod5Src,&mod5Dst,&mod6Src,&mod6Dst })
        styleCombo(*cb);

    oscAOct.addItemList(octaveItems(), 1); oscASemi.addItemList(semiItems(), 1); oscAUnison.addItemList(unisonItems(), 1);
    oscBOct.addItemList(octaveItems(), 1); oscBSemi.addItemList(semiItems(), 1); oscBUnison.addItemList(unisonItems(), 1);
    subWave.addItemList(subWaveItems(), 1); noiseType.addItemList(noiseTypeItems(), 1); filterMode.addItemList(filterItems(), 1); bassMode.addItemList(bassModeItems(), 1); playMode.addItemList(playItems(), 1); polyphony.addItemList(polyItems(), 1);
    for (auto* cb : { &mod1Src,&mod2Src,&mod3Src,&mod4Src,&mod5Src,&mod6Src }) cb->addItemList(modSources(), 1);
    for (auto* cb : { &mod1Dst,&mod2Dst,&mod3Dst,&mod4Dst,&mod5Dst,&mod6Dst }) cb->addItemList(modDests(), 1);

    subDirect.setButtonText("Direct Out");
    noiseDirect.setButtonText("Direct Out");
    for (auto* b : { &subDirect, &noiseDirect })
    {
        b->setColour(ToggleButton::textColourId, textLo());
        b->setColour(ToggleButton::tickColourId, accent());
        b->setColour(ToggleButton::tickDisabledColourId, textLo());
    }
    subBox.addAndMakeVisible(subWave); subBox.addAndMakeVisible(subDirect);
    noiseBox.addAndMakeVisible(noiseType); noiseBox.addAndMakeVisible(noiseDirect);
    filterBox.addAndMakeVisible(filterMode); bassBox.addAndMakeVisible(bassMode);
    perfBox.addAndMakeVisible(playMode); perfBox.addAndMakeVisible(polyphony);

    attach(oscABox, oscAPos, "oscA_pos", "WT POS");
    attach(oscABox, oscALevel, "oscA_level", "LEVEL");
    attach(oscABox, oscADetune, "oscA_detune", "DETUNE");
    attach(oscABox, oscABlend, "oscA_blend", "BLEND");
    attach(oscABox, oscAPan, "oscA_pan", "PAN");

    attach(oscBBox, oscBPos, "oscB_pos", "WT POS");
    attach(oscBBox, oscBLevel, "oscB_level", "LEVEL");
    attach(oscBBox, oscBDetune, "oscB_detune", "DETUNE");
    attach(oscBBox, oscBBlend, "oscB_blend", "BLEND");
    attach(oscBBox, oscBPan, "oscB_pan", "PAN");

    attach(subBox, subLevel, "sub_level", "LEVEL");
    attach(noiseBox, noiseLevel, "noise_level", "LEVEL");
    attach(filterBox, filterCutoff, "filter_cutoff", "CUTOFF");
    attach(filterBox, filterRes, "filter_res", "RES");
    attach(filterBox, filterDrive, "filter_drive", "DRIVE");
    attach(filterBox, filterMix, "filter_mix", "MIX");

    attach(bassBox, reeseAmt, "reese_amt", "REESE");
    attach(bassBox, warhornAmt, "warhorn_amt", "WARHORN");
    attach(bassBox, wompAmt, "womp_amt", "WOMP");
    attach(bassBox, subharmAmt, "subharm_amt", "SUB HARM");
    attach(bassBox, comboAmt, "combo_amt", "COMBO");
    attach(bassBox, stereoWidth, "stereo_width", "WIDTH");
    attach(bassBox, fmAmt, "fm_amt", "FM GRIT");
    attach(bassBox, airAmt, "air_amt", "AIR");
    attach(bassBox, bodyAmt, "body_amt", "BODY");
    attach(bassBox, growlAmt, "growl_amt", "GROWL");
    attach(bassBox, toneAmt, "tone_amt", "TONE");
    attach(bassBox, motionAmt, "motion_amt", "MOTION");
    attach(bassBox, screechAmt, "screech_amt", "SCREECH");
    attach(bassBox, whoopAmt, "whoop_amt", "WHOOP");
    attach(bassBox, vowelAmt, "vowel_amt", "VOWEL");
    attach(bassBox, smearAmt, "smear_amt", "SMEAR");

    attach(envBox, env1A, "env1_a", "ATTACK");
    attach(envBox, env1D, "env1_d", "DECAY");
    attach(envBox, env1S, "env1_s", "SUSTAIN");
    attach(envBox, env1R, "env1_r", "RELEASE");

    attach(lfoBox, lfo1Rate, "lfo1_rate", "RATE");
    attach(lfoBox, lfo1Amt, "lfo1_amt", "AMOUNT");
    attach(lfoBox, lfo1Shape, "lfo1_shape", "SHAPE");
    attach(lfoBox, lfo1Smooth, "lfo1_smooth", "SMOOTH");

    attach(fxBox, distDrive, "dist_drive", "DIST DRIVE");
    attach(fxBox, distMix, "dist_mix", "DIST MIX");
    attach(fxBox, chorusMix, "chorus_mix", "CHORUS");
    attach(fxBox, delayMix, "delay_mix", "DELAY");
    attach(fxBox, reverbMix, "reverb_mix", "REVERB");
    attach(fxBox, compAmt, "comp_amt", "COMP");

    attach(perfBox, glide, "glide_ms", "GLIDE");
    attach(perfBox, masterGain, "master_gain", "MASTER");

    addMatrixSlot(1, mod1Src, mod1Dst, mod1Amt, "mod1_src", "mod1_dst", "mod1_amt");
    addMatrixSlot(2, mod2Src, mod2Dst, mod2Amt, "mod2_src", "mod2_dst", "mod2_amt");
    addMatrixSlot(3, mod3Src, mod3Dst, mod3Amt, "mod3_src", "mod3_dst", "mod3_amt");
    addMatrixSlot(4, mod4Src, mod4Dst, mod4Amt, "mod4_src", "mod4_dst", "mod4_amt");
    addMatrixSlot(5, mod5Src, mod5Dst, mod5Amt, "mod5_src", "mod5_dst", "mod5_amt");
    addMatrixSlot(6, mod6Src, mod6Dst, mod6Amt, "mod6_src", "mod6_dst", "mod6_amt");

    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscA_oct", oscAOct));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscA_semi", oscASemi));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscA_unison", oscAUnison));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscB_oct", oscBOct));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscB_semi", oscBSemi));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscB_unison", oscBUnison));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "sub_wave", subWave));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "noise_type", noiseType));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "filter_mode", filterMode));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "bass_mode", bassMode));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "play_mode", playMode));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "polyphony", polyphony));
    buttonAtts.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "sub_direct", subDirect));
    buttonAtts.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "noise_direct", noiseDirect));
}

void FiveParksVST3AudioProcessorEditor::configureKnob(SerumKnob& knob, const String& title)
{
    knob.title.setText(title, dontSendNotification);
}

void FiveParksVST3AudioProcessorEditor::attach(Component& parent, SerumKnob& knob, const String& id, const String& title)
{
    configureKnob(knob, title);
    parent.addAndMakeVisible(knob);
    sliderAtts.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, id, knob.slider));
}

void FiveParksVST3AudioProcessorEditor::styleCombo(ComboBox& box)
{
    box.setColour(ComboBox::backgroundColourId, Colours::black.withAlpha(0.22f));
    box.setColour(ComboBox::outlineColourId, edge());
    box.setColour(ComboBox::textColourId, textHi());
    box.setColour(ComboBox::arrowColourId, accent());
}

void FiveParksVST3AudioProcessorEditor::addMatrixSlot(int, ComboBox& src, ComboBox& dst, SerumKnob& amt, const String& srcId, const String& dstId, const String& amtId)
{
    matrixBox.addAndMakeVisible(src);
    matrixBox.addAndMakeVisible(dst);
    attach(matrixBox, amt, amtId, "AMT");
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, srcId, src));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, dstId, dst));
}


void FiveParksVST3AudioProcessorEditor::paint(Graphics& g)
{
    ColourGradient bgGrad(Colour(0xff05070d), 0.0f, 0.0f, Colour(0xff0d1424), 0.0f, (float) getHeight(), false);
    bgGrad.addColour(0.45, Colour(0xff101726));
    bgGrad.addColour(1.0, Colour(0xff05060c));
    g.setGradientFill(bgGrad);
    g.fillAll();

    auto halo = getLocalBounds().toFloat().reduced(8.0f);
    ColourGradient sweep(Colour(0x103ae7ff), halo.getTopLeft(), Colour(0x109f5cff), halo.getBottomRight(), false);
    sweep.addColour(0.55, Colour(0x10ff6bb8));
    g.setGradientFill(sweep);
    g.fillEllipse(halo.removeFromTop((int) (halo.getHeight() * 0.58f)).translated(180.0f, -40.0f));

    g.setColour(Colours::white.withAlpha(0.028f));
    for (int y = 0; y < getHeight(); y += 28) g.drawHorizontalLine(y, 0.0f, (float) getWidth());
    for (int x = 0; x < getWidth(); x += 28) g.drawVerticalLine(x, 0.0f, (float) getHeight());

    drawCard(g, displayBox.getBounds().toFloat(), "WAVETABLE DISPLAY · 3D SCOPE · SPECTRUM");
    drawCard(g, oscABox.getBounds().toFloat(), "OSC A");
    drawCard(g, oscBBox.getBounds().toFloat(), "OSC B");
    drawCard(g, subBox.getBounds().toFloat(), "SUB");
    drawCard(g, noiseBox.getBounds().toFloat(), "NOISE");
    drawCard(g, filterBox.getBounds().toFloat(), "FILTER");
    drawCard(g, bassBox.getBounds().toFloat(), "BASS ENGINE");
    drawCard(g, envBox.getBounds().toFloat(), "AMP ENV");
    drawCard(g, lfoBox.getBounds().toFloat(), "LFO / MOTION");
    drawCard(g, matrixBox.getBounds().toFloat(), "MOD");
    drawCard(g, fxBox.getBounds().toFloat(), "FX");
    drawCard(g, perfBox.getBounds().toFloat(), "PERFORMANCE");

    auto kb = getLocalBounds().removeFromBottom(42).reduced(24, 0).toFloat();
    const int whiteKeys = 34;
    const float kw = kb.getWidth() / (float) whiteKeys;
    for (int i = 0; i < whiteKeys; ++i)
    {
        auto key = Rectangle<float>(kb.getX() + i * kw, kb.getY(), kw - 2.0f, kb.getHeight());
        g.setColour(Colours::white.withAlpha(0.82f));
        g.fillRoundedRectangle(key, 3.0f);
        Colour kc = (i % 4 == 0 ? accent() : (i % 4 == 1 ? accent2() : (i % 4 == 2 ? accent3() : accent4()))).withAlpha(0.22f);
        g.setColour(kc);
        g.fillRoundedRectangle(key.removeFromBottom(6.0f), 3.0f);
    }
}

void FiveParksVST3AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(18);
    auto header = area.removeFromTop(54);
    logo.setBounds(header.removeFromLeft(300));
    subtitle.setBounds(header.removeFromLeft(500));
    area.removeFromTop(18);

    auto top = area.removeFromTop(270);
    displayBox.setBounds(top.removeFromLeft(820));
    top.removeFromLeft(8);
    oscABox.setBounds(top.removeFromLeft(240));
    top.removeFromLeft(8);
    oscBBox.setBounds(top.removeFromLeft(240));
    top.removeFromLeft(8);
    auto sideTop = top;
    subBox.setBounds(sideTop.removeFromTop(132));
    sideTop.removeFromTop(8);
    noiseBox.setBounds(sideTop);

    area.removeFromTop(10);
    auto middle = area.removeFromTop(290);
    filterBox.setBounds(middle.removeFromLeft(240));
    middle.removeFromLeft(8);
    bassBox.setBounds(middle.removeFromLeft(620));
    middle.removeFromLeft(8);
    envBox.setBounds(middle.removeFromLeft(220));
    middle.removeFromLeft(8);
    lfoBox.setBounds(middle.removeFromLeft(220));
    middle.removeFromLeft(8);
    perfBox.setBounds(middle);

    area.removeFromTop(10);
    auto bottom = area;
    matrixBox.setBounds(bottom.removeFromLeft(430));
    bottom.removeFromLeft(8);
    fxBox.setBounds(bottom);

    auto disp = displayBox.getLocalBounds().reduced(12); disp.removeFromTop(24);
    auto hero = disp.removeFromTop(152);
    scope.setBounds(hero);
    disp.removeFromTop(8);
    auto lowerDisp = disp.removeFromTop(92);
    spectrum.setBounds(lowerDisp.removeFromLeft(lowerDisp.getWidth() / 2 - 4));
    lowerDisp.removeFromLeft(8);
    lfoShape.setBounds(lowerDisp);

    auto layoutOsc = [](Component& box, ComboBox& oct, ComboBox& semi, ComboBox& uni, std::initializer_list<SerumKnob*> knobs)
    {
        auto r = box.getLocalBounds().reduced(10); r.removeFromTop(24);
        auto head = r.removeFromTop(24);
        oct.setBounds(head.removeFromLeft(70)); head.removeFromLeft(6); semi.setBounds(head.removeFromLeft(70)); head.removeFromLeft(6); uni.setBounds(head.removeFromLeft(70));
        r.removeFromTop(8);
        auto row1 = r.removeFromTop(145);
        for (auto* k : knobs)
        {
            k->setBounds(row1.removeFromLeft(48).withTrimmedTop(0).reduced(2));
            row1.removeFromLeft(2);
        }
    };
    layoutOsc(oscABox, oscAOct, oscASemi, oscAUnison, { &oscAPos, &oscALevel, &oscADetune, &oscABlend, &oscAPan });
    layoutOsc(oscBBox, oscBOct, oscBSemi, oscBUnison, { &oscBPos, &oscBLevel, &oscBDetune, &oscBBlend, &oscBPan });

    auto sb = subBox.getLocalBounds().reduced(10); sb.removeFromTop(24);
    auto srow = sb.removeFromTop(24);
    subWave.setBounds(srow.removeFromLeft(90)); srow.removeFromLeft(8); subDirect.setBounds(srow.removeFromLeft(100));
    subLevel.setBounds(sb.removeFromTop(140).removeFromLeft(60));

    auto nb = noiseBox.getLocalBounds().reduced(10); nb.removeFromTop(24);
    auto nrow = nb.removeFromTop(24);
    noiseType.setBounds(nrow.removeFromLeft(100)); nrow.removeFromLeft(8); noiseDirect.setBounds(nrow.removeFromLeft(100));
    noiseLevel.setBounds(nb.removeFromTop(140).removeFromLeft(60));

    auto fb = filterBox.getLocalBounds().reduced(10); fb.removeFromTop(24);
    filterMode.setBounds(fb.removeFromTop(24).removeFromLeft(80));
    fb.removeFromTop(8);
    auto frow = fb.removeFromTop(150);
    for (auto* k : { &filterCutoff, &filterRes, &filterDrive, &filterMix }) { k->setBounds(frow.removeFromLeft(64).reduced(2)); frow.removeFromLeft(4); }

    auto bb = bassBox.getLocalBounds().reduced(10); bb.removeFromTop(24);
    bassMode.setBounds(bb.removeFromTop(24).removeFromLeft(140));
    bb.removeFromTop(6);
    auto brow1 = bb.removeFromTop(70);
    for (auto* k : { &reeseAmt, &warhornAmt, &wompAmt, &subharmAmt }) { k->setBounds(brow1.removeFromLeft(96).reduced(2)); brow1.removeFromLeft(4); }
    bb.removeFromTop(4);
    auto brow2 = bb.removeFromTop(70);
    for (auto* k : { &comboAmt, &stereoWidth, &fmAmt, &airAmt }) { k->setBounds(brow2.removeFromLeft(96).reduced(2)); brow2.removeFromLeft(4); }
    bb.removeFromTop(4);
    auto brow3 = bb.removeFromTop(70);
    for (auto* k : { &bodyAmt, &growlAmt, &toneAmt, &motionAmt }) { k->setBounds(brow3.removeFromLeft(96).reduced(2)); brow3.removeFromLeft(4); }
    bb.removeFromTop(4);
    auto brow4 = bb.removeFromTop(70);
    for (auto* k : { &screechAmt, &whoopAmt, &vowelAmt, &smearAmt }) { k->setBounds(brow4.removeFromLeft(96).reduced(2)); brow4.removeFromLeft(4); }

    auto eb = envBox.getLocalBounds().reduced(10); eb.removeFromTop(24);
    auto erow = eb.removeFromTop(150);
    for (auto* k : { &env1A, &env1D, &env1S, &env1R }) { k->setBounds(erow.removeFromLeft(64).reduced(2)); erow.removeFromLeft(4); }

    auto lb = lfoBox.getLocalBounds().reduced(10); lb.removeFromTop(24);
    auto lrow = lb.removeFromTop(150);
    for (auto* k : { &lfo1Rate, &lfo1Amt, &lfo1Shape, &lfo1Smooth }) { k->setBounds(lrow.removeFromLeft(64).reduced(2)); lrow.removeFromLeft(4); }

    auto pb = perfBox.getLocalBounds().reduced(10); pb.removeFromTop(24);
    auto prow = pb.removeFromTop(24);
    playMode.setBounds(prow.removeFromLeft(100)); prow.removeFromLeft(8); polyphony.setBounds(prow.removeFromLeft(100));
    pb.removeFromTop(10);
    auto pkn = pb.removeFromTop(150);
    glide.setBounds(pkn.removeFromLeft(74).reduced(2)); pkn.removeFromLeft(4);
    masterGain.setBounds(pkn.removeFromLeft(74).reduced(2));

    auto mb = matrixBox.getLocalBounds().reduced(10); mb.removeFromTop(24);
    auto viewArea = mb.removeFromBottom(86);
    matrixView.setBounds(viewArea);
    for (int i = 0; i < 4; ++i)
    {
        auto row = mb.removeFromTop(34);
        ComboBox* srcs[] = { &mod1Src,&mod2Src,&mod3Src,&mod4Src,&mod5Src,&mod6Src };
        ComboBox* dsts[] = { &mod1Dst,&mod2Dst,&mod3Dst,&mod4Dst,&mod5Dst,&mod6Dst };
        SerumKnob* amts[] = { &mod1Amt,&mod2Amt,&mod3Amt,&mod4Amt,&mod5Amt,&mod6Amt };
        srcs[i]->setBounds(row.removeFromLeft(112)); row.removeFromLeft(4);
        dsts[i]->setBounds(row.removeFromLeft(122)); row.removeFromLeft(4);
        amts[i]->setBounds(row.removeFromLeft(72).withHeight(96));
        mb.removeFromTop(4);
    }
    for (auto* cb : { &mod5Src, &mod5Dst, &mod6Src, &mod6Dst }) cb->setBounds(0,0,0,0);
    mod5Amt.setBounds(0,0,0,0);
    mod6Amt.setBounds(0,0,0,0);

    auto xb = fxBox.getLocalBounds().reduced(10); xb.removeFromTop(24);
    auto xrow = xb.removeFromTop(150);
    for (auto* k : { &distDrive, &distMix, &chorusMix, &delayMix, &reverbMix, &compAmt }) { k->setBounds(xrow.removeFromLeft(84).reduced(2)); xrow.removeFromLeft(6); }
}
