#include "MyGaze.h"


MyGaze::MyGaze(int windowWidthPixel, int windowHeightPixel)
{
	//The server must be running before we can connect to it. 
	 // Connect to the server in push mode on the default TCP port (6555)
    if( m_api.connect( true ) )
    {
        // Enable GazeData notifications
		active = true;
        m_api.add_listener( *this );//setup a listener for the pushed gaze data. 
    }
	else
	{
		active = false;
		printf( "Server is not active");
	}
	mScreen.screenresw = windowWidthPixel;
	mScreen.screenresh = windowHeightPixel;
}


MyGaze::~MyGaze()
{
	m_api.remove_listener( *this );
    m_api.disconnect();
	
}


void MyGaze::on_gaze_data( gtl::GazeData const & gaze_data )
{
    if( gaze_data.state & gtl::GazeData::GD_STATE_TRACKING_GAZE )
    {
        gtl::Point2D const & smoothedCoordinates = gaze_data.avg;

        // Move GUI point, do hit-testing, log coordinates, etc.
    }
}

glm::vec2 MyGaze::point2dToOfVec2d(const gtl::Point2D point2d)
{
    return glm::vec2(point2d.x, point2d.y);
}

void MyGaze::normarize(gtl::Point2D & point2d)
{
    point2d.x = point2d.x / mScreen.screenresw;
    point2d.y = point2d.y / mScreen.screenresh;
}

void MyGaze::update()
{
    if (m_api.is_connected())
    {
        m_api.get_frame(mGazeData);
        m_api.get_screen(mScreen);
    }
}

glm::vec2 MyGaze::getRightEyePcenter()
{ 
	//printf("Right eye center position is: %f, %f \n", mGazeData.righteye.pcenter.x,mGazeData.righteye.pcenter.y);
	return point2dToOfVec2d(mGazeData.righteye.pcenter); 
}

void MyGaze::updateState()
{
    bool act = m_api.is_connected();

    if (act)
    {
        gtl::ServerState const& state = m_api.get_server_state();
        act = state.trackerstate == gtl::ServerState::TRACKER_CONNECTED && state.iscalibrated;
    }

    active = act;
  
}