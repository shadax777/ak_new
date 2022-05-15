#ifndef __AKSINUSMOVEMENT_H
#define __AKSINUSMOVEMENT_H


class AKSinusMovement
{
private:
	int		m_frame;		// run-away counter
	float	m_degreesPerSec;
	float	m_initialAngle;
	float	m_currentAngle;	// normalized to [0 .. 360[
	float	m_radius;
	int		m_axesBits;		// bit field describing the axes on which to perform waving (bit #0 = x-axis, bit #1 = y-axis)

public:
			AKSinusMovement(float radius, float degreesPerSec, int axesBits, float initialAngle = 0.0f);

	// current position on circle (i. e. relative to the rotation center of the caller)
	Vec2	GetLocalPos() const;

	// returns the difference between the old and current position on the circle
	Vec2	Update();
};


#endif	// __AKSINUSMOVEMENT_H
