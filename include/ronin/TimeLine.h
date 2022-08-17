#pragma once

#include "begin.h"

using namespace RoninEngine::Runtime;

namespace RoninEngine::UI
{
	struct TimelineRoad
	{
		Texture* texture;
		float duration;
	}; 

	enum class TimelineOptions
	{
		Linear,
		LinearReverse
	};

     class RONIN_API Timeline
	{
	private:
		TimelineOptions _option;
          std::list<TimelineRoad> _roads;
          std::list<TimelineRoad>::iterator iter;
		char state;
	public: 
		Timeline();
		~Timeline();

		void AddRoad(const TimelineRoad& road);
		void AddRoad(Texture* texture, const float duration);
          void AddRoads(const std::list<TimelineRoad>& roads);

		TimelineRoad* Evaluate(float time);

		void SetOptions(TimelineOptions option);
		TimelineOptions GetOptions();

		inline void Reset();
		inline void Stop();
		inline void Play();
		inline void Pause();
		inline bool isPlay();
		inline bool isPause();
	};
}
