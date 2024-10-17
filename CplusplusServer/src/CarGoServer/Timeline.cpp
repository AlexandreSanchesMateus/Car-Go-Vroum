#include "CarGoServer/Timeline.hpp"
#include <fmt/core.h>
#include <fmt/color.h>

void Timeline::AddKey(float time, float value)
{
	if (time <= 0.f)
	{
		if (m_timeline.empty())
			m_timeline.push_back(std::make_tuple(0.f, value));
		else if (std::get<0>(m_timeline[0]) != 0.f)
			m_timeline.insert(m_timeline.begin(), std::make_tuple(0.f, value));
		else
			std::get<1>(m_timeline[0]) = value;
	}
	else if (time >= 1.f)
	{
		std::vector<std::tuple<float, float>>::iterator it = std::find_if(m_timeline.begin(), m_timeline.end(), [](std::tuple<float, float>& key) { return std::get<0>(key) == 1.f; });
		
		if (m_timeline.empty())
			m_timeline.push_back(std::make_tuple(1.f, value));
		else if (std::get<0>(m_timeline[m_timeline.size() - 1]) != 1.f)
			m_timeline.insert(m_timeline.begin(), std::make_tuple(1.f, value));
		else
			std::get<1>(m_timeline[m_timeline.size() - 1]) = value;
	}
	else
	{
		for (std::vector<std::tuple<float, float>>::iterator it = m_timeline.begin(); it != m_timeline.end(); ++it)
		{
			if (std::get<0>(*it) == time) {
				std::get<1>(*it) = value;
				return;
			}
			else if (std::get<0>(*it) > time)
			{
				m_timeline.insert(it, std::make_tuple(time, value));
				return;
			}
		}

		m_timeline.push_back(std::make_tuple(time, value));
	}
}

float Timeline::Evaluate(float time) const
{
	if (m_timeline.empty())
	{
		fmt::print(stderr, fg(fmt::color::yellow), "[Timeline Warning]");
		fmt::println(" empty timeline. return 0");
		return 0.f;
	}

	if (time <= 0.f)
		return std::get<1>(m_timeline[0]);
	else if (time >= 1.f)
		return std::get<1>(m_timeline[m_timeline.size() - 1]);
	else
	{
		auto itBefore = std::find_if(m_timeline.rbegin(), m_timeline.rend(), [time](const std::tuple<float, float>& keyValue) { return std::get<0>(keyValue) <= time; });
		auto itAfter = std::find_if(m_timeline.begin(), m_timeline.end(), [time](const std::tuple<float, float>& keyValue) { return std::get<0>(keyValue) > time; });

		// entre les deux
		if (itBefore != m_timeline.rend() && itAfter != m_timeline.end())
		{
			float delta = (time - std::get<0>(*itBefore)) / (std::get<0>(*itAfter) - std::get<0>(*itBefore));
			return std::lerp(std::get<1>(*itBefore), std::get<1>(*itAfter), delta);
		}
		else
		{
			if (itBefore != m_timeline.rend())
				return std::get<1>(*itBefore);
			else
				return std::get<1>(*itAfter);
		}
	}
}
