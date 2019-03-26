#include "etc_element.h"

using namespace ofxBenG;

void etc_element_midi_proxy::setup(std::string midiDeviceName) {
    midiOut.openPort(midiDeviceName);
}

void etc_element_midi_proxy::setSubmaster(int faderNumber, int level) {
    // https://www.etcconnect.com/Support/Articles/Understanding-the-MSC-Commands-Eos-Family-Receives-and-Transmits.aspx?LangType=1033
    int const setSubmasterCommand = 0x06;
    int const broadcastDeviceId = 0x7F;
    std::vector<unsigned char> sysexMsg;
    sysexMsg.push_back(MIDI_SYSEX);
    sysexMsg.push_back(0x7F);
    sysexMsg.push_back(broadcastDeviceId);
    sysexMsg.push_back(0x02);
    sysexMsg.push_back(0x01);
    sysexMsg.push_back(setSubmasterCommand);
    sysexMsg.push_back(faderNumber);
    sysexMsg.push_back(0x00);
    sysexMsg.push_back(level);
    sysexMsg.push_back(0x00);
    sysexMsg.push_back(MIDI_SYSEX_END);
    midiOut.sendMidiBytes(sysexMsg);
}

void etc_element_osc_proxy::setup(std::string remoteIp, int remotePort) {
    oscSender.setup(remoteIp, remotePort);
}

void etc_element_osc_proxy::setSubmaster(int faderNumber, float level) {
    ofxOscMessage message;
    std::string address = "/eos/sub/" + ofToString(faderNumber);
    message.setAddress(address);
    message.addFloatArg(level);
    oscSender.sendMessage(message, false);
}

void etc_element_osc_proxy::setChannel(int faderNumber, int level) {
    ofxOscMessage message;
    std::string address = "/eos/chan/" + ofToString(faderNumber);
    message.setAddress(address);
    message.addInt32Arg(level);
    oscSender.sendMessage(message, false);
}
