#pragma once
#include <gazeapi.h>
#include <glm/glm.hpp>

enum
{
    TRACKER_CONNECTED          = 0,
    TRACKER_NOT_CONNECTED      = 1,
    TRACKER_CONNECTED_BADFW    = 2,
    TRACKER_CONNECTED_NOUSB3   = 3,
    TRACKER_CONNECTED_NOSTREAM = 4
};


class MyGaze : public gtl::IGazeListener
{
public:
	MyGaze(int windowWidthPixel, int windowHeightPixel);
	~MyGaze();
	glm::vec2 point2dToOfVec2d(const gtl::Point2D point2d);
	void normarize(gtl::Point2D & point2d);
	void update();
	glm::vec2 getPoint2dRaw() { return point2dToOfVec2d(mGazeData.raw); }
	glm::vec2 getPoint2dAvg() { return point2dToOfVec2d(mGazeData.avg); }
	glm::vec2 getRightEyePcenter();
	glm::vec2 getLeftEyePcenter() { return point2dToOfVec2d(mGazeData.lefteye.pcenter); }
	bool isFix() { return mGazeData.fix; }
	bool isConnected() { return m_api.is_connected(); }
    gtl::ServerState const & getServerState() { return m_api.get_server_state(); }
	void updateState();
private:
        // IGazeListener
	//Objects that wish to automatically receive gaze data should implement the IGazeListener interface. 
	//This interface contains the on_gaze_data callback method that will receive the coordinates of the estimated on-screen gaze position, size of the pupils, position relative to the sensor, etc.
    void on_gaze_data( gtl::GazeData const & gaze_data );

    gtl::GazeApi m_api;
	gtl::GazeData mGazeData;
    gtl::Screen mScreen;
	bool active;

};

