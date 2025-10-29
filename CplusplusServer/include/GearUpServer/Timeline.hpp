#pragma once

#include <vector>
#include <tuple>

class Timeline
{
public:
	Timeline() = default;
	~Timeline() = default;

	Timeline(const Timeline&) = default;
	Timeline(Timeline&&) = default;

	Timeline& operator=(const Timeline&) = default;
	Timeline& operator=(Timeline&&) = default;

	void AddKey(float time, float value);
	float Evaluate(float time) const;

private:
	// time, value
	std::vector <std::tuple<float, float>> m_timeline;
};
