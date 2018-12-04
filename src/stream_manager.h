#ifndef playmodes_h
#define playmodes_h

#include "ofxPS3EyeGrabber.h"
#include "ofxPlaymodes.h"
#include "blackmagic.h"
#include "video_stream.h"
#include "window.h"

namespace ofxBenG {

    using namespace ofxPm;

    class stream_manager {
    public:
        static const std::string blackmagic;
        static const std::string c920;
        static const std::string c615;
        static const std::string ps3eye;
        static const std::string facetime;

        stream_manager(float defaultWidth, float defaultHeight, float defaultFps, int defaultBufferSize)
                : defaultBufferSize(defaultBufferSize),
                  defaultWidth(defaultWidth),
                  defaultHeight(defaultHeight),
                  defaultFps(defaultFps) {
        }

        stream_manager(const std::string &deviceName, float defaultWidth, float defaultHeight, float defaultFps)
                : defaultBufferSize(defaultBufferSize),
                  defaultWidth(defaultWidth),
                  defaultHeight(defaultHeight),
                  defaultFps(defaultFps) {
            addVideoStream(deviceName);
        }

        ~stream_manager() {
            std::cout << "playmodes is closing" << std::endl;
            for (auto stream : streams) {
                delete stream;
            }
        }

        void update() {
            refreshStreamList();
            for (auto stream : streams) {
                stream->update();
            };
        }

        void addVideoStream(const std::string &deviceName) {
            video_stream *newStream = nullptr;
            if (deviceName.compare(blackmagic) == 0) {
                newStream = addBlackMagic();
            } else if (deviceName.compare(ps3eye) == 0) {
                newStream = addPs3Eye();
            } else {
                newStream = addGenericVideoDevice(deviceName);
            }

            streams.push_back(newStream);
            if (newStream != nullptr)
                ofNotifyEvent(onVideoStreamAdded, *newStream);
        }

        int getStreamCount() {
            return streams.size();
        }

        video_stream *getStream(int index) {
            if (index >= streams.size()) return nullptr;
            return streams[index];
        }

        video_stream *getLastStream() {
            return (streams.size() > 0) ? streams.front() : nullptr;
        }

        video_stream *getUnusedStream() {
            video_stream* unused = nullptr;
            for (auto stream : streams) {
                if (stream->getWindow() == nullptr) {
                    unused = stream;
                    break;
                }
            }
            return unused;
        }

        video_stream *findStream(std::string deviceName) {
            for (auto stream : streams) {
                if (stream->getDeviceName() == deviceName) {
                    return stream;
                }
            }
            return nullptr;
        }

        void freeStream(int i) {
            delete streams[i];
        }

        std::map<ofxPm::VideoFormat, std::vector<ofPtr<ofxPm::VideoFrame::Obj>>> *pool; // do not delete
        ofEvent<video_stream> onVideoStreamAdded;
        ofEvent<video_stream> onVideoStreamRemoved;

    private:
        void refreshStreamList() {
            auto videoDevices = getVideoDevices();

            // Add new video streams
            for (auto device : videoDevices) {
                if (!isStreamActive(device)) {
                    addVideoStream(device);
                }
            }

            // Cull stale video streams
            for (auto it = streams.begin(); it != streams.end();) {
                video_stream *stream = *it;
                if (isStreamStale(stream, videoDevices)) {
                    ofNotifyEvent(onVideoStreamRemoved, *stream);
                    delete stream;
                    it = streams.erase(it);
                } else {
                    it++;
                }
            }
        }

        std::vector<ofVideoDevice> getVideoDevices() {
            auto grabber = new ofxPm::VideoGrabber();
            auto videoDevices = grabber->listDevices();
            auto ps3EyeGrabber = std::make_shared<ofxPS3EyeGrabber>();
            auto ps3EyeDevices = ps3EyeGrabber->listDevices();
            for (auto device : ps3EyeDevices) {
                videoDevices.push_back(device);
            }
            delete grabber;
            return videoDevices;
        }

