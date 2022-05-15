#include "../GameLocal.h"
#pragma hdrstop
#include "AKLocal.h"


AKSinusMovement::AKSinusMovement(float radius, float degreesPerSec, int axesBits, float initialAngle /* = 0.0f */)
: m_frame(0),
m_degreesPerSec(degreesPerSec),
m_initialAngle(initialAngle),
m_currentAngle(initialAngle),
m_radius(radius),
m_axesBits(axesBits)
{
	// nothing
}

Vec2 AKSinusMovement::GetLocalPos() const
{
	float x = sinf(MATH_DEG2RAD(m_currentAngle)) * m_radius;
	float y = cosf(MATH_DEG2RAD(m_currentAngle)) * m_radius;
	return Vec2(x, y);
}

Vec2 AKSinusMovement::Update()
{
	Vec2 oldLocalPos = GetLocalPos();

	m_frame++;
	m_currentAngle = m_initialAngle + ((float)m_frame * GAME_VIRTUAL_TIMEDIFF) * m_degreesPerSec;
	m_currentAngle = Math_AngleNormalize360(m_currentAngle);

	Vec2 newLocalPos = GetLocalPos();

	Vec2 diff;
	for(int axis = 0; axis < 2; axis++)
	{
		if(m_axesBits & BIT(axis))
		{
			diff[axis] = newLocalPos[axis] - oldLocalPos[axis];
		}
	}
	return diff;
}
