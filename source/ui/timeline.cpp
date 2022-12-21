#include "ronin.h"
#include "begin.h"

namespace RoninEngine::UI
{
	Timeline::Timeline() {

		state = 1;
		_option = TimelineOptions::Linear;
		Reset();
	}
	Timeline::~Timeline() {

	}

	void Timeline::AddRoad(const TimelineRoad& road) {
		_roads.emplace_back(road);
		Reset();
	}
	void Timeline::AddRoad(Texture* texture, const float duration) {
		TimelineRoad road;
		road.texture = texture;
		road.duration = duration;
		AddRoad(road);
	}
    void Timeline::AddRoads(const std::list<TimelineRoad>& roads) {

        _roads.assign(std::begin(roads), std::end(roads));
		Reset();
	}

	TimelineRoad* Timeline::Evaluate(float time) {
		static float __innertime = 0;

		if (iter == end(_roads))
			return nullptr;

		if (isPause())
		{
			__innertime = time;
			return &*iter;
		}
		if (!isPlay())
		{
			__innertime = 0;
			return &*begin(_roads);
		}

		if (time > __innertime)
		{
			if (__innertime != 0)
			{
				switch (_option)
				{
				case RoninEngine::UI::TimelineOptions::Linear:
					++iter;
					if (iter == end(_roads))
						Reset();
					break;
				case RoninEngine::UI::TimelineOptions::LinearReverse:
				{
					static bool isReversing = false;

					if (!isReversing)
					{
						++iter;
						if (iter == end(_roads))
						{
							isReversing = true;
							--iter;
						}
					}
					else
					{
						if (iter == begin(_roads))
						{
							isReversing = false;
						}
						else
							--iter;
					}
				}
				break;
				}
			}

			__innertime = time + iter->duration;
		}

		return &*iter;
	}


	void Timeline::SetOptions(TimelineOptions option) {
		_option = option;
	}
	TimelineOptions Timeline::GetOptions() {
		return _option;
	}

	void Timeline::Reset()
	{
		iter = begin(_roads);
	}

	void Timeline::Stop() {
		Reset();
		state = 0;
	}
	void Timeline::Play() {
		if (!isPause())
			Reset();
		state = 1;
	}
	void Timeline::Pause() {
		state = 2;
	}
	bool Timeline::isPlay() {
		return state == 1;
	}
	bool Timeline::isPause() {
		return state == 2;
	}
}