        bool isStreamActive(ofVideoDevice &device) {
            return findStream(device.deviceName) != nullptr;
        }

        bool isStreamStale(video_stream *stream, std::vector<ofVideoDevice> &devices) {
            bool foundDevice = false;
            for (auto device : devices) {
                if (device.deviceName == stream->getDeviceName()) {
                    foundDevice = true;
                    break;
                }
            }
            return !foundDevice;
        }

        void addVideoStream(ofVideoDevice &device) {
            if (device.deviceName == "PS3-Eye") {
                addPs3Eye();
            } else if (device.deviceName != "FaceTime HD Camera") {
                int width = defaultWidth;
                int height = defaultHeight;
                std::cout << "Adding video stream at (" << width << ", " << height << ")" << std::endl;
                auto grabber = new ofxPm::VideoGrabber();
                grabber->setDeviceID(device.id);
                grabber->setDesiredFrameRate(defaultFps);
                grabber->setFps(defaultFps);
                grabber->initGrabber(defaultWidth, defaultHeight);
                auto stream = new video_stream(device.deviceName, grabber, defaultBufferSize);
                streams.push_back(stream);
                ofNotifyEvent(onVideoStreamAdded, *stream);
            }
        }

        video_stream *addPs3Eye() {
            int videoWidth = 640;
            int videoHeight = 480;
            int fps = 60;
            auto grabber = new ofxPm::VideoGrabber();
            grabber->setGrabber(std::make_shared<ofxPS3EyeGrabber>());
            grabber->setDesiredFrameRate(fps);
            grabber->initGrabberWithUpdate(videoWidth, videoHeight);
            grabber->update();
            auto stream = new video_stream(ofxBenG::stream_manager::ps3eye, grabber, defaultBufferSize);
            streams.push_back(stream);
            std::cout << "PS3 Eye capturing at resolution (" << videoWidth << ", " << videoHeight << ")" << std::endl;
            return stream;
        }

        video_stream *addBlackMagic() {
            // BlackMagic Pocket Cinema: 24fps, 1920x1080
            // iPhone: 59.94fps, 1280x720
            auto stream = findStream(ofxBenG::stream_manager::blackmagic);
            if (stream == nullptr) {
                auto grabber = new ofxBenG::BlackMagicVideoSource();
                if (grabber->setup(defaultFps, defaultWidth, defaultHeight)) {
                    auto stream = new video_stream(ofxBenG::stream_manager::blackmagic, (ofxPm::VideoGrabber *) grabber, defaultBufferSize);
                    ofVec2f dimensions = grabber->getDimensions();
                    std::cout << "capturing at resolution (" << dimensions[0] << ", " << dimensions[1] << ")"
                              << std::endl;
                } else {
                    std::cout << "failed to initialize blackmagic" << std::endl;
                }
            }
            return stream;
        }

        video_stream *addGenericVideoDevice(const std::string &deviceName) {
            auto grabber = new ofxPm::VideoGrabber();
            auto videoDevices = grabber->listDevices();
            bool foundDeviceName = false;
            for (auto device : videoDevices) {
                if (device.deviceName.find(deviceName) != std::string::npos) {
                    std::cout << "Found deviceName=" << deviceName << " at deviceId=" << device.id << std::endl;
                    foundDeviceName = true;
                    grabber->setDeviceID(device.id);
                    break;
                }
            }

            if (foundDeviceName) {
                grabber->setDesiredFrameRate(defaultFps);
                grabber->setFps(defaultFps);
                grabber->initGrabber(defaultWidth, defaultHeight);

                auto stream = new video_stream(deviceName, grabber, defaultBufferSize);

                std::cout << "capturing at resolution (" << grabber->getWidth() << ", " << grabber->getHeight() << ")"
                          << std::endl;
                return stream;
            } else {
                delete grabber;
                return nullptr;
            }
        }

        std::vector<ofxBenG::video_stream *> streams;
        int defaultBufferSize;
        int defaultWidth;
        int defaultHeight;
        int defaultFps;
    };

}; /* ofxBenG */
#endif /* playmodes_h */
