#include "pch.h"

#include "log.h"
#include "trackers.h"
#include <util.h>

namespace openxr_api_layer {
    using namespace log;

    struct InseyeEyeTracker : IEyeTracker {
        InseyeEyeTracker()
            : m_inseyeEyeTracker(new inseye::EyeTracker(3000)) {
        }

        ~InseyeEyeTracker() override = default;

        void start(XrSession session) override {
        }

        void stop() override {
        }

        bool isGazeAvailable(XrTime time) const override {
            // time is ignored for now
            inseye::EyeTrackerDataStruct data_struct;
            if (m_inseyeEyeTracker->TryReadLatestEyeTrackerData(data_struct) || m_inseyeEyeTracker->
                TryReadLastEyeTrackerData(data_struct))
                return data_struct.gaze_event != inseye::GazeEvent::kInsGazeHeadsetDismount;
            return false;
        }

        bool getGaze(XrTime time, XrVector3f& unitVector) override {
            // time is ignored for now
            inseye::EyeTrackerDataStruct data_struct;
            if (!m_inseyeEyeTracker->TryReadLatestEyeTrackerData(data_struct) && !m_inseyeEyeTracker->
                TryReadLastEyeTrackerData(data_struct)) {
                TraceLoggingWrite(g_traceProvider, "Inseye_getGaze: failed to read gaze data");
                return false;
            }
            if (data_struct.gaze_event == inseye::GazeEvent::kInsGazeHeadsetDismount) {
                TraceLoggingWrite(g_traceProvider, "Inseye_getGaze: gaze data from unmounted headset");
                return false;
            }
            TraceLoggingWrite(g_traceProvider,
                              "Inseye_getGaze",
                              TLArg(xr::ToString(XrVector2f{data_struct.left_eye_x,
                                      data_struct.left_eye_y})
                                  .c_str(),
                                  "LeftEye"),
                              TLArg(xr::ToString(XrVector2f{data_struct.right_eye_x, data_struct.right_eye_y})
                                  .c_str(),
                                  "RightEye"));
            const float angleHorizontal =
                atan((data_struct.left_eye_x + data_struct.right_eye_x) / 2.f);
            const float angleVertical =
                atan((data_struct.left_eye_y + data_struct.right_eye_y) / 2.f);

            unitVector = {
                sin(angleHorizontal) * cos(angleVertical),
                sin(angleVertical),
                -cos(angleHorizontal) * cos(angleVertical),
            };
            return true;
        }

        TrackerType getType() const override {
            return TrackerType::Inseye;
        }


        std::unique_ptr<inseye::EyeTracker> m_inseyeEyeTracker;
    };

    std::unique_ptr<IEyeTracker> createInseyeEyeTracker() {
        return std::make_unique<InseyeEyeTracker>();
    }
}
